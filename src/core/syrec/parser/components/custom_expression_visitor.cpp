#include "core/syrec/parser/components/custom_expression_visitor.hpp"

#include "core/syrec/expression.hpp"
#include "core/syrec/parser/utils/variable_access_index_check.hpp"
#include "core/syrec/parser/utils/symbolTable/temporary_variable_scope.hpp"
#include "core/syrec/parser/utils/syrec_operation_utils.hpp"

// TODO: Truncation of values in expressions where a signal access is defined in a nested expression needs to be propagated to the 'past' operands

using namespace syrecParser;

std::optional<syrec::Expression::ptr> CustomExpressionVisitor::visitExpressionTyped(TSyrecParser::ExpressionContext* context) {
    if (!context)
        return std::nullopt;

    if (const auto binaryExpressionContext = dynamic_cast<TSyrecParser::ExpressionFromBinaryExpressionContext*>(context))
        return visitBinaryExpressionTyped(binaryExpressionContext->binaryExpression());
    if (const auto shiftExpressionContext = dynamic_cast<TSyrecParser::ExpressionFromShiftExpressionContext*>(context))
        return visitShiftExpressionTyped(shiftExpressionContext->shiftExpression());
    if (const auto unaryExpressionContext = dynamic_cast<TSyrecParser::ExpressionFromUnaryExpressionContext*>(context))
        return visitUnaryExpressionTyped(unaryExpressionContext->unaryExpression());
    if (const auto expressionFromNumberContext = dynamic_cast<TSyrecParser::ExpressionFromNumberContext*>(context))
        return visitExpressionFromNumberTyped(expressionFromNumberContext);
    if (const auto expressionFromSignalContext = dynamic_cast<TSyrecParser::ExpressionFromSignalContext*>(context))
        return visitExpressionFromSignalTyped(expressionFromSignalContext);

    recordCustomError(Message::Position(0, 0), "Unhandled expression context variant. This should not happen");
    return std::nullopt;
}

std::optional<syrec::Expression::ptr> CustomExpressionVisitor::visitBinaryExpressionTyped(const TSyrecParser::BinaryExpressionContext* context) {
    if (!context)
        return std::nullopt;

    const std::optional<syrec::Expression::ptr>                   lhsOperand              = visitExpressionTyped(context->lhsOperand);
    const std::optional<syrec::BinaryExpression::BinaryOperation> mappedToBinaryOperation = context->binaryOperation ? deserializeBinaryOperationFromString(context->binaryOperation->getText()) : std::nullopt;
    const std::optional<syrec::Expression::ptr>                   rhsOperand              = visitExpressionTyped(context->rhsOperand);

    if (!mappedToBinaryOperation.has_value())
        return std::nullopt;

    const std::optional<unsigned int> constantValueOfLhsOperand = lhsOperand.has_value() && *lhsOperand ? tryGetConstantValueOfExpression(**lhsOperand) : std::nullopt;
    const std::optional<unsigned int> constantValueOfRhsOperand = rhsOperand.has_value() && *rhsOperand ? tryGetConstantValueOfExpression(**rhsOperand) : std::nullopt;

    if (*mappedToBinaryOperation == syrec::BinaryExpression::BinaryOperation::Divide && ((constantValueOfLhsOperand.has_value() && !*constantValueOfLhsOperand) || (constantValueOfRhsOperand.has_value() && !*constantValueOfRhsOperand)))
        recordSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(mapTokenPositionToMessagePosition(*(context->lhsOperand ? context->lhsOperand->getStart() : context->rhsOperand->getStart())));
    else if (constantValueOfLhsOperand.has_value() && constantValueOfRhsOperand.has_value()) {
        if (const std::optional<unsigned int> evaluationResultOfExpr = utils::tryEvaluate(constantValueOfLhsOperand, *mappedToBinaryOperation, constantValueOfRhsOperand, optionalExpectedBitwidthForAnyProcessedEntity.value_or(DEFAULT_EXPRESSION_BITWIDTH)); evaluationResultOfExpr.has_value())
            return std::make_shared<syrec::NumericExpression>(std::make_shared<syrec::Number>(*evaluationResultOfExpr), optionalExpectedBitwidthForAnyProcessedEntity.value_or(DEFAULT_EXPRESSION_BITWIDTH));
    }

    return lhsOperand.has_value() && rhsOperand.has_value() ? std::make_optional(std::make_shared<syrec::BinaryExpression>(*lhsOperand, *mappedToBinaryOperation, *rhsOperand)) : std::nullopt;
}

