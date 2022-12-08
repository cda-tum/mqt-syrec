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

    auto TruthTable::equal(TruthTable const& tt1, TruthTable const& tt2, bool equalityUpToDontCare) -> bool {
        if (!equalityUpToDontCare) {
            return (tt1 == tt2);
        }

        // both the truth table must have the same number of I/O combinations.
        if (tt1.size() != tt2.size()) {
            return false;
        }

        // no. of primary outputs.
        const auto noOfPrimaryOutputs = static_cast<int>(std::min(tt1.nOutputs(), tt2.nOutputs()));

        auto tt1It = tt1.begin();
        auto tt2It = tt2.begin();

        while (tt1It != tt1.end() || tt2It != tt2.end()) {
            TruthTable::Cube const out1{tt1It->second.begin(), tt1It->second.begin() + noOfPrimaryOutputs};
            TruthTable::Cube const out2{tt2It->second.begin(), tt2It->second.begin() + noOfPrimaryOutputs};

            if (!(TruthTable::Cube::checkCubeEquality(out1, out2))) {
                return false;
            }
            ++tt1It;
            ++tt2It;
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
