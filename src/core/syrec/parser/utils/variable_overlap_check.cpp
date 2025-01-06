#include "core/syrec/expression.hpp"

#include <core/syrec/parser/utils/variable_overlap_check.hpp>

using namespace utils;

std::optional<unsigned int> tryEvaluateNumber(const syrec::Number::ptr& numberToEvaluate) {
    return numberToEvaluate && numberToEvaluate->isConstant() ? numberToEvaluate->tryEvaluate({}) : std::nullopt;
}

std::optional<unsigned int> tryEvaluateNumericExpr(const syrec::Expression::ptr& expression) {
    if (const auto& numericExprOfContainerToEvaluate = std::dynamic_pointer_cast<syrec::NumericExpression>(expression); numericExprOfContainerToEvaluate)
        return tryEvaluateNumber(numericExprOfContainerToEvaluate->value);

    return std::nullopt;
}

inline bool doReferenceVariablesMatch(const syrec::Variable& lVarReference, const syrec::Variable& rVarReference) noexcept {
    return lVarReference.name == rVarReference.name
        && lVarReference.bitwidth == rVarReference.bitwidth
        && std::equal(lVarReference.dimensions.cbegin(), lVarReference.dimensions.cend(), rVarReference.dimensions.cbegin(), rVarReference.dimensions.cend());
}

[[nodiscard]] std::optional<VariableAccessOverlapCheckResult> utils::checkOverlapBetweenVariableAccesses(const syrec::VariableAccess& lVariableAccess, const syrec::VariableAccess& rVariableAccess) {
    const syrec::Variable::ptr& lVarPtr = lVariableAccess.getVar();
    const syrec::Variable::ptr& rVarPtr = rVariableAccess.getVar();
    if (!lVarPtr || !rVarPtr || !doReferenceVariablesMatch(*lVarPtr, *rVarPtr))
        return std::nullopt;

    const syrec::Variable& lVar = *lVarPtr;
    const syrec::Variable& rVar = *rVarPtr;

    const std::size_t numDimensionsToCheck = std::min(
        std::min(lVar.dimensions.size(), lVariableAccess.indexes.size()), 
        std::min(rVar.dimensions.size(), rVariableAccess.indexes.size())
    );
    for (std::size_t i = 0; i < numDimensionsToCheck; ++i) {
        const auto& exprDefiningAccessedValueOfDimensionInLVar = lVariableAccess.indexes.at(i);
        const auto& exprDefiningAccessedValueOfDimensionInRVar = rVariableAccess.indexes.at(i);
        if (!exprDefiningAccessedValueOfDimensionInLVar || !exprDefiningAccessedValueOfDimensionInRVar)
            return VariableAccessOverlapCheckResult::MaybeOverlapping;

        const std::optional<unsigned int> accessedValueInLVar = tryEvaluateNumericExpr(exprDefiningAccessedValueOfDimensionInLVar);
        const std::optional<unsigned int> accessedValueInRVar = tryEvaluateNumericExpr(exprDefiningAccessedValueOfDimensionInRVar);
        
        if (!accessedValueInLVar.has_value() || !accessedValueInRVar.has_value())
            return VariableAccessOverlapCheckResult::MaybeOverlapping;
        if (accessedValueInLVar.value() != accessedValueInRVar.value())
            return VariableAccessOverlapCheckResult::NotOverlapping;
    }

    if (!(lVariableAccess.range.has_value() && rVariableAccess.range.has_value()))
        return VariableAccessOverlapCheckResult::Overlapping;

    const auto& [lVarBitrangeStart, lVarBitrangeEnd] = lVariableAccess.range.value();
    const auto& [rVarBitrangeStart, rVarBitrangeEnd] = rVariableAccess.range.value();

    // We are assuming that even a bit access has both the start and end component set (with a bit access being identified by the associated smart pointers being equal).
    // Whether our distincation between bit and bitrange access using the smart pointers should be swapped with an actual compare of their values needs to be defined.
    // However, the stringifiction utility class also utilizes the same behaviour.
    if (!lVarBitrangeEnd || !rVarBitrangeEnd)
        return VariableAccessOverlapCheckResult::MaybeOverlapping;

    const std::optional<unsigned int> evaluatedLVarBitRangeStart = tryEvaluateNumber(lVarBitrangeStart);
    const std::optional<unsigned int> evaluatedRVarBitRangeStart = tryEvaluateNumber(rVarBitrangeStart);
    if (!evaluatedLVarBitRangeStart.has_value() || !evaluatedRVarBitRangeStart.has_value())
        return VariableAccessOverlapCheckResult::MaybeOverlapping;

    const std::optional<unsigned int> evaluatedLVarBitRangeEnd = lVarBitrangeStart == lVarBitrangeEnd ? evaluatedLVarBitRangeStart : tryEvaluateNumber(lVarBitrangeEnd);
    const std::optional<unsigned int> evaluatedRVarBitRangeEnd = rVarBitrangeStart == rVarBitrangeEnd ? evaluatedRVarBitRangeStart : tryEvaluateNumber(rVarBitrangeEnd);
    if (!evaluatedLVarBitRangeEnd.has_value() || !evaluatedRVarBitRangeEnd.has_value())
        return VariableAccessOverlapCheckResult::MaybeOverlapping;

    // To support both bit range variants (I. start > end && II. end < start) we reorder both variants to the I. variant for easier handling.
    const std::pair<unsigned int, unsigned int> orderedBitrangeOfLVar = *evaluatedLVarBitRangeStart <= *evaluatedLVarBitRangeEnd
        ? std::make_pair(*evaluatedLVarBitRangeStart, *evaluatedLVarBitRangeEnd)
        : std::make_pair(*evaluatedLVarBitRangeEnd, *evaluatedLVarBitRangeStart);

    const std::pair<unsigned int, unsigned int> orderedBitrangeOfRVar = *evaluatedRVarBitRangeStart <= *evaluatedRVarBitRangeEnd
        ? std::make_pair(*evaluatedRVarBitRangeStart, *evaluatedRVarBitRangeEnd)
        : std::make_pair(*evaluatedRVarBitRangeEnd, *evaluatedRVarBitRangeStart);

    if ((orderedBitrangeOfLVar.first < orderedBitrangeOfRVar.first && orderedBitrangeOfLVar.second < orderedBitrangeOfRVar.first) 
        || (orderedBitrangeOfLVar.first > orderedBitrangeOfRVar.first && orderedBitrangeOfLVar.first > orderedBitrangeOfRVar.second))
        return VariableAccessOverlapCheckResult::NotOverlapping;
    
    return VariableAccessOverlapCheckResult::Overlapping;
}