std::optional<syrec::Expression::ptr> CustomExpressionVisitor::visitShiftExpressionTyped(TSyrecParser::ShiftExpressionContext* context) {
    if (!context)
        return std::nullopt;

    std::optional<syrec::Expression::ptr>                       toBeShiftedOperand     = visitExpressionTyped(context->expression());
    const std::optional<syrec::ShiftExpression::ShiftOperation> mappedToShiftOperation = context->shiftOperation ? deserializeShiftOperationFromString(context->shiftOperation->getText()) : std::nullopt;
    const std::optional<syrec::Number::ptr>                     shiftAmount            = visitNumberTyped(context->number());

    if (!mappedToShiftOperation.has_value())
        return std::nullopt;

    const std::optional<unsigned int> constantValueOfToBeShiftedOperand = toBeShiftedOperand.has_value() && *toBeShiftedOperand ? tryGetConstantValueOfExpression(**toBeShiftedOperand) : std::nullopt;
    const std::optional<unsigned int> constantValueOfShiftAmount        = shiftAmount.has_value() && *shiftAmount ? shiftAmount.value()->tryEvaluate({}) : std::nullopt;

    if (constantValueOfShiftAmount.has_value()) {
        if (!*constantValueOfShiftAmount)
            return toBeShiftedOperand.has_value() ? std::make_optional(*toBeShiftedOperand) : std::nullopt;
        if (constantValueOfToBeShiftedOperand.has_value()) {
            if (const std::optional<unsigned int> evaluationResultOfShiftOperation = utils::tryEvaluate(constantValueOfToBeShiftedOperand, *mappedToShiftOperation, constantValueOfShiftAmount, optionalExpectedBitwidthForAnyProcessedEntity.value_or(DEFAULT_EXPRESSION_BITWIDTH)); evaluationResultOfShiftOperation.has_value())
                return std::make_shared<syrec::NumericExpression>(std::make_shared<syrec::Number>(*evaluationResultOfShiftOperation), optionalExpectedBitwidthForAnyProcessedEntity.value_or(DEFAULT_EXPRESSION_BITWIDTH));
        }
    }

    return toBeShiftedOperand.has_value() && shiftAmount.has_value() ? std::make_optional(std::make_shared<syrec::ShiftExpression>(*toBeShiftedOperand, *mappedToShiftOperation, *shiftAmount)) : std::nullopt;
}

std::optional<syrec::Expression::ptr> CustomExpressionVisitor::visitUnaryExpressionTyped(const TSyrecParser::UnaryExpressionContext* context) const {
    if (context && context->start)
        recordCustomError(mapTokenPositionToMessagePosition(*context->start), "Unary expressions are currently not supported");
    return std::nullopt;
}

std::optional<syrec::Expression::ptr> CustomExpressionVisitor::visitExpressionFromNumberTyped(TSyrecParser::ExpressionFromNumberContext* context) const {
    // TODO: Bitwidth of expression
    if (const auto& generatedNumberContainer = context ? visitNumberTyped(context->number()) : std::nullopt; generatedNumberContainer.has_value())
        return std::make_shared<syrec::NumericExpression>(*generatedNumberContainer, optionalExpectedBitwidthForAnyProcessedEntity.value_or(DEFAULT_EXPRESSION_BITWIDTH));
    return std::nullopt;
}

std::optional<syrec::Expression::ptr> CustomExpressionVisitor::visitExpressionFromSignalTyped(TSyrecParser::ExpressionFromSignalContext* context) {
    if (const auto& generatedSignalContainer = context ? visitSignalTyped(context->signal()) : std::nullopt; generatedSignalContainer.has_value())
        return std::make_shared<syrec::VariableExpression>(*generatedSignalContainer);
    return std::nullopt;
}

