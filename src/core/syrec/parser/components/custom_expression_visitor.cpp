#include "core/syrec/parser/components/custom_expression_visitor.hpp"

#include "core/syrec/parser/utils/variable_access_index_check.hpp"
#include "core/syrec/parser/utils/symbolTable/temporary_variable_scope.hpp"

using namespace syrecParser;

std::optional<syrec::Expression::ptr> CustomExpressionVisitor::visitBinaryExpressionTyped(TSyrecParser::BinaryExpressionContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Expression>(context);
}

std::optional<syrec::Expression::ptr> CustomExpressionVisitor::visitShiftExpressionTyped(TSyrecParser::ShiftExpressionContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Expression>(context);
}

std::optional<syrec::Expression::ptr> CustomExpressionVisitor::visitUnaryExpressionTyped(TSyrecParser::UnaryExpressionContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Expression>(context);
}

std::optional<syrec::Expression::ptr> CustomExpressionVisitor::visitExpressionFromNumberTyped(TSyrecParser::ExpressionFromNumberContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Expression>(context);
}

std::optional<syrec::Expression::ptr> CustomExpressionVisitor::visitExpressionFromSignalTyped(TSyrecParser::ExpressionFromSignalContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Expression>(context);
}

std::optional<syrec::Number::ptr> CustomExpressionVisitor::visitNumberFromConstantTyped(TSyrecParser::NumberFromConstantContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Number>(context);
}

std::optional<syrec::Number::ptr> CustomExpressionVisitor::visitNumberFromExpressionTyped(TSyrecParser::NumberFromExpressionContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Number>(context);
}

std::optional<syrec::Number::ptr> CustomExpressionVisitor::visitNumberFromLoopVariableTyped(TSyrecParser::NumberFromLoopVariableContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Number>(context);
}

std::optional<syrec::Number::ptr> CustomExpressionVisitor::visitNumberFromSignalwidthTyped(TSyrecParser::NumberFromSignalwidthContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Number>(context);
}

std::optional<syrec::Variable::ptr> CustomExpressionVisitor::visitSignalTyped(TSyrecParser::SignalContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Variable>(context);
}

// START OF NON-PUBLIC FUNCTIONALITY
std::any CustomExpressionVisitor::visitBinaryExpression(TSyrecParser::BinaryExpressionContext* context) {
    return 0;
}

std::any CustomExpressionVisitor::visitShiftExpression(TSyrecParser::ShiftExpressionContext* context) {
    return 0;
}

std::any CustomExpressionVisitor::visitUnaryExpression(TSyrecParser::UnaryExpressionContext* context) {
    return 0;
}

std::any CustomExpressionVisitor::visitExpressionFromNumber(TSyrecParser::ExpressionFromNumberContext* context) {
    return 0;
}

std::any CustomExpressionVisitor::visitExpressionFromSignal(TSyrecParser::ExpressionFromSignalContext* context) {
    return 0;
}

std::any CustomExpressionVisitor::visitNumberFromConstant(TSyrecParser::NumberFromConstantContext* context) {
    // TODO: Check these assumptions
    // Production should only be called if the token contains only numeric characters and thus deserialization should only fail if an overflow occurs.
    // Leading and trailing whitespace should also be trimmed from the token text by the parser.
    return context && context->INT() ? deserializeConstantFromString(context->INT()->getText()) : std::nullopt;
}

std::any CustomExpressionVisitor::visitNumberFromExpression(TSyrecParser::NumberFromExpressionContext* context) {
    if (!context)
        return std::nullopt;

    recordCustomError(mapTokenPositionToMessagePosition(*context->start), "Compile time constant expressions are not supported");
    return std::nullopt;
}

// TODO: Check that loop variable and 'normal' variable can share same identfier since the former is distinguished by the loop variable prefix '$'
std::any CustomExpressionVisitor::visitNumberFromLoopVariable(TSyrecParser::NumberFromLoopVariableContext* context) {
    if (!context || !context->IDENT())
        return std::nullopt;

    const std::optional<utils::TemporaryVariableScope::ptr> activeVariableScopeInSymbolTable = symbolTable->getActiveTemporaryScope();
    if (!activeVariableScopeInSymbolTable)
        return std::nullopt;

    const std::string loopVariableIdentifier = "$" + context->IDENT()->getText();
    if (const std::optional<utils::TemporaryVariableScope::ScopeEntry::readOnylPtr> matchingLoopVariableForIdentifier = activeVariableScopeInSymbolTable->get()->getVariableByName(loopVariableIdentifier); matchingLoopVariableForIdentifier.has_value() && matchingLoopVariableForIdentifier->get()->isReferenceToLoopVariable())
        return std::make_shared<syrec::Number>(loopVariableIdentifier);

    recordSemanticError<SemanticError::NoVariableMatchingIdentifier>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()), loopVariableIdentifier);
    return std::nullopt;
}

