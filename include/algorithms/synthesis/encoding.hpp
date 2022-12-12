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

        auto traverse(TruthTable::Cube&& encodedCube, TruthTable::CubeMultiMap& encoding) const -> void {
            // leaf node -> add encoding
            if (!data.empty()) {
                encoding.emplace(data, std::move(encodedCube));
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

    auto encodeHuffman(TruthTable& tt, bool additionalLine = true) -> TruthTable::CubeMultiMap;

    auto augmentWithConstants(TruthTable& tt, std::size_t const& nBits, bool appendZero = false) -> void;

} //namespace syrec