std::optional<syrec::Number::ptr> CustomExpressionVisitor::visitNumberTyped(TSyrecParser::NumberContext* context) const {
    if (!context)
        return std::nullopt;

    if (const auto numberFromConstantContext = dynamic_cast<TSyrecParser::NumberFromConstantContext*>(context))
        return visitNumberFromConstantTyped(numberFromConstantContext);
    if (const auto& numberFromExpressionContext = dynamic_cast<TSyrecParser::NumberFromExpressionContext*>(context))
        return visitNumberFromExpressionTyped(numberFromExpressionContext);
    if (const auto& numberFromLoopVariableContext = dynamic_cast<TSyrecParser::NumberFromLoopVariableContext*>(context))
        return visitNumberFromLoopVariableTyped(numberFromLoopVariableContext);
    if (const auto& numberFromSignalWitdhContext = dynamic_cast<TSyrecParser::NumberFromSignalwidthContext*>(context))
        return visitNumberFromSignalwidthTyped(numberFromSignalWitdhContext);

    recordCustomError(Message::Position(0, 0), "Unhandled number context variant. This should not happen");
    return std::nullopt;
}

std::optional<syrec::Number::ptr> CustomExpressionVisitor::visitNumberFromConstantTyped(TSyrecParser::NumberFromConstantContext* context) const {
    // TODO: Check these assumptions
    // Production should only be called if the token contains only numeric characters and thus deserialization should only fail if an overflow occurs.
    // Leading and trailing whitespace should also be trimmed from the token text by the parser.
    if (!context || !context->INT())
        return std::nullopt;

    if (const std::optional<unsigned int> constantValue = deserializeConstantFromString(context->INT()->getText(), nullptr); constantValue.has_value()) {
        if (optionalExpectedBitwidthForAnyProcessedEntity.has_value())
            return std::make_shared<syrec::Number>(utils::truncateConstantValueToExpectedBitwidth(*constantValue, *optionalExpectedBitwidthForAnyProcessedEntity));
        return std::make_shared<syrec::Number>(*constantValue);
    }
    return std::nullopt;
}

std::optional<syrec::Number::ptr> CustomExpressionVisitor::visitNumberFromExpressionTyped(const TSyrecParser::NumberFromExpressionContext* context) const {
    if (!context)
        return std::nullopt;

    recordCustomError(mapTokenPositionToMessagePosition(*context->start), "Compile time constant expressions are not supported");
    return std::nullopt;
}

// TODO: Check that loop variable and 'normal' variable can share same identfier since the former is distinguished by the loop variable prefix '$'
std::optional<syrec::Number::ptr> CustomExpressionVisitor::visitNumberFromLoopVariableTyped(TSyrecParser::NumberFromLoopVariableContext* context) const {
    if (!context || !context->IDENT())
        return std::nullopt;

    const std::optional<utils::TemporaryVariableScope::ptr> activeVariableScopeInSymbolTable = symbolTable->getActiveTemporaryScope();
    if (!activeVariableScopeInSymbolTable)
        return std::nullopt;

    const std::string loopVariableIdentifier = "$" + context->IDENT()->getText();
    if (const std::optional<utils::TemporaryVariableScope::ScopeEntry::readOnylPtr> matchingLoopVariableForIdentifier = activeVariableScopeInSymbolTable->get()->getVariableByName(loopVariableIdentifier); matchingLoopVariableForIdentifier.has_value() && matchingLoopVariableForIdentifier->get()->isReferenceToLoopVariable()) {
        if (optionalRestrictionOnLoopVariableUsageInLoopVariableValueInitialization.has_value() && optionalRestrictionOnLoopVariableUsageInLoopVariableValueInitialization.value() == loopVariableIdentifier)
            recordSemanticError<SemanticError::ValueOfLoopVariableNotUsableInItsInitialValueDeclaration>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()), loopVariableIdentifier);
        return std::make_shared<syrec::Number>(loopVariableIdentifier);
    }
    recordSemanticError<SemanticError::NoVariableMatchingIdentifier>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()), loopVariableIdentifier);
    return std::nullopt;
}

