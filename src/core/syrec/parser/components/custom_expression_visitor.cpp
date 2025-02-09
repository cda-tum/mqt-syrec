#include "core/syrec/parser/components/custom_expression_visitor.hpp"

#include "core/syrec/expression.hpp"
#include "core/syrec/parser/utils/variable_access_index_check.hpp"
#include "core/syrec/parser/utils/symbolTable/temporary_variable_scope.hpp"
#include "core/syrec/parser/utils/syrec_operation_utils.hpp"
#include <core/syrec/parser/utils/variable_overlap_check.hpp>
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

    // We should not have to report an error at this position since the tokenizer should already report an error if the currently processed token is
    // not in the union of the FIRST sets of the potential alternatives.
    //recordCustomError(Message::Position(0, 0), "Unhandled expression context variant. This should not happen");
    return std::nullopt;
}

std::optional<syrec::Expression::ptr> CustomExpressionVisitor::visitBinaryExpressionTyped(const TSyrecParser::BinaryExpressionContext* context) {
    if (!context)
        return std::nullopt;

    recordExpressionComponent(utils::IfStatementExpressionComponentsRecorder::ExpressionBracketKind::Opening);
    std::optional<syrec::Expression::ptr> lhsOperand = visitExpressionTyped(context->lhsOperand);
    const std::optional<syrec::BinaryExpression::BinaryOperation> mappedToBinaryOperation = context->binaryOperation ? deserializeBinaryOperationFromString(context->binaryOperation->getText()) : std::nullopt;
    if (context->binaryOperation && !mappedToBinaryOperation.has_value())
        recordSemanticError<SemanticError::UnhandledOperationFromGrammarInParser>(mapTokenPositionToMessagePosition(*context->binaryOperation), context->binaryOperation->getText());

    if (mappedToBinaryOperation.has_value())
        recordExpressionComponent(*mappedToBinaryOperation);

    std::optional<syrec::Expression::ptr> rhsOperand = visitExpressionTyped(context->rhsOperand);
    recordExpressionComponent(utils::IfStatementExpressionComponentsRecorder::ExpressionBracketKind::Closing);
    if (!mappedToBinaryOperation.has_value())
        return std::nullopt;

    const std::optional<unsigned int> constantValueOfLhsOperand = lhsOperand.has_value() && *lhsOperand ? tryGetConstantValueOfExpression(**lhsOperand) : std::nullopt;
    const std::optional<unsigned int> constantValueOfRhsOperand = rhsOperand.has_value() && *rhsOperand ? tryGetConstantValueOfExpression(**rhsOperand) : std::nullopt;

    if ((*mappedToBinaryOperation == syrec::BinaryExpression::BinaryOperation::Divide || *mappedToBinaryOperation == syrec::BinaryExpression::BinaryOperation::Modulo || *mappedToBinaryOperation == syrec::BinaryExpression::BinaryOperation::FracDivide) 
        && constantValueOfRhsOperand.has_value() && !*constantValueOfRhsOperand) {
        recordSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(mapTokenPositionToMessagePosition(*(context->lhsOperand ? context->lhsOperand->getStart() : context->rhsOperand->getStart())));
        return std::nullopt;
    }

    // We delegate the truncation of constant values to the caller of this function since the expected bitwidth of the operands could have been set in the currently processed
    // expression and needs to be propagate to any parent expression
    if (constantValueOfLhsOperand.has_value() && rhsOperand.has_value() && !constantValueOfRhsOperand.has_value()) {
        if (const std::optional<syrec::Expression::ptr> simplifiedBinaryExpression = trySimplifyBinaryExpressionWithConstantValueOfOneOperandKnown(*constantValueOfLhsOperand, *mappedToBinaryOperation, *rhsOperand, true); simplifiedBinaryExpression.has_value())
            return *simplifiedBinaryExpression;   
        if (utils::isOperandIdentityElementOfOperation(*constantValueOfLhsOperand, *mappedToBinaryOperation))
            return rhsOperand;
    }

    if (constantValueOfRhsOperand.has_value() && lhsOperand.has_value() && !constantValueOfLhsOperand.has_value()) {
        if (const std::optional<syrec::Expression::ptr> simplifiedBinaryExpression = trySimplifyBinaryExpressionWithConstantValueOfOneOperandKnown(*constantValueOfRhsOperand, *mappedToBinaryOperation, *lhsOperand, false); simplifiedBinaryExpression.has_value())
            return *simplifiedBinaryExpression;
        if (utils::isOperandIdentityElementOfOperation(*constantValueOfRhsOperand, *mappedToBinaryOperation))
            return lhsOperand;
    }

    if (constantValueOfLhsOperand.has_value() && constantValueOfRhsOperand.has_value()) {
        if (const std::optional<unsigned int> evaluationResultOfExpr = utils::tryEvaluate(constantValueOfLhsOperand, *mappedToBinaryOperation, constantValueOfRhsOperand); evaluationResultOfExpr.has_value())
            return std::make_shared<syrec::NumericExpression>(std::make_shared<syrec::Number>(*evaluationResultOfExpr), optionalExpectedBitwidthForAnyProcessedEntity.value_or(DEFAULT_EXPRESSION_BITWIDTH));
    }
    return lhsOperand.has_value() && rhsOperand.has_value() ? std::make_optional(std::make_shared<syrec::BinaryExpression>(*lhsOperand, *mappedToBinaryOperation, *rhsOperand)) : std::nullopt;
}

