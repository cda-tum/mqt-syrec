#include "algorithms/synthesis/encoding.hpp"

namespace syrec {

    auto extend(TruthTable& tt) -> void {
        // ensure that the resulting complete table can be stored in the cube map (at most 63 inputs, probably less in practice)
        if (!tt.empty() && (tt.nInputs() > static_cast<std::size_t>(std::log2(tt.max_size())) || tt.nInputs() > 63U)) {
            throw std::invalid_argument("Overflow!, Number of inputs is greater than maximum capacity " + std::string("(") + std::to_string(std::min(static_cast<unsigned>(std::log2(tt.max_size())), 63U)) + std::string(")"));
        }

        TruthTable newTT{};

        for (auto const& [input, output]: tt) {
            // ensure that all outputs are complete
            assert(std::none_of(output.cbegin(), output.cend(), [](auto const& v) { return !v.has_value(); }));

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

    auto encodeHuffman(TruthTable& tt, bool additionalLine) -> void {
        std::multimap<TruthTable::Cube, std::size_t> outputFreq;

        for (const auto& [input, output]: tt) {
            auto it = outputFreq.find(output);
            if (it == outputFreq.end()) {
                outputFreq.emplace(output, 1U);
            } else {
                it->second++;
            }
        }

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

        std::map<TruthTable::Cube, std::stack<TruthTable::Cube>> encFreq;
        const auto                                               requiredGarbage = minHeap.top()->freq;
        // determine encoding from Huffman tree
        std::multimap<TruthTable::Cube, TruthTable::Cube> encoding{};
        minHeap.top()->traverse({}, encoding);

        // resize all outputs to the correct size (by adding don't care values)
        for (auto& [input, output]: encoding) {
            auto freq = 1U << (requiredGarbage - output.size());
            output.resize(requiredGarbage);

            if (!additionalLine) {
                while (freq != 0U) {
                    encFreq[input].emplace(output);
                    freq--;
                }
            }
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
    }

    auto augmentWithConstants(TruthTable& tt) -> void {
        for (auto const& [input, output]: tt) {
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
            auto nh  = tt.extract(input);
            nh.key() = newCube;
            tt.insert(std::move(nh));
        }
    }

} // namespace syrec
