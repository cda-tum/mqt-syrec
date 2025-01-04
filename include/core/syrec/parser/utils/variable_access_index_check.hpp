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

    /// Determine whether the defined indices of the variable access are within range of the dimensions of the accessed variable. \n
    /// Note that no expressions are evaluated and no loop variable value lookup is performed, both cases will lead to the validity of the index being considered as unknown. \n
    /// Additionally, the validity of the indices of all accessed dimensions at indices larger than the number of dimensions of the accessed variable are considered as unknown.
    /// @param variableAccess The variable access to validate
    /// @return The validity of the indeces defined in the accessed values per dimension and bitrange.
    [[nodiscard]] std::optional<VariableAccessIndicesValidity> validate(const syrec::VariableAccess& variableAccess);
}
#endif