// TODO: Tests that bitwidth of loop variable cannot be fetched (results in syntax error)
std::optional<syrec::Number::ptr> CustomExpressionVisitor::visitNumberFromSignalwidthTyped(TSyrecParser::NumberFromSignalwidthContext* context) const {
    if (!context || !context->IDENT())
        return std::nullopt;

    const std::optional<utils::TemporaryVariableScope::ptr> activeVariableScopeInSymbolTable = symbolTable->getActiveTemporaryScope();
    if (!activeVariableScopeInSymbolTable)
        return std::nullopt;

    const std::string& variableIdentifier = context->IDENT()->getSymbol()->getText();
    if (const std::optional<utils::TemporaryVariableScope::ScopeEntry::readOnylPtr> matchingVariableForIdentifier = activeVariableScopeInSymbolTable->get()->getVariableByName(variableIdentifier); matchingVariableForIdentifier.has_value()) {
        return optionalExpectedBitwidthForAnyProcessedEntity.has_value() && matchingVariableForIdentifier->get()->getDeclaredVariableBitwidth().has_value() ? std::make_shared<syrec::Number>(utils::truncateConstantValueToExpectedBitwidth(*matchingVariableForIdentifier->get()->getDeclaredVariableBitwidth(), *optionalExpectedBitwidthForAnyProcessedEntity)) : std::make_shared<syrec::Number>(*matchingVariableForIdentifier->get()->getDeclaredVariableBitwidth());
    }

    recordSemanticError<SemanticError::NoVariableMatchingIdentifier>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()), variableIdentifier);
    return std::nullopt;
}