std::optional<syrec::Expression::ptr> CustomExpressionVisitor::visitShiftExpressionTyped(TSyrecParser::ShiftExpressionContext* context) {
    if (!context)
        return std::nullopt;

    recordExpressionComponent(utils::IfStatementExpressionComponentsRecorder::ExpressionBracketKind::Opening);
    std::optional<syrec::Expression::ptr>                       toBeShiftedOperand     = visitExpressionTyped(context->expression());
    const std::optional<syrec::ShiftExpression::ShiftOperation> mappedToShiftOperation = context->shiftOperation ? deserializeShiftOperationFromString(context->shiftOperation->getText()) : std::nullopt;
    if (context->shiftOperation && !mappedToShiftOperation.has_value())
        recordSemanticError<SemanticError::UnhandledOperationFromGrammarInParser>(mapTokenPositionToMessagePosition(*context->shiftOperation), context->shiftOperation->getText());

    if (mappedToShiftOperation.has_value()) {
        recordExpressionComponent(*mappedToShiftOperation);
    }

    const std::optional<syrec::Number::ptr> shiftAmount = visitNumberTyped(context->number());
    recordExpressionComponent(utils::IfStatementExpressionComponentsRecorder::ExpressionBracketKind::Closing);
    if (!mappedToShiftOperation.has_value())
        return std::nullopt;

    const std::optional<unsigned int> constantValueOfToBeShiftedOperand = toBeShiftedOperand.has_value() && *toBeShiftedOperand ? tryGetConstantValueOfExpression(**toBeShiftedOperand) : std::nullopt;
    const std::optional<unsigned int> constantValueOfShiftAmount        = shiftAmount.has_value() && *shiftAmount ? shiftAmount.value()->tryEvaluate({}) : std::nullopt;

    // A note regaring truncation of the operands of the generated shift expression.
    // In case that both operands evaluate to a constant value, evaluation of the expression is performed at compile time and the constant value propagated without truncation (since we do not know anything about any potential outer expression)
    // Otherwise, when the lhs operand evaluate to a variable access or expression, the performed shift will not change the bitwidth of the result and thus neither of the two operands need to be truncated.
    if (constantValueOfShiftAmount.has_value()) {
        if (!*constantValueOfShiftAmount)
            return toBeShiftedOperand.has_value() ? std::make_optional(*toBeShiftedOperand) : std::nullopt;
        if (*constantValueOfShiftAmount >= MAX_SUPPORTED_SIGNAL_BITWIDTH)
            return std::make_shared<syrec::NumericExpression>(std::make_shared<syrec::Number>(0), optionalExpectedBitwidthForAnyProcessedEntity.value_or(1));
        if (constantValueOfToBeShiftedOperand.has_value()) {
            if (const std::optional<unsigned int> evaluationResultOfShiftOperation = utils::tryEvaluate(constantValueOfToBeShiftedOperand, *mappedToShiftOperation, constantValueOfShiftAmount); evaluationResultOfShiftOperation.has_value())
                return std::make_shared<syrec::NumericExpression>(std::make_shared<syrec::Number>(*evaluationResultOfShiftOperation), optionalExpectedBitwidthForAnyProcessedEntity.value_or(DEFAULT_EXPRESSION_BITWIDTH));
        }
    }
    return toBeShiftedOperand.has_value() && shiftAmount.has_value() ? std::make_optional(std::make_shared<syrec::ShiftExpression>(*toBeShiftedOperand, *mappedToShiftOperation, *shiftAmount)) : std::nullopt;
}

