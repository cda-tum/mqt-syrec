#include "core/truthTable/truth_table.hpp"

namespace syrec {

    auto TruthTable::Cube::completeCubes() const -> Vector {
        std::vector<std::size_t> dcPositions;
        dcPositions.reserve(size());
        for (auto pos = 0U; pos < size(); pos++) {
            if (!cube[pos].has_value()) // if DC
            {
                dcPositions.emplace_back(pos);
            }
        }

        Vector     result{};
        const auto dcVecSize = dcPositions.size();
        const auto dcSize    = 1U << dcVecSize;
        result.reserve(dcSize);
        Cube dcCube(cube);

        for (auto i = 0U; i < dcSize; ++i) {
            for (auto j = 0U; j < dcVecSize; ++j) {
                const auto localBit = (i & (1U << (dcVecSize - j - 1))) != 0;

                dcCube[dcPositions[j]] = localBit;
            }
            result.emplace_back(dcCube);
        }
        return result;
    }

    auto TruthTable::equal(TruthTable& tt1, TruthTable& tt2, bool append, bool equalityUpToDontCare) -> bool {
        if (!equalityUpToDontCare) {
            return (tt1 == tt2);
        }

        // total number of bits.
        const auto nBits = std::max(tt1.nInputs(), tt2.nInputs());
        assert(nBits < 65U);

        // no. of primary inputs and primary outputs.
        const auto noOfPrimaryInputs  = std::min(tt1.nInputs(), tt2.nInputs());
        const auto noOfPrimaryOutputs = std::min(tt1.nOutputs(), tt2.nOutputs());

        // total number of input combinations.
        const auto totalInputs = 1U << noOfPrimaryInputs;

        // no. of zeros to be appended or inserted to the inputs.
        const auto ancillaBits = nBits - noOfPrimaryInputs;

        // no. of zeros that is to be inserted to the inputs to match the length of the outputs.
        const auto matchingAncillaBits = noOfPrimaryOutputs - noOfPrimaryInputs;

        std::uint64_t n = 0U;

        while (n < totalInputs) {
            const auto input = TruthTable::Cube::fromInteger(n, noOfPrimaryInputs);
            ++n;

            TruthTable::Cube tt1Input{input};
            TruthTable::Cube tt2Input{input};

            // match the inputs to the length of the outputs.
            if (tt1.nOutputs() > tt1.nInputs()) {
                for (auto i = 0U; i < matchingAncillaBits; i++) {
                    tt1Input.insertZero();
                }
            }

            if (tt2.nOutputs() > tt2.nInputs()) {
                for (auto i = 0U; i < matchingAncillaBits; i++) {
                    tt2Input.insertZero();
                }
            }

            const bool alterInput1 = tt2.nInputs() == noOfPrimaryInputs && tt1.nInputs() > noOfPrimaryInputs;
            const bool alterInput2 = tt1.nInputs() == noOfPrimaryInputs && tt2.nInputs() > noOfPrimaryInputs;

            for (auto i = 0U; i < ancillaBits; i++) {
                // append zeros to the corresponding input
                if (append) {
                    if (alterInput1 && !alterInput2) {
                        tt1Input.emplace_back(false);
                    } else if (alterInput2 && !alterInput1) {
                        tt2Input.emplace_back(false);
                    }
                } else {
                    if (alterInput1 && !alterInput2) {
                        tt1Input.insertZero();
                    } else if (alterInput2 && !alterInput1) {
                        tt2Input.insertZero();
                    }
                }
            }

            const auto foundtt1 = tt1.find(tt1Input) != tt1.end();
            const auto foundtt2 = tt2.find(tt2Input) != tt2.end();

            if (!foundtt1 || !foundtt2) {
                continue;
            }

            TruthTable::Cube tt1Output{tt1[tt1Input]};
            tt1Output.resize(nBits);
            TruthTable::Cube tt2Output{tt2[tt2Input]};
            tt2Output.resize(nBits);

            if (!Cube::checkCubeEquality(tt1Output, tt2Output)) {
                return false;
            }
        }

        return true;
    }

    auto TruthTable::minimumAdditionalLinesRequired() const -> std::size_t {
        // calculate the frequency of each unique output pattern.
        std::map<TruthTable::Cube, std::size_t> outputFreq;
        for (const auto& [input, output]: cubeMap) {
            outputFreq[output]++;
        }

        const auto maxPair = std::max_element(outputFreq.begin(), outputFreq.end(), [](const std::pair<TruthTable::Cube, std::size_t>& p1, const std::pair<TruthTable::Cube, std::size_t>& p2) { return p1.second < p2.second; });

        return static_cast<std::size_t>(std::ceil(std::log2(maxPair->second)));
    }
} // namespace syrec
