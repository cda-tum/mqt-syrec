#ifndef ENCODING_HPP
#define ENCODING_HPP

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
            return freq > other.freq;
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

    inline auto extend(TruthTable& tt) -> void {
        // ensure that the resulting complete table can be stored in the cube map (at most 63 inputs, probably less in practice)
        assert(tt.cubeMap.empty() || (tt.nInputs() <= std::log2(tt.cubeMap.max_size()) && tt.nInputs() <= 63U));

        TruthTable::CubeMap newCubeMap{};

        for (auto const& [input, output]: tt.cubeMap) {
            // ensure that all outputs are complete
            assert(std::none_of(output.cbegin(), output.cend(), [](auto const& v) { return !v.has_value(); }));

            // compute the complete cubes for the input
            auto completeInputs = input.completeCubes();
            // move all the complete cubes to the new cube map
            for (auto& completeInput: completeInputs) {
                newCubeMap.try_emplace(std::move(completeInput), output);
            }
        }
        // swap the new cube map with the old one
        tt.cubeMap = std::move(newCubeMap);

        // construct output cube
        const auto output = TruthTable::Cube(tt.nOutputs(), false);

        std::uint64_t pos = 0U;
        for (const auto& [input, _]: tt.cubeMap) {
            // fill in all the missing inputs
            const auto number = input.toInteger();
            for (std::uint64_t i = pos; i < number; ++i) {
                tt.cubeMap[TruthTable::Cube::fromInteger(i, tt.nInputs())] = output;
            }
            pos = number + 1U;
        }
        // fill in the remaining missing inputs (if any)
        const std::uint64_t max = 1ULL << tt.nInputs();
        for (std::uint64_t i = pos; i < max; ++i) {
            tt.cubeMap[TruthTable::Cube::fromInteger(i, tt.nInputs())] = output;
        }
    }

    inline auto encodeHuffman(TruthTable& tt) -> void {
        std::map<TruthTable::Cube, std::size_t> outputFreq;
        for (const auto& [input, output]: tt.cubeMap) {
            outputFreq[output]++;
        }

        // if the truth table function is already reversible, no encoding is necessary
        if (outputFreq.size() == tt.cubeMap.size()) {
            return;
        }

        // create a priority queue for building the Huffman tree
        auto comp = [](const std::shared_ptr<MinHeapNode>& a, const std::shared_ptr<MinHeapNode>& b) {
            return *a > *b;
        };
        std::priority_queue<std::shared_ptr<MinHeapNode>,
                            std::vector<std::shared_ptr<MinHeapNode>>,
                            decltype(comp)>
                minHeap(comp);

        // initialize the leaves of the Huffman tree from the output frequencies
        for (const auto& [output, freq]: outputFreq) {
            const auto requiredGarbage = static_cast<std::size_t>(std::ceil(std::log2(freq)));
            minHeap.emplace(std::make_shared<MinHeapNode>(output, requiredGarbage));
        }

        // combine the nodes with the smallest weights until there is only one node left
        while (minHeap.size() > 1U) {
            // pop the two nodes with the smallest weights
            const auto left = minHeap.top();
            minHeap.pop();
            const auto right = minHeap.top();
            minHeap.pop();
            // compute appropriate frequency to cover both nodes
            const auto freq = std::max(left->freq, right->freq) + 1U;
            // create new parent node
            auto top   = std::make_shared<MinHeapNode>(TruthTable::Cube{}, freq);
            top->left  = left;
            top->right = right;
            // add node to queue
            minHeap.emplace(std::move(top));
        }

        const auto requiredGarbage = minHeap.top()->freq;
        // determine encoding from Huffman tree
        TruthTable::CubeMap encoding{};
        minHeap.top()->traverse({}, encoding);

        // resize all outputs to the correct size (by adding don't care values)
        for (auto& [input, output]: encoding) {
            output.resize(requiredGarbage);
        }

        // encode all the outputs
        for (auto& [input, output]: tt.cubeMap) {
            output = encoding[output];
        }
    }

    inline auto augmentWithConstants(TruthTable& tt) -> void {
        for (auto const& [input, output]: tt.cubeMap) {
            const auto inputSize  = input.size();
            const auto outputSize = output.size();
            if (inputSize >= outputSize) {
                continue;
            }

            const auto requiredConstants = outputSize - inputSize;
            auto       newCube           = input;
            newCube.reserve(outputSize);
            for (std::size_t i = 0; i < requiredConstants; i++) {
                newCube.insertZero();
            }
            auto nh  = tt.cubeMap.extract(input);
            nh.key() = newCube;
            tt.cubeMap.insert(std::move(nh));
        }
    }

} //namespace syrec
#endif /* ENCODING_HPP */
