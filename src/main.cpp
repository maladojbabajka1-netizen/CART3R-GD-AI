#include <Geode/Geode.hpp>
#include <algorithm>
#include <array>
#include <unordered_map>

using namespace geode::prelude;

// Prototype Q-learning memory. This version exposes basic controls through
// Geode's generated mod settings. It does NOT yet read gameplay or control
// the player's jump button.

namespace {
    enum class Action {
        Release = 0,
        Hold = 1
    };

    struct StateMemory {
        std::array<float, 2> values{0.0f, 0.0f};
    };

    class LearningAI {
    public:
        float learningRate = 0.10f;
        int trainingSpeed = 5;
        bool enabled = false;
        std::unordered_map<int, StateMemory> memory;

        Action chooseAction(int state) {
            auto& values = memory[state].values;
            return values[1] > values[0] ? Action::Hold : Action::Release;
        }

        void learn(int state, Action action, float reward, int nextState) {
            auto& current = memory[state].values;
            auto& next = memory[nextState].values;
            int index = static_cast<int>(action);
            float bestFuture = std::max(next[0], next[1]);

            current[index] += learningRate *
                (reward + 0.90f * bestFuture - current[index]);
        }

        void syncSettings() {
            auto mod = Mod::get();
            enabled = mod->getSettingValue<bool>("training-enabled");
            learningRate = mod->getSettingValue<float>("learning-rate");
            trainingSpeed = mod->getSettingValue<int>("training-speed");
        }
    };

    LearningAI g_ai;
}

$on_mod(Loaded) {
    g_ai.syncSettings();
    log::info("CART3R GD AI v0.2.0 loaded.");
    log::info("Training controls are available in this mod's Geode settings.");
    log::info("Gameplay observation and player control are not connected yet.");
}