// TODO: Tests that bitwidth of loop variable cannot be fetched (results in syntax error)
std::any CustomExpressionVisitor::visitNumberFromSignalwidth(TSyrecParser::NumberFromSignalwidthContext* context) {
    if (!context || !context->IDENT())
        return std::nullopt;

    const std::optional<utils::TemporaryVariableScope::ptr> activeVariableScopeInSymbolTable = symbolTable->getActiveTemporaryScope();
    if (!activeVariableScopeInSymbolTable)
        return std::nullopt;

    const std::string& variableIdentifier = context->IDENT()->getSymbol()->getText();
    if (const std::optional<utils::TemporaryVariableScope::ScopeEntry::readOnylPtr> matchingVariableForIdentifier = activeVariableScopeInSymbolTable->get()->getVariableByName(variableIdentifier); matchingVariableForIdentifier.has_value())
        return matchingVariableForIdentifier->get()->getDeclaredVariableBitwidth();

    recordSemanticError<SemanticError::NoVariableMatchingIdentifier>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()), variableIdentifier);
    return std::nullopt;
}

// TODO: Substitution of loop variable values for compile time index checks
std::any CustomExpressionVisitor::visitSignal(TSyrecParser::SignalContext* context) {
    if (!context || !context->IDENT())
        return std::nullopt;

    const std::optional<utils::TemporaryVariableScope::ptr> activeVariableScopeInSymbolTable = symbolTable->getActiveTemporaryScope();
    if (!activeVariableScopeInSymbolTable)
        return std::nullopt;

    const std::string&                                                          variableIdentifier            = context->IDENT()->getSymbol()->getText();
    const std::optional<utils::TemporaryVariableScope::ScopeEntry::readOnylPtr> matchingVariableForIdentifier = activeVariableScopeInSymbolTable->get()->getVariableByName(variableIdentifier);
    if (!matchingVariableForIdentifier.has_value()) {
        recordSemanticError<SemanticError::NoVariableMatchingIdentifier>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()), variableIdentifier);
        return std::nullopt;
    }

    syrec::VariableAccess::ptr generatedVariableAccess = std::make_shared<syrec::VariableAccess>();
    // TODO: Current shared_pointer to const T does not allow for assignment of variable to variable access
    //generatedVariableAccess->setVar(*matchingVariableForIdentifier->get()->getReadonlyVariableData())

    const std::vector<unsigned int>& declaredValuesPerDimensionOfReferenceVariable = matchingVariableForIdentifier.value()->getDeclaredVariableDimensions();
    const std::size_t                numUserAccessedDimensions                     = context->accessedDimensions.size();
    generatedVariableAccess->indexes                                               = syrec::Expression::vec(numUserAccessedDimensions, nullptr);
    for (const auto& antlrContextForAccessedValueOfDimension: context->accessedDimensions) {
        if (const std::optional<syrec::Expression::ptr> exprDefiningAccessedValueOfDimension = visitNonTerminalSymbolWithSingleResult<syrec::Expression>(antlrContextForAccessedValueOfDimension); exprDefiningAccessedValueOfDimension.has_value())
            generatedVariableAccess->indexes.emplace_back(*exprDefiningAccessedValueOfDimension);
    }

    if (context->accessedDimensions.empty() && declaredValuesPerDimensionOfReferenceVariable.front() > 1)
        recordSemanticError<SemanticError::OmittingDimensionAccessOnlyPossibleFor1DSignalWithSingleValue>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()));

    if (numUserAccessedDimensions != declaredValuesPerDimensionOfReferenceVariable.size()) {
        if (numUserAccessedDimensions > declaredValuesPerDimensionOfReferenceVariable.size())
            recordSemanticError<SemanticError::TooManyDimensionsAccessed>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()), numUserAccessedDimensions, declaredValuesPerDimensionOfReferenceVariable.size());
        else
            recordSemanticError<SemanticError::TooFewDimensionsAccessed>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()), numUserAccessedDimensions, declaredValuesPerDimensionOfReferenceVariable.size());
    }
    
    if (const std::optional<utils::VariableAccessIndicesValidity> indexValidityOfUserDefinedAccessedValuesPerDimension = utils::validateVariableAccessIndices(*generatedVariableAccess); indexValidityOfUserDefinedAccessedValuesPerDimension.has_value() && !indexValidityOfUserDefinedAccessedValuesPerDimension->isValid()) {
        
        const std::size_t                numDimensionsToCheck          = declaredValuesPerDimensionOfReferenceVariable.size();
        for (std::size_t dimensionIdx = 0; dimensionIdx < numDimensionsToCheck; ++dimensionIdx) {
            const utils::VariableAccessIndicesValidity::IndexValidationResult validityOfAccessedValueOfDimension = indexValidityOfUserDefinedAccessedValuesPerDimension->accessedValuePerDimensionValidity.at(dimensionIdx);
            // TODO: We should not have to check whether the index validation result for the given index contains a value when an out of range access is reported.
            if (validityOfAccessedValueOfDimension.indexValidity == utils::VariableAccessIndicesValidity::IndexValidationResult::OutOfRange && validityOfAccessedValueOfDimension.indexValue.has_value())
                recordSemanticError<SemanticError::IndexOfAccessedValueForDimensionOutOfRange>(mapTokenPositionToMessagePosition(*context->accessedDimensions.at(dimensionIdx)->getStart()), validityOfAccessedValueOfDimension.indexValue.value(), dimensionIdx, declaredValuesPerDimensionOfReferenceVariable.at(dimensionIdx));
        }
    }

    if (!context->BITRANGE_START_PREFIX())
        return generatedVariableAccess;

    const std::optional<syrec::Number::ptr> bitRangeStart = context->bitStart ? visitNonTerminalSymbolWithSingleResult<syrec::Number>(context->bitStart) : std::nullopt;
    const std::optional<syrec::Number::ptr> bitRangeEnd   = context->bitRangeEnd ? visitNonTerminalSymbolWithSingleResult<syrec::Number>(context->bitRangeEnd) : bitRangeStart;

    if (bitRangeStart.has_value() && bitRangeEnd.has_value()) {
        generatedVariableAccess->range = std::make_pair(bitRangeStart.value(), bitRangeEnd.value());
        syrec::VariableAccess temporaryVariableAccess = syrec::VariableAccess();
        temporaryVariableAccess.setVar(generatedVariableAccess->var);
        temporaryVariableAccess.range = generatedVariableAccess->range;

        if (const std::optional<utils::VariableAccessIndicesValidity> indexValidityOfUserDefinedAccessOnBitrange = utils::validateVariableAccessIndices(temporaryVariableAccess); indexValidityOfUserDefinedAccessOnBitrange.has_value() && !indexValidityOfUserDefinedAccessOnBitrange->isValid()
            && indexValidityOfUserDefinedAccessOnBitrange->bitRangeAccessValidity.has_value()) {
            if (const utils::VariableAccessIndicesValidity::IndexValidationResult accessedBitRangeStartIndexValidity = indexValidityOfUserDefinedAccessOnBitrange->bitRangeAccessValidity->bitRangeStartValidity; accessedBitRangeStartIndexValidity.indexValidity == utils::VariableAccessIndicesValidity::IndexValidationResult::OutOfRange && accessedBitRangeStartIndexValidity.indexValue.has_value())
                recordSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(mapTokenPositionToMessagePosition(*context->bitStart->getStart()), accessedBitRangeStartIndexValidity.indexValue.value(), generatedVariableAccess->var->bitwidth);

            if (const utils::VariableAccessIndicesValidity::IndexValidationResult accessedBitRangEndIndexValidity = indexValidityOfUserDefinedAccessOnBitrange->bitRangeAccessValidity->bitRangeEndValiditiy; accessedBitRangEndIndexValidity.indexValidity == utils::VariableAccessIndicesValidity::IndexValidationResult::OutOfRange && accessedBitRangEndIndexValidity.indexValue.has_value())
                recordSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(mapTokenPositionToMessagePosition(*context->bitRangeEnd->getStart()), accessedBitRangEndIndexValidity.indexValue.value(), generatedVariableAccess->var->bitwidth);
        }
    }
    return generatedVariableAccess;
}

std::optional<unsigned> CustomExpressionVisitor::deserializeConstantFromString(const std::string& stringifiedConstantValue) {
    char* pointerToLastCharacterInString = nullptr;
    // Need to reset errno to not reuse already set values
    errno = 0;

    // Using this conversion method for any user provided constant value forces the maximum possible value of a constant that can be specified
    // by the user in a SyReC circuit to 2^32. Larger values are not truncated but reported as an error instead.
    const unsigned int constantValue = std::strtoul(stringifiedConstantValue.c_str(), &pointerToLastCharacterInString, 10);
    // Using these error conditions checks will detect strings of the form "0 " as not valid while " 0" is considered valid.
    if (stringifiedConstantValue.c_str() == pointerToLastCharacterInString || errno == ERANGE || *pointerToLastCharacterInString)
        return std::nullopt;

    return constantValue;
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