#include "algorithms/synthesis/encoding.hpp"

namespace syrec {

    auto extend(TruthTable& tt) -> void {
        // ensure that the resulting complete table can be stored in the cube map (at most 63 inputs, probably less in practice)
        if (!tt.empty() && (tt.nInputs() > static_cast<std::size_t>(std::log2(tt.max_size())) || tt.nInputs() > 63U)) {
            throw std::invalid_argument("Overflow!, Number of inputs is greater than maximum capacity " + std::string("(") + std::to_string(std::min(static_cast<unsigned>(std::log2(tt.max_size())), 63U)) + std::string(")"));
        }

        TruthTable newTT{};

        for (auto const& [input, output]: tt) {
            // compute the complete cubes for the input
            auto completeInputs = input.completeCubes();
            // move all the complete cubes to the new cube map
            for (auto const& completeInput: completeInputs) {
                newTT.try_emplace(completeInput, output);
            }
        }
        // swap the new cube map with the old one
        tt.swap(newTT);

        // construct output cube
        const auto output = TruthTable::Cube(tt.nOutputs(), false);

        std::uint64_t pos = 0U;
        for (const auto& [input, _]: tt) {
            // fill in all the missing inputs
            const auto number = input.toInteger();
            for (std::uint64_t i = pos; i < number; ++i) {
                tt[TruthTable::Cube::fromInteger(i, tt.nInputs())] = output;
            }
            pos = number + 1U;
        }
        // fill in the remaining missing inputs (if any)
        const std::uint64_t max = 1ULL << tt.nInputs();
        for (std::uint64_t i = pos; i < max; ++i) {
            tt[TruthTable::Cube::fromInteger(i, tt.nInputs())] = output;
        }
    }

    auto encodeHuffman(TruthTable& tt) -> TruthTable::CubeMap {
        std::map<TruthTable::Cube, std::size_t> outputFreq;
        for (const auto& [input, output]: tt) {
            outputFreq[output]++;
        }

        // if the truth table function is already reversible, no encoding is necessary
        if (outputFreq.size() == tt.size()) {
            return {};
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

        // Minimum no. of additional lines required.
        const auto additionalLines = tt.minimumAdditionalLinesRequired();

        const auto requiredGarbage = minHeap.top()->freq;
        const auto nBits           = std::max(tt.nInputs(), tt.nOutputs() + additionalLines);
        const auto r               = nBits - requiredGarbage;

        // determine encoding from Huffman tree
        TruthTable::CubeMap encoding{};
        minHeap.top()->traverse({}, encoding);

        // resize all outputs to the correct size (by adding don't care values)
        for (auto& [input, output]: encoding) {
            output.resize(requiredGarbage);
        }

        // modify the codewords by filling in the redundant dc positions.
        for (auto& [pattern, code]: encoding) {
            TruthTable::Cube outCube(pattern);
            outCube.resize(nBits);

            TruthTable::Cube newCode{};
            newCode.reserve(requiredGarbage);
            for (auto i = 0U; i < requiredGarbage; i++) {
                if (code[i].has_value()) {
                    newCode.emplace_back(code[i]);
                } else {
                    newCode.emplace_back(outCube[r + i]);
                }
            }

            encoding[pattern] = newCode;
        }

        // encode all the outputs
        for (auto& [input, output]: tt) {
            output = encoding[output];
        }

        return encoding;
    }

    auto augmentWithConstants(TruthTable& tt, std::size_t const& nBits, bool appendZero, bool appendDc) -> void {
        const auto requiredOutConstants = nBits - tt.nOutputs();
        const auto requiredInConstants  = nBits - tt.nInputs();

        for (auto& [input, output]: tt) {
            // add necessary constant inputs to the outputs based on the total number of bits (nBits).
            if (appendDc || appendZero) {
                if (appendDc) {
                    // based on the nBits, don't cares are appended to the outputs.
                    output.resize(nBits);
                } else {
                    // based on the requiredOutConstants, zeros are appended to the outputs.
                    for (auto i = 0U; i < requiredOutConstants; i++) {
                        output.emplace_back(false);
                    }
                }
            } else {
                // based on the requiredOutConstants, zeros are inserted to the outputs.
                for (auto i = 0U; i < requiredOutConstants; i++) {
                    output.insertZero();
                }
            }
            const auto inputSize  = input.size();
            const auto outputSize = output.size();
            if (inputSize >= outputSize) {
                continue;
            }

            const auto requiredConstants = outputSize - inputSize;
            auto       newCube           = input;
            newCube.reserve(outputSize);
            if (appendZero) {
                // based on the requiredInConstants, zeros are appended to the inputs.
                for (std::size_t i = 0; i < requiredInConstants; i++) {
                    newCube.emplace_back(false);
                }
            } else {
                // based on the requiredConstants, zeros are inserted to the inputs.
                for (std::size_t i = 0; i < requiredConstants; i++) {
                    newCube.insertZero();
                }
            }
            auto nh  = tt.extract(input);
            nh.key() = newCube;
            tt.insert(std::move(nh));
        }
    }

} // namespace syrec
