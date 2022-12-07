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

    auto TruthTable::equal(TruthTable& tt1, TruthTable& tt2, bool equalityUpToDontCare) -> bool {
        if (!equalityUpToDontCare) {
            return (tt1 == tt2);
        }

        if (tt1.nInputs() != tt2.nInputs()) {
            return false;
        }

        const auto nBits = tt1.nInputs();
        assert(nBits < 65U);
        const auto totalInputs = 1U << nBits;

        std::uint64_t n = 0U;

        while (n < totalInputs) {
            const auto input = TruthTable::Cube::fromInteger(n, nBits);
            ++n;

            const auto foundtt1 = tt1.find(input) != tt1.end();
            const auto foundtt2 = tt2.find(input) != tt2.end();

            if (!foundtt1 || !foundtt2) {
                continue;
            }

            if (!Cube::checkCubeEquality(tt1[input], tt2[input])) {
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