std::optional<syrec::Expression::ptr> CustomExpressionVisitor::visitUnaryExpressionTyped(const TSyrecParser::UnaryExpressionContext* context) const {
    if (context && context->start)
        recordCustomError(mapTokenPositionToMessagePosition(*context->start), "Unary expressions are currently not supported");

    // As a future note, identical to the behaviour in the shift expression, the unary operation applied by this type of expression does not the change the bitwidth of the latter which
    // in turn means that no truncation is required.
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

    // We should not have to report an error at this position since the tokenizer should already report an error if the currently processed token is
    // not in the union of the FIRST sets of the potential alternatives.
    //recordCustomError(Message::Position(0, 0), "Unhandled number context variant. This should not happen");
    return std::nullopt;
}

std::optional<syrec::Number::ptr> CustomExpressionVisitor::visitNumberFromConstantTyped(TSyrecParser::NumberFromConstantContext* context) const {
    // TODO: Check these assumptions
    // Production should only be called if the token contains only numeric characters and thus deserialization should only fail if an overflow occurs.
    // Leading and trailing whitespace should also be trimmed from the token text by the parser.
    if (!context || !context->INT())
        return std::nullopt;

    if (const std::optional<unsigned int> constantValue = deserializeConstantFromString(context->INT()->getText(), nullptr); constantValue.has_value()) {
        recordExpressionComponent(*constantValue);
        return std::make_shared<syrec::Number>(*constantValue);
    }
    return std::nullopt;
}

std::optional<syrec::Number::ptr> CustomExpressionVisitor::visitNumberFromExpressionTyped(const TSyrecParser::NumberFromExpressionContext* context) const {
    if (!context)
        return std::nullopt;

    recordExpressionComponent(utils::IfStatementExpressionComponentsRecorder::ExpressionBracketKind::Opening);
    std::optional<syrec::Number::ptr> lhsOperand = visitNumberTyped(context->lhsOperand);
    
    const std::optional<syrec::Number::ConstantExpression::Operation> operation  = context->op ? deserializeConstantExpressionOperationFromString(context->op->getText()) : std::nullopt;
    if (context->op && !operation.has_value())
        recordSemanticError<SemanticError::UnhandledOperationFromGrammarInParser>(mapTokenPositionToMessagePosition(*context->op), context->op->getText());

    if (operation.has_value()) {
        switch (*operation) {
            case syrec::Number::ConstantExpression::Operation::Addition:
                recordExpressionComponent(syrec::BinaryExpression::BinaryOperation::Add);
                break;
            case syrec::Number::ConstantExpression::Operation::Subtraction:
                recordExpressionComponent(syrec::BinaryExpression::BinaryOperation::Subtract);
                break;
            case syrec::Number::ConstantExpression::Operation::Multiplication:
                recordExpressionComponent(syrec::BinaryExpression::BinaryOperation::Multiply);
                break;
            case syrec::Number::ConstantExpression::Operation::Division:
                recordExpressionComponent(syrec::BinaryExpression::BinaryOperation::Divide);
                break;
        }
    }

    std::optional<syrec::Number::ptr> rhsOperand = visitNumberTyped(context->rhsOperand);
    recordExpressionComponent(utils::IfStatementExpressionComponentsRecorder::ExpressionBracketKind::Closing);

    const std::optional<unsigned int> evaluationResultOfLhsOperand = lhsOperand.has_value() && *lhsOperand ? lhsOperand->get()->tryEvaluate({}) : std::nullopt;
    const std::optional<unsigned int> evaluationResultOfRhsOperand = rhsOperand.has_value() && *rhsOperand ? rhsOperand->get()->tryEvaluate({}) : std::nullopt;

    if (operation.has_value()) {
        switch (*operation) {
            case syrec::Number::ConstantExpression::Operation::Addition: {
                if (evaluationResultOfLhsOperand.has_value() && !*evaluationResultOfLhsOperand)
                    return rhsOperand;
                if (evaluationResultOfRhsOperand.has_value() && !*evaluationResultOfRhsOperand)
                    return lhsOperand;
                break;
            }
            case syrec::Number::ConstantExpression::Operation::Subtraction: {
                if (evaluationResultOfRhsOperand.has_value() && !*evaluationResultOfRhsOperand)
                    return lhsOperand;
                break;
            }
            case syrec::Number::ConstantExpression::Operation::Multiplication: {
                if (evaluationResultOfLhsOperand.has_value() && *evaluationResultOfLhsOperand == 1)
                    return rhsOperand;

                if (evaluationResultOfRhsOperand.has_value() && *evaluationResultOfRhsOperand == 1)
                    return lhsOperand;

                break;
            }
            case syrec::Number::ConstantExpression::Operation::Division: {
                if (evaluationResultOfLhsOperand.has_value()) {
                    if (!*evaluationResultOfLhsOperand) {
                        recordSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(mapTokenPositionToMessagePosition(*context->lhsOperand->getStart()));
                        return std::nullopt;        
                    }
                }
                if (evaluationResultOfRhsOperand.has_value()) {
                    if (!*evaluationResultOfRhsOperand) {
                        recordSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(mapTokenPositionToMessagePosition(*context->rhsOperand->getStart()));
                        return std::nullopt;
                    }
                    if (*evaluationResultOfRhsOperand == 1)
                        return lhsOperand;
                }
            }
        }

        if (lhsOperand.has_value() && rhsOperand.has_value()) {
            const auto constantExpression = syrec::Number::ConstantExpression(*lhsOperand, *operation, *rhsOperand);
            if (const std::optional<unsigned int> evaluatedConstantExpressionValue = constantExpression.tryEvaluate({}); evaluatedConstantExpressionValue.has_value())
                return std::make_shared<syrec::Number>(*evaluatedConstantExpressionValue);
            return std::make_shared<syrec::Number>(constantExpression);
        }
    }
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
    recordExpressionComponent(loopVariableIdentifier);
    if (const std::optional<utils::TemporaryVariableScope::ScopeEntry::readOnylPtr> matchingLoopVariableForIdentifier = activeVariableScopeInSymbolTable->get()->getVariableByName(loopVariableIdentifier); matchingLoopVariableForIdentifier.has_value() && matchingLoopVariableForIdentifier->get()->isReferenceToLoopVariable()) {
        if (optionalRestrictionOnLoopVariableUsageInLoopVariableValueInitialization.has_value() && optionalRestrictionOnLoopVariableUsageInLoopVariableValueInitialization.value() == loopVariableIdentifier)
            recordSemanticError<SemanticError::ValueOfLoopVariableNotUsableInItsInitialValueDeclaration>(mapTokenPositionToMessagePosition(*context->LOOP_VARIABLE_PREFIX()->getSymbol()), loopVariableIdentifier);
        return std::make_shared<syrec::Number>(loopVariableIdentifier);
    }
    recordSemanticError<SemanticError::NoVariableMatchingIdentifier>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()), loopVariableIdentifier);
    return std::nullopt;
}

