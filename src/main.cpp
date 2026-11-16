#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <algorithm>
#include <string>

using namespace geode::prelude;

namespace {
    std::string makeLevelKey(GJGameLevel* level) {
        if (!level) return "unknown";
        auto id = level->m_levelID.value();
        if (id > 0) return "level-" + std::to_string(id);
        return "local-" + std::string(level->m_levelName);
    }
}

class $modify(CART3RAIPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* hud = nullptr;
        std::string saveKey;
        float inputTimer = 0.f;
        bool holding = false;
        bool hasSentInput = false;
        float hudTimer = 0.f;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        m_fields->saveKey = "cart3r-gdai-" + makeLevelKey(level);

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        m_fields->hud = CCLabelBMFont::create("CART3R AI | MANUAL", "bigFont.fnt");
        if (m_fields->hud) {
            m_fields->hud->setAnchorPoint(ccp(0.f, 1.f));
            m_fields->hud->setPosition(ccp(10.f, winSize.height - 10.f));
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

    void updateHUD() {
        if (!m_fields->hud) return;

        bool enabled = Mod::get()->getSettingValue<bool>("auto-control");
        auto best = Mod::get()->getSavedValue<int64_t>(
            m_fields->saveKey + "-best", 0
        );
        auto deaths = Mod::get()->getSavedValue<int64_t>(
            m_fields->saveKey + "-deaths", 0
        );

        int percent = std::clamp(
            static_cast<int>(this->getCurrentPercent()), 0, 100
        );

        auto label = fmt::format(
            "CART3R AI | {} | {}% | BEST {}% | DEATHS {}",
            enabled ? "AUTO" : "MANUAL", percent, best, deaths
        );
        m_fields->hud->setString(label.c_str());
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        auto mod = Mod::get();
        bool enabled = mod->getSettingValue<bool>("auto-control");

        if (!enabled || m_isPaused || m_hasCompletedLevel ||
            !m_player1 || m_player1->m_isDead) {
            releaseAutoInput();
            m_fields->inputTimer = 0.f;
        }
        else {
            float pressTime = std::clamp(
                mod->getSettingValue<float>("press-duration"),
                0.02f, 0.50f
            );
            float releaseTime = std::clamp(
                mod->getSettingValue<float>("release-duration"),
                0.02f, 1.00f
            );

            m_fields->inputTimer += dt;

            if (!m_fields->holding && m_fields->inputTimer >= releaseTime) {
                m_fields->inputTimer = 0.f;
                this->handleButton(true, 1, true);
                m_fields->holding = true;
                m_fields->hasSentInput = true;
            }
            else if (m_fields->holding && m_fields->inputTimer >= pressTime) {
                m_fields->inputTimer = 0.f;
                this->handleButton(false, 1, true);
                m_fields->holding = false;
            }
        }

        int percent = std::clamp(
            static_cast<int>(this->getCurrentPercent()), 0, 100
        );
        auto best = mod->getSavedValue<int64_t>(
            m_fields->saveKey + "-best", 0
        );

        if (percent > best) {
            mod->setSavedValue<int64_t>(
                m_fields->saveKey + "-best", percent
            );
        }

        m_fields->hudTimer += dt;
        if (m_fields->hudTimer >= 0.20f) {
            m_fields->hudTimer = 0.f;
            updateHUD();
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        releaseAutoInput();
        PlayLayer::destroyPlayer(player, object);

        if (object == m_anticheatSpike) return;

        auto mod = Mod::get();
        auto deaths = mod->getSavedValue<int64_t>(
            m_fields->saveKey + "-deaths", 0
        );
        mod->setSavedValue<int64_t>(
            m_fields->saveKey + "-deaths", deaths + 1
        );

        updateHUD();
    }

    void onQuit() {
        releaseAutoInput();
        PlayLayer::onQuit();
    }
};

$on_mod(Loaded) {
    log::info("CART3R GD AI v0.4.0 loaded.");
    log::info("Auto Control is a fixed-interval input prototype.");
    log::info("It does not detect obstacles or learn a successful jump policy yet.");
}
