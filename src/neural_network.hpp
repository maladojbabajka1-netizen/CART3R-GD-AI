
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <random>

namespace CART3R {

    constexpr std::size_t INPUT_SIZE = 64;
    constexpr std::size_t HIDDEN_SIZE = 1024;
    constexpr std::size_t OUTPUT_SIZE = 3;

    using HiddenLayer =
        std::array<float, HIDDEN_SIZE>;

    using WeightMatrix =
        std::array<std::array<float, HIDDEN_SIZE>, HIDDEN_SIZE>;

    class NeuralNetwork {
    private:
        std::array<std::array<float, HIDDEN_SIZE>, INPUT_SIZE> weights1{};
        WeightMatrix weights2{};
        WeightMatrix weights3{};

        std::array<std::array<float, OUTPUT_SIZE>, HIDDEN_SIZE> weights4{};

        HiddenLayer biases1{};
        HiddenLayer biases2{};
        HiddenLayer biases3{};

        std::array<float, OUTPUT_SIZE> biases4{};

        static float randomWeight() {
            static std::mt19937 rng(std::random_device{}());
            static std::uniform_real_distribution<float> dist(-0.05f, 0.05f);
            return dist(rng);
        }

        static float activation(float x) {
            return std::tanh(x);
        }

    public:
        NeuralNetwork() {
            for (auto& row : weights1)
                for (auto& w : row)
                    w = randomWeight();

            for (auto& row : weights2)
                for (auto& w : row)
                    w = randomWeight();

            for (auto& row : weights3)
                for (auto& w : row)
                    w = randomWeight();

            for (auto& row : weights4)
                for (auto& w : row)
                    w = randomWeight();
        }

        std::array<float, OUTPUT_SIZE> predict(
            const std::array<float, INPUT_SIZE>& input
        ) const {
            HiddenLayer first{};
            HiddenLayer second{};
            HiddenLayer third{};

            std::array<float, OUTPUT_SIZE> output{};

            // Input -> Hidden layer 1
            for (std::size_t h = 0; h < HIDDEN_SIZE; ++h) {
                float sum = biases1[h];

                for (std::size_t i = 0; i < INPUT_SIZE; ++i)
                    sum += input[i] * weights1[i][h];

                first[h] = activation(sum);
            }

            // Hidden layer 1 -> Hidden layer 2
            for (std::size_t h = 0; h < HIDDEN_SIZE; ++h) {
                float sum = biases2[h];

                for (std::size_t i = 0; i < HIDDEN_SIZE; ++i)
                    sum += first[i] * weights2[i][h];

                second[h] = activation(sum);
            }

            // Hidden layer 2 -> Hidden layer 3
            for (std::size_t h = 0; h < HIDDEN_SIZE; ++h) {
                float sum = biases3[h];

                for (std::size_t i = 0; i < HIDDEN_SIZE; ++i)
                    sum += second[i] * weights3[i][h];

                third[h] = activation(sum);
            }

            // Hidden layer 3 -> Output
            for (std::size_t o = 0; o < OUTPUT_SIZE; ++o) {
                float sum = biases4[o];

                for (std::size_t i = 0; i < HIDDEN_SIZE; ++i)
                    sum += third[i] * weights4[i][o];

                output[o] = activation(sum);
            }

            return output;
        }
    };

}