// TODO: Substitution of loop variable values for compile time index checks
std::optional<syrec::VariableAccess::ptr> CustomExpressionVisitor::visitSignalTyped(TSyrecParser::SignalContext* context) {
    if (!context || !context->IDENT())
        return std::nullopt;

    const std::optional<utils::TemporaryVariableScope::ptr> activeVariableScopeInSymbolTable = symbolTable->getActiveTemporaryScope();
    if (!activeVariableScopeInSymbolTable)
        return std::nullopt;

    const std::string&                                                          variableIdentifier            = context->IDENT()->getSymbol()->getText();
    const std::optional<utils::TemporaryVariableScope::ScopeEntry::readOnylPtr> matchingVariableForIdentifier = activeVariableScopeInSymbolTable->get()->getVariableByName(variableIdentifier);
    if (!matchingVariableForIdentifier.has_value() || !matchingVariableForIdentifier.value()->getReadonlyVariableData().has_value()) {
        recordSemanticError<SemanticError::NoVariableMatchingIdentifier>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()), variableIdentifier);
        return std::nullopt;
    }

    syrec::VariableAccess::ptr generatedVariableAccess = std::make_shared<syrec::VariableAccess>();
    // TODO: Current shared_pointer to const T does not allow for assignment of variable to variable access
    generatedVariableAccess->setVar(std::make_shared<syrec::Variable>(**matchingVariableForIdentifier->get()->getReadonlyVariableData()));

    const std::vector<unsigned int>& declaredValuesPerDimensionOfReferenceVariable = matchingVariableForIdentifier.value()->getDeclaredVariableDimensions();
    const std::size_t                numUserAccessedDimensions                     = context->accessedDimensions.size();
    generatedVariableAccess->indexes                                               = syrec::Expression::vec(numUserAccessedDimensions, nullptr);
    for (const auto& antlrContextForAccessedValueOfDimension: context->accessedDimensions) {
        if (const std::optional<syrec::Expression::ptr> exprDefiningAccessedValueOfDimension = visitExpressionTyped(antlrContextForAccessedValueOfDimension); exprDefiningAccessedValueOfDimension.has_value())
            generatedVariableAccess->indexes.emplace_back(*exprDefiningAccessedValueOfDimension);
    }

    if (!numUserAccessedDimensions) {
        if (declaredValuesPerDimensionOfReferenceVariable.size() == 1 && declaredValuesPerDimensionOfReferenceVariable.front() == 1)
            generatedVariableAccess->indexes.emplace_back(std::make_shared<syrec::NumericExpression>(std::make_shared<syrec::Number>(0), 1));
        else
            recordSemanticError<SemanticError::OmittingDimensionAccessOnlyPossibleFor1DSignalWithSingleValue>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()));
    }
    else if (numUserAccessedDimensions != declaredValuesPerDimensionOfReferenceVariable.size()) {
        if (numUserAccessedDimensions > declaredValuesPerDimensionOfReferenceVariable.size())
            recordSemanticError<SemanticError::TooManyDimensionsAccessed>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()), numUserAccessedDimensions, declaredValuesPerDimensionOfReferenceVariable.size());
        else
            // Checking whether the dimension access of the variable was fully defined by the user prevents the propagation of non-1D signal values
            recordSemanticError<SemanticError::TooFewDimensionsAccessed>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()), numUserAccessedDimensions, declaredValuesPerDimensionOfReferenceVariable.size());
    }

    if (const std::optional<utils::VariableAccessIndicesValidity> indexValidityOfUserDefinedAccessedValuesPerDimension = utils::validateVariableAccessIndices(*generatedVariableAccess); indexValidityOfUserDefinedAccessedValuesPerDimension.has_value() && !indexValidityOfUserDefinedAccessedValuesPerDimension->isValid()) {
        const std::size_t numDimensionsToCheck = declaredValuesPerDimensionOfReferenceVariable.size();
        for (std::size_t dimensionIdx = 0; dimensionIdx < numDimensionsToCheck; ++dimensionIdx) {
            const utils::VariableAccessIndicesValidity::IndexValidationResult validityOfAccessedValueOfDimension = indexValidityOfUserDefinedAccessedValuesPerDimension->accessedValuePerDimensionValidity.at(dimensionIdx);
            // TODO: We should not have to check whether the index validation result for the given index contains a value when an out of range access is reported.
            if (validityOfAccessedValueOfDimension.indexValidity == utils::VariableAccessIndicesValidity::IndexValidationResult::OutOfRange && validityOfAccessedValueOfDimension.indexValue.has_value())
                recordSemanticError<SemanticError::IndexOfAccessedValueForDimensionOutOfRange>(mapTokenPositionToMessagePosition(*context->accessedDimensions.at(dimensionIdx)->getStart()), validityOfAccessedValueOfDimension.indexValue.value(), dimensionIdx, declaredValuesPerDimensionOfReferenceVariable.at(dimensionIdx));
        }
    }

    if (!context->BITRANGE_START_PREFIX())
        return generatedVariableAccess;

    const std::optional<syrec::Number::ptr> bitRangeStart = visitNumberTyped(context->bitStart);
    const std::optional<syrec::Number::ptr> bitRangeEnd   = visitNumberTyped(context->bitRangeEnd);

    bool wasBitrangeWithinRange = true;
    if (bitRangeStart.has_value() || bitRangeEnd.has_value()) {
        if (bitRangeStart.has_value() && bitRangeEnd.has_value())
            generatedVariableAccess->range = std::make_pair(bitRangeStart.value(), bitRangeEnd.value());
        else if (bitRangeStart.has_value())
            generatedVariableAccess->range = std::make_pair(bitRangeStart.value(), bitRangeStart.value());
        else
            generatedVariableAccess->range = std::make_pair(bitRangeEnd.value(), bitRangeEnd.value());

        syrec::VariableAccess temporaryVariableAccess = syrec::VariableAccess();
        temporaryVariableAccess.setVar(generatedVariableAccess->var);
        temporaryVariableAccess.range = generatedVariableAccess->range;

        if (const std::optional<utils::VariableAccessIndicesValidity> indexValidityOfUserDefinedAccessOnBitrange = utils::validateVariableAccessIndices(temporaryVariableAccess); indexValidityOfUserDefinedAccessOnBitrange.has_value() && !indexValidityOfUserDefinedAccessOnBitrange->isValid() && indexValidityOfUserDefinedAccessOnBitrange->bitRangeAccessValidity.has_value()) {
            if (bitRangeStart.has_value() && bitRangeEnd.has_value()) {
                if (const utils::VariableAccessIndicesValidity::IndexValidationResult accessedBitRangeStartIndexValidity = indexValidityOfUserDefinedAccessOnBitrange->bitRangeAccessValidity->bitRangeStartValidity; 
                    accessedBitRangeStartIndexValidity.indexValidity == utils::VariableAccessIndicesValidity::IndexValidationResult::OutOfRange && accessedBitRangeStartIndexValidity.indexValue.has_value()) {
                    recordSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(mapTokenPositionToMessagePosition(*context->bitStart->getStart()), accessedBitRangeStartIndexValidity.indexValue.value(), generatedVariableAccess->var->bitwidth);
                    wasBitrangeWithinRange = false;
                }

                if (const utils::VariableAccessIndicesValidity::IndexValidationResult accessedBitRangEndIndexValidity = indexValidityOfUserDefinedAccessOnBitrange->bitRangeAccessValidity->bitRangeEndValiditiy; 
                    accessedBitRangEndIndexValidity.indexValidity == utils::VariableAccessIndicesValidity::IndexValidationResult::OutOfRange && accessedBitRangEndIndexValidity.indexValue.has_value()) {
                    recordSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(mapTokenPositionToMessagePosition(*context->bitRangeEnd->getStart()), accessedBitRangEndIndexValidity.indexValue.value(), generatedVariableAccess->var->bitwidth);
                    wasBitrangeWithinRange = false;
                }
            } else {
                if (const std::optional accessedBitIndexValidity = bitRangeStart.has_value() ? indexValidityOfUserDefinedAccessOnBitrange->bitRangeAccessValidity->bitRangeStartValidity : indexValidityOfUserDefinedAccessOnBitrange->bitRangeAccessValidity->bitRangeEndValiditiy; 
                    accessedBitIndexValidity.has_value() && accessedBitIndexValidity->indexValidity == utils::VariableAccessIndicesValidity::IndexValidationResult::OutOfRange && accessedBitIndexValidity->indexValue.has_value()) {
                    recordSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(
                            mapTokenPositionToMessagePosition(bitRangeStart.has_value() ? *context->bitStart->getStart() : *context->bitRangeEnd->getStart()),
                            accessedBitIndexValidity->indexValue.value(),
                            generatedVariableAccess->var->bitwidth);
                    wasBitrangeWithinRange = false;
                }
            }
        }
    }

    if (wasBitrangeWithinRange) {
        const std::optional<unsigned int> accessedBitRangeStart = bitRangeStart.has_value() ? bitRangeStart->get()->tryEvaluate({}) : std::make_optional(0u);
        const std::optional<unsigned int> accessedBitRangeEnd   = bitRangeEnd.has_value() ? bitRangeEnd->get()->tryEvaluate({}) : std::make_optional(generatedVariableAccess->getVar()->bitwidth - 1);
        std::optional<unsigned int>       userAccessedBitrangeLength;
        if (accessedBitRangeStart.has_value() && accessedBitRangeEnd.has_value())
            userAccessedBitrangeLength = (*accessedBitRangeStart > *accessedBitRangeEnd 
                    ? *accessedBitRangeStart - *accessedBitRangeEnd
                    : *accessedBitRangeEnd - *accessedBitRangeStart) + 1;

        if (optionalExpectedBitwidthForAnyProcessedEntity.has_value()) {
            if (userAccessedBitrangeLength != *optionalExpectedBitwidthForAnyProcessedEntity)
                recordSemanticError<SemanticError::ExpressionBitwidthMissmatches>(mapTokenPositionToMessagePosition(*context->bitStart->getStart()), *optionalExpectedBitwidthForAnyProcessedEntity, *userAccessedBitrangeLength);
        } else {
            setExpectedBitwidthForAnyProcessedEntity(*userAccessedBitrangeLength);
        }
    }
    return generatedVariableAccess;
}

