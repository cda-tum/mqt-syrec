#include "core/syrec/expression.hpp"
#include <core/syrec/parser/utils/variable_access_index_check.hpp>

#include <algorithm>

using namespace utils;

inline VariableAccessIndicesValidity::IndexValidationResult::IndexValidity isIndexInRange(unsigned int indexValue, unsigned int maxAllowedValue) {
    return indexValue <= maxAllowedValue ? VariableAccessIndicesValidity::IndexValidationResult::IndexValidity::Ok : VariableAccessIndicesValidity::IndexValidationResult::IndexValidity::OutOfRange;
}

bool VariableAccessIndicesValidity::isValid() const {
    return std::all_of(
    accessedValuePerDimensionValidity.cbegin(),
    accessedValuePerDimensionValidity.cend(),
    [](const IndexValidationResult& validityOfAccessedValueOfDimension) {
        return validityOfAccessedValueOfDimension.indexValidity == IndexValidationResult::IndexValidity::Ok;
    })
    && bitRangeAccessValidity.has_value()
        ? bitRangeAccessValidity->bitRangeStartValidity.indexValidity == IndexValidationResult::IndexValidity::Ok && bitRangeAccessValidity->bitRangeEndValiditiy.indexValidity == IndexValidationResult::IndexValidity::Ok
        : true;
}

std::optional<VariableAccessIndicesValidity> validate(const syrec::VariableAccess& variableAccess) {
    VariableAccessIndicesValidity validityOfVariableAccessIndices;
    validityOfVariableAccessIndices.accessedValuePerDimensionValidity = std::vector(variableAccess.indexes.size(), VariableAccessIndicesValidity::IndexValidationResult(VariableAccessIndicesValidity::IndexValidationResult::Unknown, std::nullopt));

    std::size_t dimensionIndex = 0;
    for (const std::shared_ptr<syrec::Expression>& accessedValueOfDimension: variableAccess.indexes) {
        if (!accessedValueOfDimension)
            continue;

        if (const auto accessExprAsNumericOne = std::dynamic_pointer_cast<syrec::NumericExpression>(variableAccess.indexes.at(dimensionIndex)); accessExprAsNumericOne && accessExprAsNumericOne->value) {
            if (const std::optional<unsigned int> evaluatedAccessedValueOfDimension = accessExprAsNumericOne->value->isConstant() ? accessExprAsNumericOne->value->tryEvaluate({}) : std::nullopt; evaluatedAccessedValueOfDimension.has_value()) {
                if (dimensionIndex < variableAccess.indexes.size())
                    validityOfVariableAccessIndices.accessedValuePerDimensionValidity[dimensionIndex].indexValidity = isIndexInRange(*evaluatedAccessedValueOfDimension, variableAccess.getVar()->dimensions.at(dimensionIndex));

                validityOfVariableAccessIndices.accessedValuePerDimensionValidity[dimensionIndex].indexValue = *evaluatedAccessedValueOfDimension;
            }
        }
        ++dimensionIndex;
    }

    if (variableAccess.range.has_value()) {
        auto bitRangeStartValidity = VariableAccessIndicesValidity::IndexValidationResult(VariableAccessIndicesValidity::IndexValidationResult::Unknown, std::nullopt);
        auto bitRangeEndValidity   = VariableAccessIndicesValidity::IndexValidationResult(VariableAccessIndicesValidity::IndexValidationResult::Unknown, std::nullopt);

        const syrec::Number::ptr& bitRangeStart = variableAccess.range->first;
        const syrec::Number::ptr& bitRangeEnd   = variableAccess.range->second;
        if (const std::optional<unsigned int> evaluatedBitRangeStart = bitRangeStart && bitRangeStart->isConstant() ? bitRangeStart->tryEvaluate({}) : std::nullopt; evaluatedBitRangeStart.has_value()) {
            bitRangeStartValidity.indexValidity = isIndexInRange(*evaluatedBitRangeStart, variableAccess.getVar()->bitwidth);
            bitRangeStartValidity.indexValue    = evaluatedBitRangeStart;
        }

        if (const std::optional<unsigned int> evaluatedBitRangeEnd = bitRangeEnd && bitRangeEnd->isConstant() ? bitRangeEnd->tryEvaluate({}) : std::nullopt; evaluatedBitRangeEnd.has_value()) {
            bitRangeStartValidity.indexValidity = bitRangeStartValidity.indexValidity = isIndexInRange(*evaluatedBitRangeEnd, variableAccess.getVar()->bitwidth);
            bitRangeStartValidity.indexValue                                          = evaluatedBitRangeEnd;
        }
        validityOfVariableAccessIndices.bitRangeAccessValidity = VariableAccessIndicesValidity::BitRangeValidityResult({bitRangeStartValidity, bitRangeEndValidity});
    }
    return validityOfVariableAccessIndices;
}