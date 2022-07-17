#pragma once

#include "dd/Package.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <utility>
#include <vector>

namespace syrec {

    class TruthTable {
    public:
        class Cube {
        public:
            using Value  = std::optional<bool>;
            using Vector = std::vector<Cube>;

        private:
            std::vector<Value> cube{};

        public:
            Cube() = default;
            explicit Cube(std::vector<Value> cube):
                cube(std::move(cube)) {}

            Cube(const std::size_t bw, const Value& initializer) {
                cube.resize(bw, initializer);
            }

            template<class InputIt>
            Cube(InputIt first, InputIt last) {
                std::copy(first, last, std::back_inserter(cube));
            }

            // construct a cube from a (64bit) number with a given bitwidth
            static auto fromInteger(const std::uint64_t number, const std::size_t bw) -> Cube {
                assert(bw <= 64U);
                Cube cube{};
                cube.reserve(bw);
                for (std::size_t i = 0U; i < bw; ++i) {
                    cube.emplace_back((number & (1ULL << (bw - 1U - i))) != 0U);
                }
                return cube;
            }
            // return integer representation of the cube
            [[nodiscard]] auto toInteger() const -> std::uint64_t {
                assert(cube.size() <= 64U);
                assert(std::none_of(cube.cbegin(), cube.cend(), [](auto const& v) { return !v.has_value(); }));
                std::uint64_t result = 0U;
                for (std::size_t i = 0U; i < cube.size(); ++i) {
                    if (*cube[i]) {
                        result |= (1ULL << (cube.size() - 1U - i));
                    }
                }
                return result;
            }

            [[nodiscard]] auto completeCubes() const -> Vector;

            auto insertZero() -> void {
                cube.insert(cube.begin(), false);
            }
            [[nodiscard]] auto append(const Value& v) const -> Cube {
                auto c = cube;
                c.emplace_back(v);
                return Cube(c);
            }
            [[nodiscard]] auto appendZero() const -> Cube {
                return append(false);
            }
            [[nodiscard]] auto appendOne() const -> Cube {
                return append(true);
            }

            // pass-through functions for underlying vector
            auto operator[](std::size_t pos) const -> Value {
                return cube[pos];
            }
            auto operator<(const Cube& cv) const -> bool {
                return (cube < cv.cube);
            }
            auto reserve(const std::size_t n) -> void {
                cube.reserve(n);
            }
            auto resize(const std::size_t n, const Value& val = Value()) -> void {
                cube.resize(n, val);
            }
            auto emplace_back(const Value& v) -> void {
                cube.emplace_back(v);
            }
            [[nodiscard]] auto size() const -> std::size_t {
                return cube.size();
            }
            [[nodiscard]] auto empty() const -> bool {
                return cube.empty();
            }
            [[nodiscard]] auto begin() const -> decltype(cube.begin()) {
                return cube.begin();
            }
            [[nodiscard]] auto cbegin() const -> decltype(cube.cbegin()) {
                return cube.cbegin();
            }
            [[nodiscard]] auto end() const -> decltype(cube.end()) {
                return cube.end();
            }
            [[nodiscard]] auto cend() const -> decltype(cube.cend()) {
                return cube.cend();
            }
        };

        using CubeMap = std::map<Cube, Cube>;

        [[nodiscard]] auto nInputs() const -> std::size_t {
            if (cubeMap.empty()) {
                return 0U;
            }
            return cubeMap.begin()->first.size();
        }

        [[nodiscard]] auto nOutputs() const -> std::size_t {
            if (cubeMap.empty()) {
                return 0U;
            }
            return cubeMap.begin()->second.size();
        }

        auto insert(const Cube& input, const Cube& output) -> void {
            assert(cubeMap.empty() || (input.size() == nInputs() && output.size() == nOutputs()));
            cubeMap.try_emplace(input, output);
        }
        auto insert(Cube&& input, Cube&& output) -> void {
            assert(cubeMap.empty() || (input.size() == nInputs() && output.size() == nOutputs()));
            cubeMap.try_emplace(std::move(input), std::move(output));
        }

        auto clear() -> void {
            cubeMap.clear();
        }

        auto extend() -> void;

        auto encodeHuffman() -> void;

        auto augmentWithConstants() -> void;

        auto buildDD(std::unique_ptr<dd::Package<>>& dd) const -> dd::mEdge;

    private:
        CubeMap cubeMap{};

        struct MinHeapNode {
            Cube data;

            std::size_t freq{};

            std::shared_ptr<MinHeapNode> left{};
            std::shared_ptr<MinHeapNode> right{};

            MinHeapNode(Cube data, const std::size_t freq):
                data(std::move(data)), freq(freq) {}

            auto operator>(const MinHeapNode& other) const -> bool {
                return freq > other.freq;
            }

            auto traverse(Cube&& encodedCube, CubeMap& encoding) const -> void {
                // leaf node -> add encoding
                if (!data.empty()) {
                    encoding.try_emplace(data, std::move(encodedCube));
                    return;
                }

                // non-leaf node -> traverse left and right subtree
                if (left) {
                    left->traverse(encodedCube.appendZero(), encoding);
                }
                if (right) {
                    right->traverse(encodedCube.appendOne(), encoding);
                }
            }
        };
    };
} // namespace syrec
