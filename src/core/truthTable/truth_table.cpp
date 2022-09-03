#include "core/truthTable/truth_table.hpp"

namespace syrec {

    auto transformCharToCubeValue(const char& c) -> std::optional<bool> {
        switch (c) {
            case '-':
            case '~':
                return {};
            case '0':
                return false;
            case '1':
                return true;
            default:
                throw std::invalid_argument("Unknown Character");
        }
    }

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

} // namespace syrec