std::optional<syrec::Number::ptr> CustomExpressionVisitor::visitNumberFromSignalwidthTyped(TSyrecParser::NumberFromSignalwidthContext* context) const {
    if (!context || !context->IDENT())
        return std::nullopt;

    const std::optional<utils::TemporaryVariableScope::ptr> activeVariableScopeInSymbolTable = symbolTable->getActiveTemporaryScope();
    if (!activeVariableScopeInSymbolTable)
        return std::nullopt;

    const std::string& variableIdentifier = context->IDENT()->getSymbol()->getText();
    recordExpressionComponent(context->SIGNAL_WIDTH_PREFIX()->getSymbol()->getText() + variableIdentifier);
    if (const std::optional<utils::TemporaryVariableScope::ScopeEntry::readOnylPtr> matchingVariableForIdentifier = activeVariableScopeInSymbolTable->get()->getVariableByName(variableIdentifier); matchingVariableForIdentifier.has_value()) {
        return optionalExpectedBitwidthForAnyProcessedEntity.has_value() && matchingVariableForIdentifier->get()->getDeclaredVariableBitwidth().has_value()
            ? std::make_shared<syrec::Number>(*matchingVariableForIdentifier->get()->getDeclaredVariableBitwidth())
            : std::make_shared<syrec::Number>(*matchingVariableForIdentifier->get()->getDeclaredVariableBitwidth());
    }

    recordSemanticError<SemanticError::NoVariableMatchingIdentifier>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()), variableIdentifier);
    return std::nullopt;
}

