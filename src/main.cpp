
#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <algorithm>
#include <string>
#include <cmath>
#include <limits>

using namespace geode::prelude;

namespace {

    std::string makeLevelKey(GJGameLevel* level) {
        if (!level) return "unknown";

        auto id = level->m_levelID.value();

        if (id > 0)
            return "level-" + std::to_string(id);

        return "local-" + std::string(level->m_levelName);
    }

    bool isSpike(int id) {
        switch (id) {
            case 8:
            case 39:
            case 103:
            case 144:
            case 145:
            case 392:
            case 458:
                return true;
            default:
                return false;
        }
    }

    bool isSaw(int id) {
        switch (id) {
            case 88:
            case 89:
            case 98:
            case 1705:
            case 1706:
            case 1707:
                return true;
            default:
                return false;
        }
    }

}

class $modify(CART3RAIPlayLayer, PlayLayer) {

    struct Fields {
        CCLabelBMFont* hud = nullptr;

        std::string saveKey;

        float inputTimer = 0.f;
        float hudTimer = 0.f;
        float scanTimer = 0.f;

        float currentReleaseTime = 0.18f;
        float stepSize = 0.03f;

        float nearestHazardDistance =
            std::numeric_limits<float>::max();

        std::string nearestHazard = "NONE";

        int direction = 1;
        int attemptBestPercent = 0;

        bool holding = false;
        bool attemptEnded = false;
        bool completionRecorded = false;
    };

    bool init(
        GJGameLevel* level,
        bool useReplay,
        bool dontCreateObjects
    ) {
        if (!PlayLayer::init(
            level,
            useReplay,
            dontCreateObjects
        )) {
            return false;
        }

        auto mod = Mod::get();

        m_fields->saveKey =
            "cart3r-gdai-" + makeLevelKey(level);

        m_fields->currentReleaseTime =
            mod->getSavedValue<float>(
                m_fields->saveKey + "-release",
                mod->getSettingValue<float>("release-duration")
            );

        m_fields->stepSize = std::clamp(
            mod->getSettingValue<float>("learning-rate"),
            0.005f,
            0.10f
        );

        auto winSize =
            CCDirector::sharedDirector()->getWinSize();

        m_fields->hud = CCLabelBMFont::create(
            "CART3R AI | DETECTOR STARTING",
            "bigFont.fnt"
        );

        if (m_fields->hud) {
            m_fields->hud->setAnchorPoint(ccp(0.f, 1.f));
            m_fields->hud->setPosition(
                ccp(10.f, winSize.height - 10.f)
            );
            m_fields->hud->setScale(0.34f);
            m_fields->hud->setZOrder(10000);
            this->addChild(m_fields->hud);
        }

        updateHUD();

        return true;
    }

    void releaseAutoInput() {
        if (m_fields->holding) {
            this->handleButton(false, 1, true);
            m_fields->holding = false;
        }
    }

    // Check the actual level objects ahead of the player.
    void scanObstacles() {
        m_fields->nearestHazardDistance =
            std::numeric_limits<float>::max();

        m_fields->nearestHazard = "NONE";

        if (!m_player1)
            return;

        float playerX = m_player1->getPositionX();

        for (auto* obj : CCArrayExt<GameObject>(m_objects)) {
            if (!obj)
                continue;

            int id = obj->m_objectID;

            bool spike = isSpike(id);
            bool saw = isSaw(id);

            if (!spike && !saw)
                continue;

            float distance =
                obj->getPositionX() - playerX;

            // Ignore objects behind the player.
            if (distance <= 0.f)
                continue;

            // Only inspect nearby objects.
            if (distance > 1200.f)
                continue;

            if (distance <
                m_fields->nearestHazardDistance) {

                m_fields->nearestHazardDistance = distance;

                m_fields->nearestHazard =
                    spike ? "SPIKE" : "SAW";
            }
        }
    }

    void updateHUD() {
        if (!m_fields->hud)
            return;

        auto mod = Mod::get();

        bool autoEnabled =
            mod->getSettingValue<bool>("auto-control");

        bool learning =
            mod->getSettingValue<bool>("learning-mode");

        auto best = mod->getSavedValue<int64_t>(
            m_fields->saveKey + "-best",
            0
        );

        auto deaths = mod->getSavedValue<int64_t>(
            m_fields->saveKey + "-deaths",
            0
        );

        int percent = std::clamp(
            static_cast<int>(this->getCurrentPercent()),
            0,
            100
        );

        std::string mode = "MANUAL";

        if (autoEnabled && learning)
            mode = "LEARNING";
        else if (autoEnabled)
            mode = "AUTO";

        std::string distanceText = "NONE";

        if (m_fields->nearestHazard != "NONE") {
            distanceText = fmt::format(
                "{} {:.0f}u",
                m_fields->nearestHazard,
                m_fields->nearestHazardDistance
            );
        }

        auto label = fmt::format(
            "CART3R | {} | {}% | HAZARD: {} | BEST: {}% | DEATHS: {}",
            mode,
            percent,
            distanceText,
            best,
            deaths
        );

        m_fields->hud->setString(label.c_str());
    }

