#ifndef CORE_SYREC_PARSER_UTILS_VARIABLE_OVERLAP_CHECK_HPP
#define CORE_SYREC_PARSER_UTILS_VARIABLE_OVERLAP_CHECK_HPP

#include <core/syrec/variable.hpp>

namespace utils {
    enum class VariableAccessOverlapCheckResult {
        Overlapping,
        MaybeOverlapping,
        NotOverlapping
    };

    /// <summary>
    /// Check whether the two given variable accesses overlap. \n
    /// Two varible accesses overlap iff: \n
    /// I.      The variable identifiers match \n
    /// II.     The accessed values per dimension match \n
    /// III.    The accessed bit ranges overlap \n
    /// Note: Expressions defined in either the bitrange or accessed values per dimension are not evaluated and will result in an inconclusive result being returned.
    /// </summary>
    /// <param name="lVariableAccess">The first operand of the binary overlap operation</param>
    /// <param name="rVariableAccess">The second operand of the binary overlap operation</param>
    /// <returns>An enumeration value returning whether the two variable accesses overlap or if an inconclusive result was determined.</returns>
    [[nodiscard]] VariableAccessOverlapCheckResult checkOverlapBetweenVariableAccesses(const syrec::VariableAccess& lVariableAccess, const syrec::VariableAccess& rVariableAccess);
}
#endif