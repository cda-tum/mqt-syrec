#include "algorithms/synthesis/encoding.hpp"

namespace syrec {

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

        const auto nPrimaryOutputs = tt.nPrimaryOutputs();

        // resize all outputs to the correct size (by adding don't care values)
        for (auto& [input, output]: encoding) {
            output.resize(requiredGarbage);
        }

        // reset and resize the garbage bits.
        tt.resizeGarbage(requiredGarbage);

        // the bits excluding the primary outputs.
        const auto garbageBits = nBits - nPrimaryOutputs;

        // the bits excluding the primary outputs must be lesser than the codeword length.
        assert(garbageBits < requiredGarbage);

        // the bits excluding the primary outputs are set to garbage.
        for (auto i = 0U; i < garbageBits; i++) {
            tt.setGarbage(i);
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

    auto augmentWithConstants(TruthTable& tt, std::size_t const& nBits, bool appendZero) -> void {
        const auto requiredOutConstants = nBits - tt.nOutputs();
        const auto requiredInConstants  = nBits - tt.nInputs();

        for (auto& [input, output]: tt) {
            const auto oldGarbageVec         = tt.getGarbage();
            const auto currentGarbageVecSize = oldGarbageVec.size();

            if (appendZero) {
                // based on the requiredOutConstants, zeros are appended to the outputs.
                for (auto i = 0U; i < requiredOutConstants; i++) {
                    // based on the requiredOutConstants, zeros are appended to the outputs.
                    output.emplace_back(false);

                    if (currentGarbageVecSize != nBits) {
                        // reset and resize the garbage.
                        tt.resizeGarbage(currentGarbageVecSize + 1);

                        for (auto cg = 0U; cg < currentGarbageVecSize; cg++) {
                            if (oldGarbageVec[cg]) {
                                // storing the old garbage bits in proper order.
                                tt.setGarbage(cg + 1);
                            }
                        }
                        // setting the newly added garbage bits.
                        tt.setGarbage(0);
                    }
                }

            } else {
                for (auto i = 0U; i < requiredOutConstants; i++) {
                    // based on the requiredOutConstants, zeros are inserted to the outputs.
                    output.insertZero();
                    if (currentGarbageVecSize != nBits) {
                        // reset and resize the garbage.
                        tt.resizeGarbage(currentGarbageVecSize + 1U);
                        for (auto cg = 0U; cg < currentGarbageVecSize; cg++) {
                            // storing the old garbage bits in proper order.
                            if (oldGarbageVec[cg]) {
                                tt.setGarbage(cg);
                            }
                        }
                    }
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

            const auto oldConstantVec         = tt.getConstants();
            const auto currentConstantVecSize = oldConstantVec.size();

            if (appendZero) {
                for (std::size_t i = 0; i < requiredInConstants; i++) {
                    // based on the requiredInConstants, zeros are appended to the inputs.
                    newCube.emplace_back(false);
                    if (currentConstantVecSize != nBits) {
                        // reset and resize the constants.
                        tt.resizeConstants(currentConstantVecSize + 1);
                        for (auto cg = 0U; cg < currentConstantVecSize; cg++) {
                            // storing the old constant bits in proper order.
                            if (oldConstantVec[cg]) {
                                tt.setConstant(cg + 1);
                            }
                        }
                        // setting the newly added constant bit.
                        tt.setConstant(0);
                    }
                }
            } else {
                // based on the requiredConstants, zeros are inserted to the inputs.
                for (std::size_t i = 0; i < requiredConstants; i++) {
                    newCube.insertZero();
                    if (currentConstantVecSize != nBits) {
                        // reset and resize the constants.
                        tt.resizeConstants(currentConstantVecSize + 1);
                        for (auto cg = 0U; cg < currentConstantVecSize; cg++) {
                            if (oldConstantVec[cg]) {
                                // storing the old constant bits in proper order.
                                tt.setConstant(cg);
                            }
                        }
                        // setting the newly added constant bit.
                        tt.setConstant(currentConstantVecSize);
                    }
                }
            }
            auto nh  = tt.extract(input);
            nh.key() = newCube;
            tt.insert(std::move(nh));
        }
    }

} // namespace syrec