// TODO: Substitution of loop variable values for compile time index checks
// TODO: Signal overlap checks - add semantic errors for overlaps
std::optional<syrec::VariableAccess::ptr> CustomExpressionVisitor::visitSignalTyped(TSyrecParser::SignalContext* context) {
    if (!context || !context->IDENT())
        return std::nullopt;

    const std::optional<utils::TemporaryVariableScope::ptr> activeVariableScopeInSymbolTable = symbolTable->getActiveTemporaryScope();
    if (!activeVariableScopeInSymbolTable)
        return std::nullopt;

    // If we are omitting the variable identifier in the production, a default token for the variable identifier is generated (its text states that the IDENT token is missing) and its
    // 'error' message text used as the variables identifier and thus potentially reported in semantic erros. Since the lexer will already generate an identical syntax error, we assume that
    // the actual variable identifier is empty in this error case.
    const std::string& variableIdentifier = context->IDENT()->getTreeType() != antlr4::tree::ParseTreeType::ERROR
        ? context->IDENT()->getSymbol()->getText()
        : "";

    const std::optional<utils::TemporaryVariableScope::ScopeEntry::readOnylPtr> matchingVariableForIdentifier = !variableIdentifier.empty()
        ? activeVariableScopeInSymbolTable->get()->getVariableByName(variableIdentifier)
        : std::nullopt;

    if (!variableIdentifier.empty() && (!matchingVariableForIdentifier.has_value() || !matchingVariableForIdentifier.value()->getReadonlyVariableData().has_value())) {
        recordSemanticError<SemanticError::NoVariableMatchingIdentifier>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()), variableIdentifier);
        return std::nullopt;
    }

    // We do not want to explicitly record the components of the dimension and bit range access again in the expression components recorder
    // since they are already stored when the generated variable access is recorded
    std::optional<utils::IfStatementExpressionComponentsRecorder::OperationMode> backupOfCurrentExpressionComponentsRecorderMode;
    if (optionalIfStatementExpressionComponentsRecorder.has_value()) {
        backupOfCurrentExpressionComponentsRecorderMode = optionalIfStatementExpressionComponentsRecorder->get()->getCurrentOperationMode();
        optionalIfStatementExpressionComponentsRecorder->get()->switchMode(utils::IfStatementExpressionComponentsRecorder::OperationMode::Ignoring);   
    }

    const std::size_t          numUserAccessedDimensions = context->accessedDimensions.size();
    syrec::VariableAccess::ptr generatedVariableAccess   = std::make_shared<syrec::VariableAccess>();
    generatedVariableAccess->indexes                     = syrec::Expression::vec(numUserAccessedDimensions, nullptr);

    for (std::size_t i = 0; i < context->accessedDimensions.size(); ++i) {
        const bool existsOperandBitwidthSizeRestriction = optionalExpectedBitwidthForAnyProcessedEntity.has_value();
        if (const std::optional<syrec::Expression::ptr> exprDefiningAccessedValueOfDimension = visitExpressionTyped(context->accessedDimensions.at(i)); exprDefiningAccessedValueOfDimension.has_value())
            generatedVariableAccess->indexes[i] = *exprDefiningAccessedValueOfDimension;

        // Any generated bitwidth restriction generated during the processing of the expression defining the accessed value of the dimension needs to be cleared if the latter was processed to prevent
        // the propagation of the restriction to the parsing process for the remaining components of the variable access
        if (!existsOperandBitwidthSizeRestriction && optionalExpectedBitwidthForAnyProcessedEntity.has_value()) {
            // Regardless of whether variable accesses with an unknown length of the accessed bitrange were defined in the expression (referred to as E) specifying the index value for the currently processed dimension of the dimension access,
            // truncation of constant values in any binary expression that is part of E can use the set bitlength of any variable access (since we are assuming that the user defined a well formed expression [i.e. the accessed bitrange of any variable access operands
            // has the same length]).
            if (generatedVariableAccess->indexes[i] && !existsOperandBitwidthSizeRestriction && optionalExpectedBitwidthForAnyProcessedEntity.has_value()) {
                truncateConstantValuesInAnyBinaryExpression(*generatedVariableAccess->indexes[i], *optionalExpectedBitwidthForAnyProcessedEntity, integerConstantTruncationOperation);
            }
            clearExpectedBitwidthForAnyProcessedEntity();   
        }
    }

    if (matchingVariableForIdentifier.has_value()) {
        const std::vector<unsigned int>& declaredValuesPerDimensionOfReferenceVariable = matchingVariableForIdentifier.value()->getDeclaredVariableDimensions();

        if (matchingVariableForIdentifier->get()->getReadonlyVariableData().has_value())
            // Currently the shared_pointer instance returned by the symbol table for entry of the variable matching the current identifier cannot be assigned to the variable access
            // data field since it expects a shared_pointer instance to a modifiable variable, thus we are forced to create a copy of the variable data from the symbol table. Future
            // versions might change the variable field in the variable access to match the value returned by the symbol table (Reasoning: Why would the user modify the referenced variable data in the variable access
            // when he can simply reassign the smart_pointer).
            generatedVariableAccess->setVar(std::make_shared<syrec::Variable>(**matchingVariableForIdentifier->get()->getReadonlyVariableData()));
        else
            recordCustomError(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()), "Symbol table entry for variable with identifier " + variableIdentifier + " did not return variable data. This should not happen");

        if (!numUserAccessedDimensions) {
            if (declaredValuesPerDimensionOfReferenceVariable.size() == 1 && declaredValuesPerDimensionOfReferenceVariable.front() == 1)
                generatedVariableAccess->indexes.emplace_back(std::make_shared<syrec::NumericExpression>(std::make_shared<syrec::Number>(0), 1));
            else
                recordSemanticError<SemanticError::OmittingDimensionAccessOnlyPossibleFor1DSignalWithSingleValue>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()));
        } else if (numUserAccessedDimensions != declaredValuesPerDimensionOfReferenceVariable.size()) {
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
    }

    const std::optional<syrec::Number::ptr> bitRangeStart = visitNumberTyped(context->bitStart);
    const std::optional<syrec::Number::ptr> bitRangeEnd   = visitNumberTyped(context->bitRangeEnd);

    // Premature exit since index checks for both the defined dimension as well as bit range access depend on the information from the referenced variable.
    if (!generatedVariableAccess->var)
        return std::nullopt;

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
                }

                if (const utils::VariableAccessIndicesValidity::IndexValidationResult accessedBitRangeEndIndexValidity = indexValidityOfUserDefinedAccessOnBitrange->bitRangeAccessValidity->bitRangeEndValiditiy;
                    accessedBitRangeEndIndexValidity.indexValidity == utils::VariableAccessIndicesValidity::IndexValidationResult::OutOfRange && accessedBitRangeEndIndexValidity.indexValue.has_value()) {
                    recordSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(mapTokenPositionToMessagePosition(*context->bitRangeEnd->getStart()), accessedBitRangeEndIndexValidity.indexValue.value(), generatedVariableAccess->var->bitwidth);
                }
            } else if (const std::optional<utils::VariableAccessIndicesValidity::IndexValidationResult> accessedBitIndexValidity = bitRangeStart.has_value() ? indexValidityOfUserDefinedAccessOnBitrange->bitRangeAccessValidity->bitRangeStartValidity : indexValidityOfUserDefinedAccessOnBitrange->bitRangeAccessValidity->bitRangeEndValiditiy;
                       accessedBitIndexValidity.has_value() && accessedBitIndexValidity->indexValidity == utils::VariableAccessIndicesValidity::IndexValidationResult::OutOfRange && accessedBitIndexValidity->indexValue.has_value()) {
                recordSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(
                    mapTokenPositionToMessagePosition(bitRangeStart.has_value() ? *context->bitStart->getStart() : *context->bitRangeEnd->getStart()),
                    accessedBitIndexValidity->indexValue.value(),
                    generatedVariableAccess->var->bitwidth);
            }
        }
    }

    if (optionalIfStatementExpressionComponentsRecorder.has_value() && backupOfCurrentExpressionComponentsRecorderMode.has_value()) {
        optionalIfStatementExpressionComponentsRecorder->get()->switchMode(*backupOfCurrentExpressionComponentsRecorderMode);
        // To be able to compare variable accesses on 1D signals in the expressions of the if statement guard conditions, we need a way to distinguish
        // between the otherwise semantically equivalent variable accesses in which the dimension access is defined implicitly/explicitly (which only works for 1D signals with a single declared value).
        // Thus, variable accesses on 1D signals declared with an implicit dimension access will be recorded with as having an empty dimension access (the implicitly added access on the value of the dimension is removed).
        if (generatedVariableAccess->var->dimensions.size() == 1 && generatedVariableAccess->var->dimensions.front() == 1 && context->accessedDimensions.empty()) {
            const auto variableAccessWithImplicitDimensionAccessRemoved = std::make_shared<syrec::VariableAccess>(*generatedVariableAccess);
            variableAccessWithImplicitDimensionAccessRemoved->indexes.clear();
            recordExpressionComponent(variableAccessWithImplicitDimensionAccessRemoved);
        }
        else {
            recordExpressionComponent(generatedVariableAccess);    
        }
    }

    // Since the error reported when defining an overlapping variable access is reported at the position of the variable identifier, this check needs to be performed prior
    // to the check for matching operand bitwidths (if no internal ordering of the reported errors is performed [which is currently the case])
    if (const std::optional<utils::VariableAccessOverlapCheckResult>& overlapCheckResultWithRestrictedVariableParts = optionalRestrictionOnVariableAccesses.has_value() && *optionalRestrictionOnVariableAccesses ? utils::checkOverlapBetweenVariableAccesses(**optionalRestrictionOnVariableAccesses, *generatedVariableAccess) : std::nullopt;
        overlapCheckResultWithRestrictedVariableParts.has_value() && overlapCheckResultWithRestrictedVariableParts->overlapState == utils::VariableAccessOverlapCheckResult::OverlapState::Overlapping) {
        if (!overlapCheckResultWithRestrictedVariableParts->overlappingIndicesInformations.has_value())
            recordCustomError(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()), "Overlap with restricted variable parts detected but no further information about overlap available. This should not happen");
        else
            recordSemanticError<SemanticError::ReversibilityOfStatementNotPossibleDueToAccessOnRestrictedVariableParts>(
                    mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()),
                    overlapCheckResultWithRestrictedVariableParts->stringifyOverlappingIndicesInformation());
    }

    std::optional<unsigned int> accessedBitRangeStart;
    if (context->bitStart)
        accessedBitRangeStart = bitRangeStart.has_value() ? bitRangeStart->get()->tryEvaluate({}) : std::nullopt;
    else
        accessedBitRangeStart = 0;

    std::optional<unsigned int> accessedBitRangeEnd;
    if (context->bitRangeEnd)
        accessedBitRangeEnd = bitRangeEnd.has_value() ? bitRangeEnd->get()->tryEvaluate({}) : std::nullopt;
    else
        accessedBitRangeEnd = context->bitStart ? accessedBitRangeStart : generatedVariableAccess->getVar()->bitwidth - 1;

    if (accessedBitRangeStart.has_value() && accessedBitRangeEnd.has_value()) {
        const unsigned int userAccessedBitrangeLength = (*accessedBitRangeStart > *accessedBitRangeEnd 
            ? *accessedBitRangeStart - *accessedBitRangeEnd
            : *accessedBitRangeEnd - *accessedBitRangeStart) + 1;

        if (!optionalExpectedBitwidthForAnyProcessedEntity.has_value())
            setExpectedBitwidthForAnyProcessedEntity(userAccessedBitrangeLength);
        else if (userAccessedBitrangeLength != *optionalExpectedBitwidthForAnyProcessedEntity)
            recordSemanticError<SemanticError::ExpressionBitwidthMissmatches>(
                mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()), 
                *optionalExpectedBitwidthForAnyProcessedEntity, 
                userAccessedBitrangeLength);
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
    if (!notAccessiblePartsForFutureVariableAccesses
        || !notAccessiblePartsForFutureVariableAccesses->var 
        || notAccessiblePartsForFutureVariableAccesses->var->name.empty()
        || std::any_of(
        notAccessiblePartsForFutureVariableAccesses->indexes.cbegin(), 
        notAccessiblePartsForFutureVariableAccesses->indexes.cbegin(), 
        [](const syrec::Expression::ptr& exprDefiningAccessedValueOfDimension) {
            return !tryGetConstantValueOfExpression(*exprDefiningAccessedValueOfDimension).has_value();
        }))
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

