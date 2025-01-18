#ifndef CORE_SYREC_PARSER_UTILS_VARIABLE_OVERLAP_CHECK_HPP
#define CORE_SYREC_PARSER_UTILS_VARIABLE_OVERLAP_CHECK_HPP

#include <core/syrec/variable.hpp>
#include <vector>

namespace utils {
    struct VariableAccessOverlapCheckResult {
        enum class OverlapState {
            Overlapping,
            MaybeOverlapping,
            NotOverlapping
        };

        /**
         * A container storing information about the indices that led to the detected of the overlap between two variable accesses.
         */
        struct OverlappingIndicesContainer {
            std::vector<unsigned int> knownValueOfAccessedValuePerDimension;
            unsigned int              optionalOverlappingBit;
        };

        explicit VariableAccessOverlapCheckResult(const OverlapState overlapState):
            overlapState(overlapState) {}

        OverlapState                               overlapState;
        std::optional<OverlappingIndicesContainer> overlappingIndicesInformations;
    };

    /**
     * @brief Check whether the two given variable accesses overlap.
     * @details Two variable accesses overlap iff:
     * - The variable identifiers match
     * - The accessed values per dimension match
     * - The accessed bit ranges overlap <br>
     * Note: Expressions defined in either the bitrange or accessed values per dimension are not evaluated and in case of that such an expression was defined
     * for any of the accessed values of any dimension will result in a potential overlap being reported. If both indices of the explicitly accessed bitrange of
     * any of the two variable accesses cannot be determined, a potential overlap will be reported. 
     *
     * @param lVariableAccess The first operand of the binary overlap operation
     * @param rVariableAccess The second operand of the binary overlap operation
     * @return A container storing the overlap state of the two variable access with additional overlap information in case of an overlap at a known position in either the accessed values of the dimensions or implicitly/explicity accessed bit range, std::nullopt if the dimensions or bitwidths of the reference variable did not match.
     */
    [[nodiscard]] std::optional<VariableAccessOverlapCheckResult> checkOverlapBetweenVariableAccesses(const syrec::VariableAccess& lVariableAccess, const syrec::VariableAccess& rVariableAccess);
}
#endif