#include "algorithms/synthesis/encoding.hpp"

namespace syrec {

    auto encodeHuffman(TruthTable& tt, bool additionalLine) -> TruthTable::CubeMultiMap {
        std::multimap<TruthTable::Cube, std::size_t> outputFreq;

        for (const auto& [input, output]: tt) {
            auto it = outputFreq.find(output);
            if (it == outputFreq.end()) {
                outputFreq.emplace(output, 1U);
            } else {
                it->second++;
            }
        }

        // Minimum no. of additional lines required.
        const auto additionalLines = tt.minimumAdditionalLinesRequired();

        if (!additionalLine) {
            // ensure that all output frequencies are a power of two
            for (auto it = outputFreq.begin(); it != outputFreq.end();) {
                auto freq = it->second;

                // continue if the output frequency is a power of two
                if ((freq & (freq - 1U)) == 0U) {
                    ++it;
                    continue;
                }

                // split frequency into powers of two
                std::size_t bitPos = 0U;
                while (freq > 0U) {
                    if ((freq & 1U) == 1U) {
                        outputFreq.emplace(it->first, 1U << bitPos);
                    }
                    freq >>= 1U;
                    ++bitPos;
                }
                // need to update iterator after erasing to avoid invalidating it
                it = outputFreq.erase(it);
            }
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

        std::map<TruthTable::Cube, std::stack<TruthTable::Cube>> encFreq;
        const auto                                               requiredGarbage = minHeap.top()->freq;
        const auto                                               nBits           = std::max(tt.nInputs(), tt.nOutputs() + additionalLines);
        const auto                                               r               = nBits - requiredGarbage;

        // determine encoding from Huffman tree
        TruthTable::CubeMultiMap encoding{};
        minHeap.top()->traverse({}, encoding);

        // resize all outputs to the correct size (by adding don't care values)
        for (auto& [input, output]: encoding) {
            auto freq = 1U << (requiredGarbage - output.size());
            output.resize(requiredGarbage);

            // modify the codewords by filling in the redundant dc positions.
            TruthTable::Cube outCube(input);
            outCube.resize(nBits);

            TruthTable::Cube newCode{};
            newCode.reserve(requiredGarbage);
            for (auto i = 0U; i < requiredGarbage; i++) {
                if (output[i].has_value()) {
                    newCode.emplace_back(output[i]);
                } else {
                    newCode.emplace_back(outCube[r + i]);
                }
            }

            output = newCode;

            if (!additionalLine) {
                while (freq != 0U) {
                    encFreq[input].emplace(output);
                    freq--;
                }
            }
        }

        const auto nPrimaryOutputs = tt.nPrimaryOutputs();

        // resize garbage to the correct size.
        tt.getGarbage().resize(requiredGarbage);

        // the bits excluding the primary outputs.
        const auto garbageBits = nBits - nPrimaryOutputs;

        // the bits excluding the primary outputs must be lesser than or equal to the codeword length.
        assert(garbageBits <= requiredGarbage);

        // the bits excluding the primary outputs are set to garbage.
        for (auto i = 0U; i < garbageBits; i++) {
            tt.setGarbage(i);
        }

        // encode all the outputs
        for (auto& [input, output]: tt) {
            if (!additionalLine) {
                const auto out = output;
                output         = encFreq[out].top();
                encFreq[out].pop();
            } else {
                output = encoding.find(output)->second;
            }
        }

        return encoding;
    }

    auto augmentWithConstants(TruthTable& tt, std::size_t const& nBits, bool appendZero) -> void {
        const auto requiredOutConstants = nBits - tt.nOutputs();
        const auto requiredInConstants  = nBits - tt.nInputs();

        for (auto& [input, output]: tt) {
            const auto currentGarbageVecSize = tt.getGarbage().size();

            if (appendZero) {
                // based on the requiredOutConstants, zeros are appended to the outputs.
                for (auto i = 0U; i < requiredOutConstants; i++) {
                    // based on the requiredOutConstants, zeros are appended to the outputs.
                    output.emplace_back(false);
                    if (currentGarbageVecSize != nBits) {
                        // add garbage at the LSB.
                        tt.getGarbage().insert(tt.getGarbage().begin(), true);
                    }
                }

            } else {
                for (auto i = 0U; i < requiredOutConstants; i++) {
                    // based on the requiredOutConstants, zeros are inserted to the outputs.
                    output.insertZero();
                    if (currentGarbageVecSize != nBits) {
                        tt.getGarbage().resize(currentGarbageVecSize + 1);
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

            const auto currentConstantVecSize = tt.getConstants().size();

            if (appendZero) {
                for (std::size_t i = 0; i < requiredInConstants; i++) {
                    // based on the requiredInConstants, zeros are appended to the inputs.
                    newCube.emplace_back(false);
                    if (currentConstantVecSize != nBits) {
                        // add a constant at the LSB.
                        tt.getConstants().insert(tt.getConstants().begin(), true);
                    }
                }
            } else {
                // based on the requiredConstants, zeros are inserted to the inputs.
                for (std::size_t i = 0; i < requiredConstants; i++) {
                    newCube.insertZero();
                    if (currentConstantVecSize != nBits) {
                        // add a constant at the MSB.
                        tt.getConstants().emplace_back(true);
                    }
                }
            }
            auto nh  = tt.extract(input);
            nh.key() = newCube;
            tt.insert(std::move(nh));
        }
    }

} // namespace syrec
