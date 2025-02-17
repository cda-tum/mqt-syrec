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
            unsigned int              overlappingBit;
        };

        explicit VariableAccessOverlapCheckResult(const OverlapState overlapState):
            overlapState(overlapState) {}

        [[nodiscard]] std::string stringifyOverlappingIndicesInformation() const;

        OverlapState                               overlapState;
        std::optional<OverlappingIndicesContainer> overlappingIndicesInformations;
    };

    /**
     * @brief Check whether the two given variable accesses overlap.
     * @details Two variable accesses overlap iff:
     * - The variable identifiers match
     * - The accessed values per dimension match
     * - The accessed bit ranges overlap <br>
     * Note: Expressions defined in either the bitrange or accessed value per dimension are not evaluated and will result in a potential overlap being reported if such an expression is detected for any of the accessed value of any dimension. <br>
     * In case that all indices in the dimension access evaluate to a constant value, the result will depend on the overlap between the accessed bit ranges. <br>
     * If both indicies of the explicitly accessed bitrange of either of the two varaible access are not constants, a potential overlap will be reported. <br>
     * Invalid values for either any of the expressions defining the accessed value per dimension of for any component of the bitrange is treated in the same way as non-constant values (i.e. resulting in a potential overlap [see description above]) while out of range constant value indices
     * are treated identical to 'valid' indices.
     *
     * @param lVariableAccess The first operand of the binary overlap operation
     * @param rVariableAccess The second operand of the binary overlap operation
     * @return In case that either a potential or definitive overlap was detected, a container holding information about the type of overlap and additional information about the overlapping indices (in case of a definitive overlap) from the point of the lhs variable access, otherwise std::nullopt is returned. <br>
     */
    [[nodiscard]] std::optional<VariableAccessOverlapCheckResult> checkOverlapBetweenVariableAccesses(const syrec::VariableAccess& lVariableAccess, const syrec::VariableAccess& rVariableAccess);
}
#endif