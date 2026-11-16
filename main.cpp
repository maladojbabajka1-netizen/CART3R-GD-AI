#include <Geode/Geode.hpp>
#include <algorithm>
#include <array>
#include <unordered_map>

using namespace geode::prelude;

// First prototype: a small Q-learning memory system.
// The next development step is connecting observations and actions
// to Geometry Dash gameplay. This file does not yet control the player.

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
        float discount = 0.90f;
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
                (reward + discount * bestFuture - current[index]);
        }

        void reset() {
            memory.clear();
        }
    };

    LearningAI g_ai;
}

$on_mod(Loaded) {
    log::info("CART3R GD AI v0.1.0 loaded.");
    log::info("Learning memory initialized. Gameplay control is not connected yet.");
}
