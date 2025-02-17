#include "core/syrec/expression.hpp"

#include <core/syrec/parser/utils/variable_overlap_check.hpp>

using namespace utils;

namespace {
    std::pair<unsigned int, unsigned int> determineBitrangeConstantIndexPairOrderedAscendingly(unsigned int bitrangeStartIndexValue, unsigned int bitrangeEndIndexValue) {
        return bitrangeStartIndexValue <= bitrangeEndIndexValue ? std::make_pair(bitrangeStartIndexValue, bitrangeEndIndexValue) : std::make_pair(bitrangeEndIndexValue, bitrangeStartIndexValue);
    }
}

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

std::string VariableAccessOverlapCheckResult::stringifyOverlappingIndicesInformation() const {
    std::string stringificationBuffer;
    for (std::size_t dimensionIdx = 0; dimensionIdx < overlappingIndicesInformations->knownValueOfAccessedValuePerDimension.size(); ++dimensionIdx)
        stringificationBuffer += "(" + std::to_string(dimensionIdx) + "," + std::to_string(overlappingIndicesInformations->knownValueOfAccessedValuePerDimension.at(dimensionIdx)) + ")";   
    stringificationBuffer += "| " + std::to_string(overlappingIndicesInformations->overlappingBit);
    return stringificationBuffer;
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
    // Either the bitrange start or end index of the lhs variable access has a constant value
    if (evaluatedLVarBitRangeStart.has_value() != evaluatedLVarBitRangeEnd.has_value()) {
        const auto lVarKnownBitIndexValue = evaluatedLVarBitRangeStart.has_value() ? *evaluatedLVarBitRangeStart : *evaluatedLVarBitRangeEnd;
        // Either the bitrange start or end index of the rhs variable access has a constant value
        if (evaluatedRVarBitRangeStart.has_value() != evaluatedRVarBitRangeEnd.has_value()) {
            const auto rVarKnonwBitIndexValue = evaluatedRVarBitRangeStart.has_value() ? *evaluatedRVarBitRangeStart : *evaluatedRVarBitRangeEnd;
            if (lVarKnownBitIndexValue == rVarKnonwBitIndexValue)
                overlappingBit = lVarKnownBitIndexValue;
        } else if (evaluatedRVarBitRangeStart.has_value()) {
            // Both bitrange start and end index of the rhs variable access have a constant value
            const std::pair<unsigned int, unsigned int> orderedIndicesOfBitrangeAccessOfRVar = determineBitrangeConstantIndexPairOrderedAscendingly(*evaluatedRVarBitRangeStart, *evaluatedRVarBitRangeEnd);
            if (orderedIndicesOfBitrangeAccessOfRVar.first <= lVarKnownBitIndexValue && lVarKnownBitIndexValue <= orderedIndicesOfBitrangeAccessOfRVar.second)
                overlappingBit = lVarKnownBitIndexValue;
        }
    } else if (evaluatedLVarBitRangeStart.has_value()) {
        // Either the bitrange start or end index of the rhs variable access has a constant value
        if (evaluatedRVarBitRangeStart.has_value() != evaluatedRVarBitRangeEnd.has_value()) {
            const auto rVarKnownBitIndexValue = evaluatedRVarBitRangeStart.has_value() ? *evaluatedRVarBitRangeStart : *evaluatedRVarBitRangeEnd;
            const std::pair<unsigned int, unsigned int> orderedIndicesOfBitrangeAccessOfLVar = determineBitrangeConstantIndexPairOrderedAscendingly(*evaluatedLVarBitRangeStart, *evaluatedLVarBitRangeEnd);
            if (orderedIndicesOfBitrangeAccessOfLVar.first <= rVarKnownBitIndexValue && rVarKnownBitIndexValue <= orderedIndicesOfBitrangeAccessOfLVar.second)
                overlappingBit = rVarKnownBitIndexValue;
        } else if (evaluatedRVarBitRangeStart.has_value()) {
            // The indices of the accessed bitrange in both operands have a constant value
            const std::pair<unsigned int, unsigned int> orderedIndicesOfBitrangeAccessOfRVar = determineBitrangeConstantIndexPairOrderedAscendingly(*evaluatedRVarBitRangeStart, *evaluatedRVarBitRangeEnd);
            const std::pair<unsigned int, unsigned int> orderedIndicesOfBitrangeAccessOfLVar = determineBitrangeConstantIndexPairOrderedAscendingly(*evaluatedLVarBitRangeStart, *evaluatedLVarBitRangeEnd);

            if ((orderedIndicesOfBitrangeAccessOfLVar.first < orderedIndicesOfBitrangeAccessOfRVar.first && orderedIndicesOfBitrangeAccessOfLVar.second < orderedIndicesOfBitrangeAccessOfRVar.first) || (orderedIndicesOfBitrangeAccessOfLVar.first > orderedIndicesOfBitrangeAccessOfRVar.first && orderedIndicesOfBitrangeAccessOfLVar.first > orderedIndicesOfBitrangeAccessOfRVar.second))
                return VariableAccessOverlapCheckResult(VariableAccessOverlapCheckResult::OverlapState::NotOverlapping);

            const std::pair<unsigned int, unsigned int> accessedBitRangeOfLVar    = std::make_pair(*evaluatedLVarBitRangeStart, *evaluatedLVarBitRangeEnd);
            if (accessedBitRangeOfLVar.first < accessedBitRangeOfLVar.second) {
                overlappingBit = accessedBitRangeOfLVar.first > orderedIndicesOfBitrangeAccessOfRVar.first ? accessedBitRangeOfLVar.first : orderedIndicesOfBitrangeAccessOfRVar.first;
            } else if (accessedBitRangeOfLVar.first > accessedBitRangeOfLVar.second) {
                overlappingBit = accessedBitRangeOfLVar.first < orderedIndicesOfBitrangeAccessOfRVar.second ? accessedBitRangeOfLVar.first : orderedIndicesOfBitrangeAccessOfRVar.second;
            } else {
                overlappingBit = accessedBitRangeOfLVar.first;
            }
        }
    }

    if (!overlappingBit.has_value())
        return VariableAccessOverlapCheckResult(VariableAccessOverlapCheckResult::OverlapState::MaybeOverlapping);

    auto overlapCheckResultContainer                           = VariableAccessOverlapCheckResult(VariableAccessOverlapCheckResult::OverlapState::Overlapping);
    overlapCheckResultContainer.overlappingIndicesInformations = VariableAccessOverlapCheckResult::OverlappingIndicesContainer({constantIndicesOfAccessedValuesPerDimension, *overlappingBit});
    return overlapCheckResultContainer;
}