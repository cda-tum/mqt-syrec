#ifndef CORE_SYREC_PARSER_UTILS_VARIABLE_OVERLAP_CHECK_HPP
#define CORE_SYREC_PARSER_UTILS_VARIABLE_OVERLAP_CHECK_HPP

#include <core/syrec/variable.hpp>

namespace utils {
    enum class VariableAccessOverlapCheckResult {
        Overlapping,
        MaybeOverlapping,
        NotOverlapping
    };

    /**
     * @brief Check whether the two given variable accesses overlap.
     * @details Two variable accesses overlap iff:
     * - The variable identifiers match
     * - The accessed values per dimension match
     * - The accessed bit ranges overlap <br>
     * Note: Expressions defined in either the bitrange or accessed values per dimension are not evaluated and will result in an inconclusive result being determined for
     * the dimension and/or bitrange in which said non-constant index was defined. 
     * @param lVariableAccess The first operand of the binary overlap operation
     * @param rVariableAccess The second operand of the binary overlap operation
     * @return An enumeration value returning whether the result of the overlap check, std::nullopt if the dimensions or bitwidths of the reference variable did not match.
     */
    [[nodiscard]] std::optional<VariableAccessOverlapCheckResult> checkOverlapBetweenVariableAccesses(const syrec::VariableAccess& lVariableAccess, const syrec::VariableAccess& rVariableAccess);
}
#endif