void CustomExpressionVisitor::setExpectedBitwidthForAnyProcessedEntity(unsigned int bitwidth) {
    optionalExpectedBitwidthForAnyProcessedEntity = bitwidth;
}

void CustomExpressionVisitor::clearExpectedBitwidthForAnyProcessedEntity() {
    optionalExpectedBitwidthForAnyProcessedEntity.reset();
}

bool CustomExpressionVisitor::setRestrictionOnVariableAccesses(const syrec::VariableAccess::ptr& notAccessiblePartsForFutureVariableAccesses) {
    if (!notAccessiblePartsForFutureVariableAccesses->var 
        || notAccessiblePartsForFutureVariableAccesses->var->name.empty()
        || std::any_of(
        notAccessiblePartsForFutureVariableAccesses->indexes.cbegin(), 
        notAccessiblePartsForFutureVariableAccesses->indexes.cbegin(), 
        [](const syrec::Expression::ptr& exprDefiningAccessedValueOfDimension) {
            return !tryGetConstantValueOfExpression(*exprDefiningAccessedValueOfDimension).has_value();
        }) 
        || !tryDetermineAccessedBitrangeOfVariableAccess(*notAccessiblePartsForFutureVariableAccesses).has_value())
        return false;

    optionalRestrictionOnVariableAccesses = notAccessiblePartsForFutureVariableAccesses;
    return true;
}