void CustomExpressionVisitor::setIfStatementExpressionComponentsRecorder(const utils::IfStatementExpressionComponentsRecorder::ptr& ifStatementExpressionComponentsRecorder) {
    if (!ifStatementExpressionComponentsRecorder)
        return;
    optionalIfStatementExpressionComponentsRecorder = ifStatementExpressionComponentsRecorder;
}

void CustomExpressionVisitor::clearIfStatementExpressionComponentsRecorder() {
    optionalIfStatementExpressionComponentsRecorder.reset();
}

std::optional<unsigned int> CustomExpressionVisitor::getCurrentExpectedBitwidthForAnyProcessedEntity() const {
    return optionalExpectedBitwidthForAnyProcessedEntity;
}

void CustomExpressionVisitor::truncateConstantValuesInAnyBinaryExpression(syrec::Expression& expression, unsigned int expectedBitwidthOfOperandsInExpression, utils::IntegerConstantTruncationOperation integerConstantTruncationOperation) {
    if (auto exprAsBinaryExpr = dynamic_cast<syrec::BinaryExpression*>(&expression); exprAsBinaryExpr) {
        if (exprAsBinaryExpr->lhs)
            truncateConstantValuesInAnyBinaryExpression(*exprAsBinaryExpr->lhs, expectedBitwidthOfOperandsInExpression, integerConstantTruncationOperation);
        if (exprAsBinaryExpr->rhs)
            truncateConstantValuesInAnyBinaryExpression(*exprAsBinaryExpr->rhs, expectedBitwidthOfOperandsInExpression, integerConstantTruncationOperation);
    } else if (auto exprAsShiftExpr = dynamic_cast<syrec::ShiftExpression*>(&expression); exprAsShiftExpr) {
        if (exprAsShiftExpr->lhs)
            truncateConstantValuesInAnyBinaryExpression(*exprAsShiftExpr->lhs, expectedBitwidthOfOperandsInExpression, integerConstantTruncationOperation);
    } else if (auto exprAsNumericExpr = dynamic_cast<syrec::NumericExpression*>(&expression); exprAsNumericExpr) {
        if (!exprAsNumericExpr->value || exprAsNumericExpr->value->isLoopVariable())
            return;
        if (const std::optional<unsigned int> constantValueOfNumericExpr = exprAsNumericExpr->value->tryEvaluate({}); constantValueOfNumericExpr.has_value())
            exprAsNumericExpr->value = std::make_shared<syrec::Number>(utils::truncateConstantValueToExpectedBitwidth(*constantValueOfNumericExpr, expectedBitwidthOfOperandsInExpression, integerConstantTruncationOperation));
    }
}


