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
            Cube zero(cube);
            zero[pos] = false;

            auto completeZero = zero.completeCubes();
            // move the computed cubes to the result vector
            result.insert(result.end(),
                          std::make_move_iterator(completeZero.begin()),
                          std::make_move_iterator(completeZero.end()));

            // recursively compute all the complete cubes for the one case

            Cube one(cube);
            one[pos] = true;

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

} // namespace syrec
