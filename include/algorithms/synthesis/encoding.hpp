#pragma once

#include "core/truthTable/truth_table.hpp"

namespace syrec {

    struct MinHeapNode {
        TruthTable::Cube data;

        std::size_t freq{};

        std::shared_ptr<MinHeapNode> left{};
        std::shared_ptr<MinHeapNode> right{};

        MinHeapNode(TruthTable::Cube data, const std::size_t freq):
            data(std::move(data)), freq(freq) {}

        auto operator>(const MinHeapNode& other) const -> bool {
            return (freq > other.freq || (freq == other.freq && (data > other.data)));
        }

        auto traverse(TruthTable::Cube&& encodedCube, TruthTable::CubeMap& encoding) const -> void {
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

    auto extend(TruthTable& tt) -> void;

    auto encodeHuffman(TruthTable& tt, bool log2MaxOccurrences = false) -> std::pair<TruthTable::CubeMap, std::size_t>;

    auto augmentWithConstants(TruthTable& tt, std::size_t const& nBits, bool dc = false, bool appendZero = false, bool appendZeroIn = false) -> void;

} //namespace syrec
