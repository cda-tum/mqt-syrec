#include "core/truthTable/truth_table.hpp"

namespace syrec {

    auto TruthTable::Cube::completeCubes() const -> Vector {
        Vector result{};

        // iterate over the values of the cube
        for (std::size_t pos = 0U; pos < size(); ++pos) {
            // skip any value that is not a don't care
            if (cube[pos].has_value()) {
                continue;
            }

            // recursively compute all the complete cubes for the zero case
            auto zero         = Cube(cube);
            zero[pos]         = false;
            auto completeZero = zero.completeCubes();
            // move the computed cubes to the result vector
            result.insert(result.end(),
                          std::make_move_iterator(completeZero.begin()),
                          std::make_move_iterator(completeZero.end()));

            // recursively compute all the complete cubes for the one case
            auto one         = Cube(cube);
            one[pos]         = true;
            auto completeOne = one.completeCubes();
            // move the computed cubes to the result vector
            result.insert(result.end(),
                          std::make_move_iterator(completeOne.begin()),
                          std::make_move_iterator(completeOne.end()));
        }
        // handle the case where the cube was already complete
        if (result.empty()) {
            result.emplace_back(cube);
        }
        return result;
    }

    auto TruthTable::extend() -> void {
        // ensure that the resulting complete table can be stored in the cube map (at most 63 inputs, probably less in practice)
        assert(cubeMap.empty() || (nInputs() <= std::log2(cubeMap.max_size()) && nInputs() <= 63U));

        CubeMap newCubeMap{};

        for (auto const& [input, output]: cubeMap) {
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
        cubeMap = std::move(newCubeMap);

        // construct output cube
        const auto output = Cube(nOutputs(), false);

        std::uint64_t pos = 0U;
        for (const auto& [input, _]: cubeMap) {
            // fill in all the missing inputs
            const auto number = input.toInteger();
            for (std::uint64_t i = pos; i < number; ++i) {
                cubeMap[Cube::fromInteger(i, nInputs())] = output;
            }
            pos = number + 1U;
        }
        // fill in the remaining missing inputs (if any)
        const std::uint64_t max = 1ULL << nInputs();
        for (std::uint64_t i = pos; i < max; ++i) {
            cubeMap[Cube::fromInteger(i, nInputs())] = output;
        }
    }

    auto TruthTable::encodeHuffman() -> void {
        std::map<Cube, std::size_t> outputFreq;
        for (const auto& [input, output]: cubeMap) {
            outputFreq[output]++;
        }

        // if the truth table function is already reversible, no encoding is necessary
        if (outputFreq.size() == cubeMap.size()) {
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
            auto top = std::make_shared<MinHeapNode>(Cube{}, freq);
            top->left = left;
            top->right = right;
            // add node to queue
            minHeap.emplace(std::move(top));
        }

        const auto requiredGarbage = minHeap.top()->freq;
        // determine encoding from Huffman tree
        CubeMap encoding{};
        minHeap.top()->traverse({}, encoding);

        // resize all outputs to the correct size (by adding don't care values)
        for (auto& [input, output]: encoding) {
            output.resize(requiredGarbage);
        }

        // encode all the outputs
        for (auto& [input, output]: cubeMap) {
            output = encoding[output];
        }
    }

    auto TruthTable::augmentWithConstants() -> void {
        for (auto const& [input, output]: cubeMap) {
            const auto inputSize = input.size();
            const auto outputSize = output.size();
            if (inputSize >= outputSize) {
                continue;
            }

            const auto requiredConstants = outputSize - inputSize;
            auto newCube = input;
            newCube.reserve(outputSize);
            for (std::size_t i = 0; i < requiredConstants; i++) {
                newCube.insertZero();
            }
            auto nh = cubeMap.extract(input);
            nh.key() = newCube;
            cubeMap.insert(std::move(nh));
        }
    }

    auto TruthTable::buildDD(std::unique_ptr<dd::Package<>>& dd) const -> dd::mEdge {
        // truth tables has to have the same number of inputs and outputs
        assert(nInputs() == nOutputs());

        if (nInputs() == 0U) {
            return dd::mEdge::zero;
        }

        auto edges = std::array<dd::mEdge, 4U>{dd::mEdge::zero, dd::mEdge::zero, dd::mEdge::zero, dd::mEdge::zero};

        // base case
        if (nInputs() == 1U) {
            for (const auto& [input, output]: cubeMap) {
                // truth table has to be completely specified
                assert(input[0].has_value());
                assert(output[0].has_value());
                const auto in = *input[0];
                const auto out = *output[0];
                const auto index = static_cast<std::size_t>(out) * 2U + static_cast<std::size_t>(in);
                edges[index] = dd::mEdge::one;
            }
            return dd->makeDDNode(0, edges);
        }

        // generate sub-tables
        std::array<TruthTable, 4U> subTables{};
        for (const auto& [input, output]: cubeMap) {
            // truth table has to be completely specified
            assert(input[0].has_value());
            assert(output[0].has_value());
            const auto in = *input[0];
            const auto out = *output[0];
            const auto index = static_cast<std::size_t>(out) * 2U + static_cast<std::size_t>(in);
            Cube reducedInput(input.begin() +1, input.end());
            Cube reducedOutput(output.begin() +1, output.end());
            subTables[index].insert(std::move(reducedInput), std::move(reducedOutput));
        }

        // recursively build the DD for each sub-table
        for(std::size_t i = 0U; i < 4U; i++) {
            edges[i] = subTables[i].buildDD(dd);
            // free up the memory used by the sub-table as fast as possible.
            subTables[i].clear();
        }

        const auto label = static_cast<dd::Qubit>(nInputs() - 1U);
        return dd->makeDDNode(label, edges);
    }
} // namespace syrec
