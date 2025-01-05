#ifndef CORE_SYREC_PARSER_UTILS_VARIABLE_ACCESS_INDEX_CHECK_HPP
#define CORE_SYREC_PARSER_UTILS_VARIABLE_ACCESS_INDEX_CHECK_HPP

#include <core/syrec/variable.hpp>

#include <optional>

namespace utils {
    struct VariableAccessIndicesValidity {
        struct IndexValidationResult {
            enum IndexValidity {
                Ok,
                OutOfRange,
                Unknown
            };
            IndexValidity               indexValidity;
            std::optional<unsigned int> indexValue;

            explicit IndexValidationResult(IndexValidity indexValidity, std::optional<unsigned int> indexValue):
                indexValidity(indexValidity), indexValue(indexValue) {}
        };

        struct BitRangeValidityResult {
            IndexValidationResult bitRangeStartValidity;
            IndexValidationResult bitRangeEndValiditiy;
        };
        std::vector<IndexValidationResult>    accessedValuePerDimensionValidity;
        std::optional<BitRangeValidityResult> bitRangeAccessValidity;

        [[nodiscard]] bool isValid() const;
    };

    // TODO: What about evaluation of expressions (handling unknown loop variables, division by zero => should index validity be considered as unknown?)
    // TODO: Providing valid variable accesses could also be the reponsibility of the caller?
    // TODO: Offer overload with evaluation of expressions?

    /**
     * @brief   Determine whether the defined indices of the variable access are within range of the dimensions of the accessed variable.
     * @details Note that no expressions are evaluated and no loop variable value lookup is performed, both cases will lead to the validity of the index to be considered as unknown.
     *          Additionally, the validity of index values in accessed dimensions at indices larger than the number of dimensions of the accessed variable is considered to be unknown.
     * @param variableAccess The variable access to validate
     * @return The validity of the indices defined in the accessed values per dimension and bitrange.
     */
    [[nodiscard]] std::optional<VariableAccessIndicesValidity> validateVariableAccessIndices(const syrec::VariableAccess& variableAccess);
}
#endif