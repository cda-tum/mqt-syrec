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

bool doReferenceVariablesMatch(const syrec::Variable& lVarReference, const syrec::Variable& rVarReference) noexcept {
    return lVarReference.name == rVarReference.name
        && lVarReference.bitwidth == rVarReference.bitwidth
        && std::equal(lVarReference.dimensions.cbegin(), lVarReference.dimensions.cend(), rVarReference.dimensions.cbegin(), rVarReference.dimensions.cend());
}

[[nodiscard]] std::optional<VariableAccessOverlapCheckResult> utils::checkOverlapBetweenVariableAccesses(const syrec::VariableAccess& lVariableAccess, const syrec::VariableAccess& rVariableAccess) {
    // TODO: syrec::VariableAccess getVar(...) will crash if the variable smart pointer is not set due to the function accessing var->reference
    // I.   Why does the variable need a smart pointer member field to a syrec::Variable instance
    // II.  Why does the getVar(...) call access this smart pointer instance instead of simply returning syrec::VariableAccess var member?
    const syrec::Variable::ptr& lVarPtr = lVariableAccess.var;
    const syrec::Variable::ptr& rVarPtr = rVariableAccess.var;
    if (!lVarPtr || !rVarPtr || !doReferenceVariablesMatch(*lVarPtr, *rVarPtr))
        return std::nullopt;

    const syrec::Variable& lVar = *lVarPtr;
    const syrec::Variable& rVar = *rVarPtr;

    const std::size_t numDimensionsToCheck = std::min(
        std::min(lVar.dimensions.size(), lVariableAccess.indexes.size()), 
        std::min(rVar.dimensions.size(), rVariableAccess.indexes.size())
    );
    std::vector<unsigned int> constantIndicesOfAccessedValuesPerDimension;
    if (!numDimensionsToCheck)
        constantIndicesOfAccessedValuesPerDimension.emplace_back(0);
    else
        constantIndicesOfAccessedValuesPerDimension.reserve(numDimensionsToCheck);

    for (std::size_t i = 0; i < numDimensionsToCheck; ++i) {
        const auto& exprDefiningAccessedValueOfDimensionInLVar = lVariableAccess.indexes.at(i);
        const auto& exprDefiningAccessedValueOfDimensionInRVar = rVariableAccess.indexes.at(i);
        if (!exprDefiningAccessedValueOfDimensionInLVar || !exprDefiningAccessedValueOfDimensionInRVar)
            return VariableAccessOverlapCheckResult(VariableAccessOverlapCheckResult::OverlapState::MaybeOverlapping);

        const std::optional<unsigned int> accessedValueInLVar = tryEvaluateNumericExpr(exprDefiningAccessedValueOfDimensionInLVar);
        const std::optional<unsigned int> accessedValueInRVar = tryEvaluateNumericExpr(exprDefiningAccessedValueOfDimensionInRVar);

        if (!accessedValueInLVar.has_value() || !accessedValueInRVar.has_value())
            return VariableAccessOverlapCheckResult(VariableAccessOverlapCheckResult::OverlapState::MaybeOverlapping);

        if (accessedValueInLVar.value() != accessedValueInRVar.value())
            return VariableAccessOverlapCheckResult(VariableAccessOverlapCheckResult::OverlapState::NotOverlapping);
        constantIndicesOfAccessedValuesPerDimension.emplace_back(accessedValueInLVar.value());
    }

    // The caller does not need to explicitly define the accessed bit range in the variable access if he wishes to access the whole variable bitwidth
    std::optional<unsigned int> evaluatedLVarBitRangeStart = 0;
    std::optional               evaluatedLVarBitRangeEnd   = lVariableAccess.var->bitwidth - 1;
    if (lVariableAccess.range.has_value()) {
        evaluatedLVarBitRangeStart = tryEvaluateNumber(lVariableAccess.range->first);
        evaluatedLVarBitRangeEnd   = lVariableAccess.range->first == lVariableAccess.range->second ? evaluatedLVarBitRangeStart : tryEvaluateNumber(lVariableAccess.range->second);
    }

    std::optional<unsigned int> evaluatedRVarBitRangeStart = 0;
    std::optional               evaluatedRVarBitRangeEnd   = rVariableAccess.var->bitwidth - 1;
    if (rVariableAccess.range.has_value()) {
        evaluatedRVarBitRangeStart = tryEvaluateNumber(rVariableAccess.range->first);
        evaluatedRVarBitRangeEnd   = rVariableAccess.range->first == rVariableAccess.range->second ? evaluatedRVarBitRangeStart : tryEvaluateNumber(rVariableAccess.range->second);
    }

    if ((!evaluatedLVarBitRangeStart.has_value() && !evaluatedLVarBitRangeEnd.has_value()) || (!evaluatedRVarBitRangeStart.has_value() && !evaluatedRVarBitRangeEnd.has_value()))
        return VariableAccessOverlapCheckResult(VariableAccessOverlapCheckResult::OverlapState::MaybeOverlapping);

    std::optional<unsigned int> overlappingBit;
    // Handle lVar bitrange with only one known index value
    if (evaluatedLVarBitRangeStart.has_value() != evaluatedLVarBitRangeEnd.has_value()) {
        const unsigned int lVarKnownBitIndex = evaluatedLVarBitRangeStart.has_value() ? *evaluatedLVarBitRangeStart : *evaluatedLVarBitRangeEnd;
        // Check for overlap of lVar index with rVar bitrange with only one known index
        if (evaluatedRVarBitRangeStart.has_value() != evaluatedRVarBitRangeEnd.has_value()) {
            const unsigned int rVarKnownBitIndex = evaluatedRVarBitRangeStart.has_value() ? *evaluatedRVarBitRangeStart : *evaluatedRVarBitRangeEnd;
            if (lVarKnownBitIndex == rVarKnownBitIndex)
                overlappingBit = lVarKnownBitIndex;
        } else {
            const std::pair<unsigned int, unsigned int> orderedBitrangeOfRVar = *evaluatedRVarBitRangeStart <= *evaluatedRVarBitRangeEnd
                ? std::make_pair(*evaluatedRVarBitRangeStart, *evaluatedRVarBitRangeEnd)
                : std::make_pair(*evaluatedRVarBitRangeEnd, *evaluatedRVarBitRangeStart);

            if (orderedBitrangeOfRVar.first <= lVarKnownBitIndex && lVarKnownBitIndex <= orderedBitrangeOfRVar.second)
                overlappingBit = lVarKnownBitIndex;
        }
    } else {
        // Handle lVar bitrange overlap check with rVar bit range with only one known index value
        if (evaluatedRVarBitRangeStart.has_value() != evaluatedRVarBitRangeEnd.has_value()) {
            const unsigned int rVarKnownBitIndex = evaluatedRVarBitRangeStart.has_value() ? *evaluatedRVarBitRangeStart : *evaluatedRVarBitRangeEnd;
            if (*evaluatedLVarBitRangeStart >= rVarKnownBitIndex || rVarKnownBitIndex <= *evaluatedLVarBitRangeEnd)
                overlappingBit = rVarKnownBitIndex;
        } else {
            // Handle bit range intersections between variable accesses with known values for both the bitrange start and end index.
            // To support both bit range variants (I. start > end && II. end < start) we reorder both variants to the I. variant for easier handling.
            const std::pair<unsigned int, unsigned int> orderedBitrangeOfLVar = *evaluatedLVarBitRangeStart <= *evaluatedLVarBitRangeEnd
                ? std::make_pair(*evaluatedLVarBitRangeStart, *evaluatedLVarBitRangeEnd)
                : std::make_pair(*evaluatedLVarBitRangeEnd, *evaluatedLVarBitRangeStart);

            const std::pair<unsigned int, unsigned int> orderedBitrangeOfRVar = *evaluatedRVarBitRangeStart <= *evaluatedRVarBitRangeEnd
                ? std::make_pair(*evaluatedRVarBitRangeStart, *evaluatedRVarBitRangeEnd)
                : std::make_pair(*evaluatedRVarBitRangeEnd, *evaluatedRVarBitRangeStart);

            if ((orderedBitrangeOfLVar.first < orderedBitrangeOfRVar.first && orderedBitrangeOfLVar.second < orderedBitrangeOfRVar.first) 
                || (orderedBitrangeOfLVar.first > orderedBitrangeOfRVar.first && orderedBitrangeOfLVar.first > orderedBitrangeOfRVar.second))
                return VariableAccessOverlapCheckResult(VariableAccessOverlapCheckResult::OverlapState::NotOverlapping);

            const std::pair<unsigned int, unsigned int> accessedBitRangeOfLVar = std::make_pair(*evaluatedLVarBitRangeStart, *evaluatedLVarBitRangeEnd);
            if (accessedBitRangeOfLVar.first > accessedBitRangeOfLVar.second)
                overlappingBit = orderedBitrangeOfRVar.second <= accessedBitRangeOfLVar.first ? orderedBitrangeOfRVar.second : accessedBitRangeOfLVar.first;
            else
                overlappingBit = orderedBitrangeOfRVar.first >= accessedBitRangeOfLVar.first ? orderedBitrangeOfRVar.first : accessedBitRangeOfLVar.first;
        }
    }

    if (!overlappingBit.has_value())
        return VariableAccessOverlapCheckResult(VariableAccessOverlapCheckResult::OverlapState::MaybeOverlapping);

    auto overlapCheckResultContainer                           = VariableAccessOverlapCheckResult(VariableAccessOverlapCheckResult::OverlapState::Overlapping);
    overlapCheckResultContainer.overlappingIndicesInformations = VariableAccessOverlapCheckResult::OverlappingIndicesContainer({constantIndicesOfAccessedValuesPerDimension, *overlappingBit});
    return overlapCheckResultContainer;
}