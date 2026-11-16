
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <random>

namespace CART3R {

    constexpr std::size_t INPUT_SIZE = 8;
    constexpr std::size_t HIDDEN_SIZE = 16;
    constexpr std::size_t OUTPUT_SIZE = 3;

    class NeuralNetwork {
    private:
        std::array<std::array<float, HIDDEN_SIZE>, INPUT_SIZE> weights1{};
        std::array<float, HIDDEN_SIZE> biases1{};

        std::array<std::array<float, OUTPUT_SIZE>, HIDDEN_SIZE> weights2{};
        std::array<float, OUTPUT_SIZE> biases2{};

        static float sigmoid(float x) {
            x = std::clamp(x, -20.0f, 20.0f);
            return 1.0f / (1.0f + std::exp(-x));
        }

        static float randomWeight() {
            static std::mt19937 rng(std::random_device{}());
            static std::uniform_real_distribution<float> dist(-0.5f, 0.5f);
            return dist(rng);
        }

    public:
        NeuralNetwork() {
            for (auto& row : weights1)
                for (auto& weight : row)
                    weight = randomWeight();

            for (auto& row : weights2)
                for (auto& weight : row)
                    weight = randomWeight();
        }

        std::array<float, OUTPUT_SIZE> predict(
            const std::array<float, INPUT_SIZE>& input
        ) const {
            std::array<float, HIDDEN_SIZE> hidden{};
            std::array<float, OUTPUT_SIZE> output{};

            for (std::size_t h = 0; h < HIDDEN_SIZE; ++h) {
                float sum = biases1[h];

                for (std::size_t i = 0; i < INPUT_SIZE; ++i)
                    sum += input[i] * weights1[i][h];

                hidden[h] = sigmoid(sum);
            }

            for (std::size_t o = 0; o < OUTPUT_SIZE; ++o) {
                float sum = biases2[o];

                for (std::size_t h = 0; h < HIDDEN_SIZE; ++h)
                    sum += hidden[h] * weights2[h][o];

                output[o] = sigmoid(sum);
            }

            return output;
        }
    };

}