// START OF NON-PUBLIC FUNCTIONALITY
void CustomExpressionVisitor::recordExpressionComponent(const utils::IfStatementExpressionComponentsRecorder::ExpressionComponent& expressionComponent) const {
    if (!optionalIfStatementExpressionComponentsRecorder.has_value() || !*optionalIfStatementExpressionComponentsRecorder)
        return;
    optionalIfStatementExpressionComponentsRecorder->get()->recordExpressionComponent(expressionComponent);
}

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
    if (stringifiedOperation == "=")
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

std::optional<syrec::Number::ConstantExpression::Operation> CustomExpressionVisitor::deserializeConstantExpressionOperationFromString(const std::string_view& stringifiedOperation) {
    if (stringifiedOperation == "+")
        return syrec::Number::ConstantExpression::Operation::Addition;
    if (stringifiedOperation == "-")
        return syrec::Number::ConstantExpression::Operation::Subtraction;
    if (stringifiedOperation == "*")
        return syrec::Number::ConstantExpression::Operation::Multiplication;
    if (stringifiedOperation == "/")
        return syrec::Number::ConstantExpression::Operation::Division;
    return std::nullopt;
}

std::optional<syrec::Expression::ptr> CustomExpressionVisitor::trySimplifyBinaryExpressionWithConstantValueOfOneOperandKnown(unsigned int knownOperandValue, syrec::BinaryExpression::BinaryOperation binaryOperation, const syrec::Expression::ptr& unknownOperandValue, bool isValueOfLhsOperandKnown) {
    if (knownOperandValue > 1)
        return std::nullopt;
    if (knownOperandValue == 1) {
        switch (binaryOperation) {
            case syrec::BinaryExpression::BinaryOperation::Multiply:
            case syrec::BinaryExpression::BinaryOperation::LogicalAnd:
            case syrec::BinaryExpression::BinaryOperation::FracDivide:
                return unknownOperandValue;
            case syrec::BinaryExpression::BinaryOperation::Modulo:
                return isValueOfLhsOperandKnown ? std::make_shared<syrec::NumericExpression>(std::make_shared<syrec::Number>(1), 1) : unknownOperandValue;
            case syrec::BinaryExpression::BinaryOperation::LogicalOr:
            case syrec::BinaryExpression::BinaryOperation::BitwiseOr:
                return std::make_shared<syrec::NumericExpression>(std::make_shared<syrec::Number>(1), 1);
            case syrec::BinaryExpression::BinaryOperation::Divide:
                return !isValueOfLhsOperandKnown ? std::make_optional(unknownOperandValue) : std::nullopt;
            default:
                return std::nullopt;
        }
    }

    // Known operand value is zero at this point
    switch (binaryOperation)
    {
        case syrec::BinaryExpression::BinaryOperation::LogicalAnd:
        case syrec::BinaryExpression::BinaryOperation::BitwiseAnd:
        case syrec::BinaryExpression::BinaryOperation::Multiply:
            return std::make_shared<syrec::NumericExpression>(std::make_shared<syrec::Number>(0), 1);
        case syrec::BinaryExpression::BinaryOperation::LogicalOr:
        case syrec::BinaryExpression::BinaryOperation::BitwiseOr:
        case syrec::BinaryExpression::BinaryOperation::Add:
        case syrec::BinaryExpression::BinaryOperation::Subtract:
        case syrec::BinaryExpression::BinaryOperation::Exor:
            return unknownOperandValue;
        case syrec::BinaryExpression::BinaryOperation::Divide:
        case syrec::BinaryExpression::BinaryOperation::FracDivide:
            return isValueOfLhsOperandKnown ? std::make_optional(std::make_shared<syrec::NumericExpression>(std::make_shared<syrec::Number>(0), 1)) : std::nullopt;
        case syrec::BinaryExpression::BinaryOperation::Modulo:
            return isValueOfLhsOperandKnown ? std::make_optional(std::make_shared<syrec::NumericExpression>(std::make_shared<syrec::Number>(0), 1)) : std::nullopt;
        default:
            break;
    }
    return std::nullopt;
}