    void recordAttempt(int percent) {
        if (m_fields->attemptEnded)
            return;

        m_fields->attemptEnded = true;

        auto mod = Mod::get();

        auto best = mod->getSavedValue<int64_t>(
            m_fields->saveKey + "-best",
            0
        );

        bool autoEnabled =
            mod->getSettingValue<bool>("auto-control");

        bool learning =
            mod->getSettingValue<bool>("learning-mode");

        if (percent > best) {
            mod->setSavedValue<int64_t>(
                m_fields->saveKey + "-best",
                percent
            );

            mod->setSavedValue<float>(
                m_fields->saveKey + "-best-release",
                m_fields->currentReleaseTime
            );
        }
        else if (autoEnabled && learning) {
            m_fields->direction *= -1;

            m_fields->stepSize = std::max(
                0.005f,
                m_fields->stepSize * 0.5f
            );
        }

        if (autoEnabled && learning && percent < 100) {
            m_fields->currentReleaseTime +=
                m_fields->direction * m_fields->stepSize;

            m_fields->currentReleaseTime = std::clamp(
                m_fields->currentReleaseTime,
                0.02f,
                1.00f
            );

            mod->setSavedValue<float>(
                m_fields->saveKey + "-release",
                m_fields->currentReleaseTime
            );
        }

        updateHUD();
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        auto mod = Mod::get();

        bool autoEnabled =
            mod->getSettingValue<bool>("auto-control");

        bool learning =
            mod->getSettingValue<bool>("learning-mode");

        bool alive =
            m_player1 && !m_player1->m_isDead;

        if (alive &&
            m_fields->attemptEnded &&
            !m_hasCompletedLevel) {

            m_fields->attemptEnded = false;
            m_fields->attemptBestPercent = 0;
            m_fields->inputTimer = 0.f;
        }

        // Scan the level every 0.1 seconds.
        m_fields->scanTimer += dt;

        if (alive && m_fields->scanTimer >= 0.1f) {
            m_fields->scanTimer = 0.f;
            scanObstacles();
        }

        if (!autoEnabled ||
            m_isPaused ||
            m_hasCompletedLevel ||
            !alive) {

            releaseAutoInput();
            m_fields->inputTimer = 0.f;
        }
        else {
            float pressTime = std::clamp(
                mod->getSettingValue<float>("press-duration"),
                0.02f,
                0.50f
            );

            float releaseTime = learning
                ? m_fields->currentReleaseTime
                : mod->getSettingValue<float>("release-duration");

            releaseTime = std::clamp(
                releaseTime,
                0.02f,
                1.00f
            );

            m_fields->inputTimer += dt;

            if (!m_fields->holding &&
                m_fields->inputTimer >= releaseTime) {

                m_fields->inputTimer = 0.f;

                this->handleButton(true, 1, true);
                m_fields->holding = true;
            }
            else if (m_fields->holding &&
                     m_fields->inputTimer >= pressTime) {

                m_fields->inputTimer = 0.f;

                this->handleButton(false, 1, true);
                m_fields->holding = false;
            }
        }

        int percent = std::clamp(
            static_cast<int>(this->getCurrentPercent()),
            0,
            100
        );

        m_fields->attemptBestPercent = std::max(
            m_fields->attemptBestPercent,
            percent
        );

        if (m_hasCompletedLevel &&
            !m_fields->completionRecorded) {

            m_fields->completionRecorded = true;
            recordAttempt(100);
        }

        m_fields->hudTimer += dt;

        if (m_fields->hudTimer >= 0.20f) {
            m_fields->hudTimer = 0.f;
            updateHUD();
        }
    }

    void destroyPlayer(
        PlayerObject* player,
        GameObject* object
    ) {
        int percent = std::clamp(
            static_cast<int>(this->getCurrentPercent()),
            0,
            100
        );

        releaseAutoInput();

        PlayLayer::destroyPlayer(player, object);

        if (object == m_anticheatSpike)
            return;

        auto mod = Mod::get();

        auto deaths = mod->getSavedValue<int64_t>(
            m_fields->saveKey + "-deaths",
            0
        );

        mod->setSavedValue<int64_t>(
            m_fields->saveKey + "-deaths",
            deaths + 1
        );

        recordAttempt(
            std::max(percent, m_fields->attemptBestPercent)
        );
    }

    void onQuit() {
        releaseAutoInput();
        PlayLayer::onQuit();
    }
};

$on_mod(Loaded) {
    log::info("CART3R GD AI loaded.");
    log::info("Level-object obstacle scanner initialized.");
    log::info("Scanner detects selected spike and saw object IDs.");
}