void CustomExpressionVisitor::clearRestrictionOnVariableAccesses() {
    optionalRestrictionOnVariableAccesses.reset();
}

void CustomExpressionVisitor::setRestrictionOnLoopVariablesUsableInFutureLoopVariableValueInitializations(const std::string_view& loopVariableIdentifier) {
    optionalRestrictionOnLoopVariableUsageInLoopVariableValueInitialization = loopVariableIdentifier;
}

void CustomExpressionVisitor::clearRestrictionOnLoopVariablesUsableInFutureLoopVariableValueInitializations() {
    optionalRestrictionOnLoopVariableUsageInLoopVariableValueInitialization.reset();
}

// START OF NON-PUBLIC FUNCTIONALITY
std::optional<syrec::BinaryExpression::BinaryOperation> CustomExpressionVisitor::deserializeBinaryOperationFromString(const std::string_view& stringifiedOperation) {
    if (stringifiedOperation == "+")
        return syrec::BinaryExpression::BinaryOperation::Add;
    if (stringifiedOperation == "-")
        return syrec::BinaryExpression::BinaryOperation::Subtract;
    if (stringifiedOperation == "^")
        return syrec::BinaryExpression::BinaryOperation::Exor;
    if (stringifiedOperation == "*")
        return syrec::BinaryExpression::BinaryOperation::Multiply;
    if (stringifiedOperation == "/")
        return syrec::BinaryExpression::BinaryOperation::Divide;
    if (stringifiedOperation == "%")
        return syrec::BinaryExpression::BinaryOperation::Modulo;
    if (stringifiedOperation == "*>")
        return syrec::BinaryExpression::BinaryOperation::FracDivide;
    if (stringifiedOperation == "&&")
        return syrec::BinaryExpression::BinaryOperation::LogicalAnd;
    if (stringifiedOperation == "||")
        return syrec::BinaryExpression::BinaryOperation::LogicalOr;
    if (stringifiedOperation == "&")
        return syrec::BinaryExpression::BinaryOperation::BitwiseAnd;
    if (stringifiedOperation == "|")
        return syrec::BinaryExpression::BinaryOperation::BitwiseOr;
    if (stringifiedOperation == "<")
        return syrec::BinaryExpression::BinaryOperation::LessThan;
    if (stringifiedOperation == ">")
        return syrec::BinaryExpression::BinaryOperation::GreaterThan;
    if (stringifiedOperation == "==")
        return syrec::BinaryExpression::BinaryOperation::Equals;
    if (stringifiedOperation == "!=")
        return syrec::BinaryExpression::BinaryOperation::NotEquals;
    if (stringifiedOperation == "<=")
        return syrec::BinaryExpression::BinaryOperation::LessEquals;
    if (stringifiedOperation == ">=")
        return syrec::BinaryExpression::BinaryOperation::GreaterEquals;
    return std::nullopt;
}

std::optional<syrec::ShiftExpression::ShiftOperation> CustomExpressionVisitor::deserializeShiftOperationFromString(const std::string_view& stringifiedOperation) {
    if (stringifiedOperation == "<<")
        return syrec::ShiftExpression::ShiftOperation::Left;
    if (stringifiedOperation == ">>")
        return syrec::ShiftExpression::ShiftOperation::Right;
    return std::nullopt;
}