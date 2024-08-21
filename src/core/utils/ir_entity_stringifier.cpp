#include "core/utils/ir_entity_stringifier.hpp"

#include <sstream>

using namespace syrec;

std::optional<std::string> IrEntityStringifier::stringify(const Program& program) {
    if (program.modules().empty())
        return std::nullopt;

    std::ostringstream stringifiedModulesBuffer;
    const auto&        programModules = program.modules();
    for (std::size_t i = 0; i < programModules.size(); ++i) {
        if (const std::optional<std::string>& stringifiedModule = programModules.at(i) ? stringify(*programModules.at(i)) : std::nullopt; stringifiedModule.has_value())
            stringifiedModulesBuffer << *stringifiedModule;
        else
            return std::nullopt;

        if (i != programModules.size() - 1)
            stringifiedModulesBuffer << stringifiedIdentsLookup.newLineIdent << stringifiedIdentsLookup.newLineIdent;
    }
    return stringifiedModulesBuffer.str();
}

std::optional<std::string> IrEntityStringifier::stringify(const Module& programModule) {
    if (programModule.statements.empty())
        return std::nullopt;

    std::ostringstream stringifiedModuleBuffer;
    stringifiedModuleBuffer << stringifiedIdentsLookup.moduleKeywordIdent << " " << programModule.name << "(";

    if (const std::optional<std::string>& stringifiedModuleParameters = stringifyCollection(programModule.variables); stringifiedModuleParameters.has_value())
        stringifiedModuleBuffer << *stringifiedModuleParameters << ")";
    else
        return std::nullopt;

    if (const std::optional<std::string>& stringifiedModuleLocalVariables = stringifyCollection(programModule.variables); stringifiedModuleLocalVariables.has_value())
        stringifiedModuleBuffer << stringifiedIdentsLookup.newLineIdent << *stringifiedModuleLocalVariables;
    else
        return std::nullopt;

    if (!incrementIdentationLevel())
        return std::nullopt;

    if (const std::optional<std::string>& stringifiedModuleStatements = stringifyCollection(programModule.statements); stringifiedModuleStatements.has_value())
        stringifiedModuleBuffer << stringifiedIdentsLookup.newLineIdent << *stringifiedModuleStatements;
    else
        return std::nullopt;

    if (!decrementIdentationLevel())
        return std::nullopt;

    return stringifiedModuleBuffer.str();
}

std::optional<std::string> IrEntityStringifier::stringify(const Variable& variable) const {
    std::ostringstream stringifiedVariableBuffer;

    if (variable.name.empty())
        return std::nullopt;

    if (const std::optional<std::string>& stringifiedVariableType = stringifyVariableType(variable.type); stringifiedVariableType.has_value())
        stringifiedVariableBuffer << *stringifiedVariableType << " " << variable.name;
    else
        return std::nullopt;

    for (const auto& variableDimension: variable.dimensions)
        stringifiedVariableBuffer << "[" << std::to_string(variableDimension) << "]";

    stringifiedVariableBuffer << "(" << std::to_string(variable.bitwidth) << ")";
    return stringifiedVariableBuffer.str();
}

std::optional<std::string> IrEntityStringifier::stringify(const Statement& statement) const {
    if (const auto* castedAssignStatement = dynamic_cast<const AssignStatement*>(&statement); castedAssignStatement)
        return stringify(*castedAssignStatement);
    if (const auto* castedCallStatement = dynamic_cast<const CallStatement*>(&statement); castedCallStatement)
        return stringify(*castedCallStatement);
    if (const auto* castedForStatement = dynamic_cast<const ForStatement*>(&statement); castedForStatement)
        return stringify(*castedForStatement);
    if (const auto* castedIfStatement = dynamic_cast<const IfStatement*>(&statement); castedIfStatement)
        return stringify(*castedIfStatement);
    if (const auto* castedSwapStatement = dynamic_cast<const SwapStatement*>(&statement); castedSwapStatement)
        return stringify(*castedSwapStatement);
    if (const auto* castedUnaryAssignStatement = dynamic_cast<const UnaryStatement*>(&statement); castedUnaryAssignStatement)
        return stringify(*castedUnaryAssignStatement);
    if (const auto* castedUncallStatement = dynamic_cast<const UncallStatement*>(&statement); castedUncallStatement)
        return stringify(*castedUncallStatement);
    if (typeid(statement) == typeid(SkipStatement))
        return stringifySkipStatement();

    return std::nullopt;
}

std::optional<std::string> IrEntityStringifier::stringify(const AssignStatement& assignStatement) const {
    std::ostringstream stringifiedAssignStatementBuffer;
    if (!appendIdentationPaddingSequence(stringifiedAssignStatementBuffer, stringifiedIdentsLookup.identationCharacterSequence, currIdentationLevel))
        return std::nullopt;

    if (const std::optional<std::string>& stringifiedLhsOperand = assignStatement.lhs ? stringify(*assignStatement.lhs) : std::nullopt; stringifiedLhsOperand.has_value())
        stringifiedAssignStatementBuffer << *stringifiedLhsOperand << " ";
    else
        return std::nullopt;

    if (const std::optional<std::string>& stringifiedAssignOperation = stringifyOperation(assignStatement.assignOperation); stringifiedAssignOperation.has_value())
        stringifiedAssignStatementBuffer << *stringifiedAssignOperation << " ";
    else
        return std::nullopt;

    if (const std::optional<std::string>& stringifiedRhsOperand = assignStatement.rhs ? stringify(*assignStatement.rhs) : std::nullopt; stringifiedRhsOperand.has_value())
        stringifiedAssignStatementBuffer << *stringifiedRhsOperand << " ";
    else
        return std::nullopt;

    return stringifiedAssignStatementBuffer.str();
}

std::optional<std::string> IrEntityStringifier::stringify(const CallStatement& callStatement) {
    if (!callStatement.target)
        return std::nullopt;

    return stringifyCallVariant(stringifiedIdentsLookup.callKeywordIdent, callStatement.target->name, callStatement.parameters);
}

std::optional<std::string> IrEntityStringifier::stringify(const ForStatement& forStatement) {
    std::ostringstream stringifiedForStatementBuffer;
    if (!appendIdentationPaddingSequence(stringifiedForStatementBuffer, stringifiedIdentsLookup.identationCharacterSequence, currIdentationLevel))
        return std::nullopt;

    stringifiedForStatementBuffer << stringifiedIdentsLookup.forKeywordIdent << " ";

    if (forStatement.range.first) {
        if (!forStatement.loopVariable.empty())
            stringifiedForStatementBuffer << stringifiedIdentsLookup.loopVariablePrefixIdent << forStatement.loopVariable << " = ";

        if (const std::optional<std::string>& stringifiedIterationRangeStart = forStatement.range.first ? stringify(*forStatement.range.first) : std::nullopt; stringifiedIterationRangeStart.has_value())
            stringifiedForStatementBuffer << *stringifiedIterationRangeStart << " ";
        else
            return std::nullopt;
    }

    if (forStatement.range.second && (!forStatement.range.first || forStatement.range.second != forStatement.range.first)) {
        if (const std::optional<std::string>& stringifiedIterationRangeEnd = forStatement.range.second ? stringify(*forStatement.range.second) : std::nullopt; stringifiedIterationRangeEnd.has_value())
            stringifiedForStatementBuffer << stringifiedIdentsLookup.toKeywordIdent << " " << *stringifiedIterationRangeEnd << " ";
        else
            return std::nullopt;
    }

    if (forStatement.step) {
        if (const std::optional<std::string>& stringifiedStepSize = stringify(*forStatement.step); stringifiedStepSize.has_value())
            stringifiedForStatementBuffer << stringifiedIdentsLookup.stepKeywordIdent << " " << *stringifiedStepSize << " ";
    }
    stringifiedForStatementBuffer << stringifiedIdentsLookup.doKeywordIdent << stringifiedIdentsLookup.newLineIdent;
    if (!incrementIdentationLevel())
        return std::nullopt;

    if (const std::optional<std::string>& stringifiedLoopBodyStatements = stringifyCollection(forStatement.statements); stringifiedLoopBodyStatements.has_value())
        stringifiedForStatementBuffer << *stringifiedLoopBodyStatements << stringifiedIdentsLookup.newLineIdent;
    else
        return std::nullopt;

    if (!decrementIdentationLevel())
        return std::nullopt;

    stringifiedForStatementBuffer << stringifiedIdentsLookup.rofKeywordIdent;
    return stringifiedForStatementBuffer.str();
}

std::optional<std::string> IrEntityStringifier::stringify(const IfStatement& ifStatement) {
    std::ostringstream stringifiedIfStatementBuffer;
    if (!appendIdentationPaddingSequence(stringifiedIfStatementBuffer, stringifiedIdentsLookup.identationCharacterSequence, currIdentationLevel))
        return std::nullopt;

    if (const std::optional<std::string>& stringifiedIfCondition = ifStatement.condition ? stringify(*ifStatement.condition) : std::nullopt; stringifiedIfCondition.has_value())
        stringifiedIfStatementBuffer << stringifiedIdentsLookup.ifKeywordIdent << " " << *stringifiedIfCondition << " " << stringifiedIdentsLookup.thenKeywordIdent << stringifiedIdentsLookup.newLineIdent;
    else
        return std::nullopt;

    if (!incrementIdentationLevel())
        return std::nullopt;

    if (const std::optional<std::string>& stringifiedThenStatements = stringifyCollection(ifStatement.thenStatements); stringifiedThenStatements.has_value())
        stringifiedIfStatementBuffer << *stringifiedThenStatements << stringifiedIdentsLookup.newLineIdent;
    else
        return std::nullopt;

    if (!decrementIdentationLevel())
        return std::nullopt;

    stringifiedIfStatementBuffer << stringifiedIdentsLookup.elseKeywordIdent << stringifiedIdentsLookup.newLineIdent;

    if (!incrementIdentationLevel())
        return std::nullopt;

    if (const std::optional<std::string>& stringifiedElseStatements = stringifyCollection(ifStatement.elseStatements); stringifiedElseStatements.has_value())
        stringifiedIfStatementBuffer << *stringifiedElseStatements << stringifiedIdentsLookup.newLineIdent;
    else
        return std::nullopt;

    if (!decrementIdentationLevel())
        return std::nullopt;

    if (const std::optional<std::string>& stringifiedFiCondition = ifStatement.fiCondition ? stringify(*ifStatement.fiCondition) : std::nullopt; stringifiedFiCondition.has_value())
        stringifiedIfStatementBuffer << stringifiedIdentsLookup.fiKeywordIdent << " " << *stringifiedFiCondition;
    else
        return std::nullopt;

    return stringifiedIfStatementBuffer.str();
}

std::optional<std::string> IrEntityStringifier::stringify(const SwapStatement& swapStatement) const {
    std::ostringstream stringifedSwapStatementBuffer;
    if (!appendIdentationPaddingSequence(stringifedSwapStatementBuffer, stringifiedIdentsLookup.identationCharacterSequence, currIdentationLevel))
        return std::nullopt;

    if (const std::optional<std::string>& stringifiedLhsOperand = swapStatement.lhs ? stringify(*swapStatement.lhs) : std::nullopt; stringifiedLhsOperand.has_value())
        stringifedSwapStatementBuffer << *stringifiedLhsOperand;
    else
        return std::nullopt;

    stringifedSwapStatementBuffer << " " << stringifiedIdentsLookup.swapOperationIdent << " ";

    if (const std::optional<std::string>& stringifiedRhsOperand = swapStatement.rhs ? stringify(*swapStatement.rhs) : std::nullopt; stringifiedRhsOperand.has_value())
        stringifedSwapStatementBuffer << *stringifiedRhsOperand;
    else
        return std::nullopt;

    return stringifedSwapStatementBuffer.str();
}

std::optional<std::string> IrEntityStringifier::stringify(const UnaryStatement& unaryAssignStatement) const {
    std::ostringstream stringifiedUnaryAssignStatementBuffer;
    if (!appendIdentationPaddingSequence(stringifiedUnaryAssignStatementBuffer, stringifiedIdentsLookup.identationCharacterSequence, currIdentationLevel))
        return std::nullopt;

    if (const std::optional<std::string>& stringifiedUnaryOperation = stringifyOperation(unaryAssignStatement.unaryOperation); stringifiedUnaryOperation.has_value())
        stringifiedUnaryAssignStatementBuffer << *stringifiedUnaryOperation << " ";
    else
        return std::nullopt;

    if (const std::optional<std::string>& stringifiedOperand = unaryAssignStatement.var ? stringify(*unaryAssignStatement.var) : std::nullopt; stringifiedOperand.has_value())
        stringifiedUnaryAssignStatementBuffer << *stringifiedOperand;
    else
        return std::nullopt;

    return stringifiedUnaryAssignStatementBuffer.str();
}

std::optional<std::string> IrEntityStringifier::stringify(const UncallStatement& uncallStatement) {
    if (!uncallStatement.target)
        return std::nullopt;

    return stringifyCallVariant(stringifiedIdentsLookup.uncallKeywordIdent, uncallStatement.target->name, uncallStatement.parameters);
}

std::optional<std::string> IrEntityStringifier::stringifySkipStatement() const noexcept {
    return stringifiedIdentsLookup.skipKeywordIdent;
}

std::optional<std::string> IrEntityStringifier::stringify(const Expression& expression) const {
    if (const auto* castedBinaryExpression = dynamic_cast<const BinaryExpression*>(&expression); castedBinaryExpression)
        return stringify(*castedBinaryExpression);
    if (const auto* castedVariableExpression = dynamic_cast<const VariableExpression*>(&expression); castedVariableExpression)
        return stringify(*castedVariableExpression);
    if (const auto& castedNumericExpression = dynamic_cast<const NumericExpression*>(&expression); castedNumericExpression)
        return stringify(*castedNumericExpression);

    return std::nullopt;
}

std::optional<std::string> IrEntityStringifier::stringify(const BinaryExpression& binaryExpression) const {
    std::ostringstream stringifiedBinaryExpressionBuffer;
    if (const std::optional<std::string>& stringifiedLhsOperand = binaryExpression.lhs ? stringify(*binaryExpression.lhs) : std::nullopt; stringifiedLhsOperand.has_value())
        stringifiedBinaryExpressionBuffer << *stringifiedLhsOperand << " ";
    else
        return std::nullopt;

    if (const std::optional<std::string>& stringifiedBinaryOperation = stringifyOperation(binaryExpression.binaryOperation); stringifiedBinaryOperation.has_value())
        stringifiedBinaryExpressionBuffer << *stringifiedBinaryOperation << " ";
    else
        return std::nullopt;

    if (const std::optional<std::string>& stringifiedRhsOperand = binaryExpression.rhs ? stringify(*binaryExpression.rhs) : std::nullopt; stringifiedRhsOperand.has_value())
        stringifiedBinaryExpressionBuffer << *stringifiedRhsOperand;
    else
        return std::nullopt;

    return stringifiedBinaryExpressionBuffer.str();
}

std::optional<std::string> IrEntityStringifier::stringify(const VariableExpression& variableExpression) const {
    if (!variableExpression.var)
        return std::nullopt;

    return stringify(*variableExpression.var);
}

std::optional<std::string> IrEntityStringifier::stringify(const NumericExpression& numericExpression) const {
    if (!numericExpression.value)
        return std::nullopt;

    return stringify(*numericExpression.value);
}

std::optional<std::string> IrEntityStringifier::stringify(const VariableAccess& variableAccess) const {
    if (!variableAccess.var)
        return std::nullopt;

    std::ostringstream stringifiedVariableAccessBuffer;
    stringifiedVariableAccessBuffer << variableAccess.var->name;

    for (const auto& accessedDimension : variableAccess.indexes) {
        if (const std::optional<std::string>& stringifiedAccessOnDimension = accessedDimension ? stringify(*accessedDimension) : std::nullopt; stringifiedAccessOnDimension.has_value())
            stringifiedVariableAccessBuffer << "[" << *stringifiedAccessOnDimension << "]";
        else
            return std::nullopt;
    }

    if (!variableAccess.range.has_value())
        return stringifiedVariableAccessBuffer.str();

    if (const std::optional<std::string>& stringifiedBitRangeStart = variableAccess.range->first ? stringify(*variableAccess.range->first) : std::nullopt; stringifiedBitRangeStart.has_value())
        stringifiedVariableAccessBuffer << stringifiedIdentsLookup.bitRangeStartIdent << *stringifiedBitRangeStart;

    if (const std::optional<std::string>& stringifiedBitRangeEnd = variableAccess.range->second && variableAccess.range->first != variableAccess.range->second ? stringify(*variableAccess.range->second) : std::nullopt; stringifiedBitRangeEnd.has_value())
        stringifiedVariableAccessBuffer << stringifiedIdentsLookup.bitRangeEndIdent << *stringifiedBitRangeEnd;

    return std::nullopt;
}

std::optional<std::string> IrEntityStringifier::stringify(const Number& number) const {
    if (number.isConstant())
        return std::to_string(number.evaluate({}));
    if (number.isLoopVariable())
        return stringifiedIdentsLookup.loopVariablePrefixIdent + number.variableName();

    return std::nullopt;
}

bool IrEntityStringifier::incrementIdentationLevel() noexcept {
    if (currIdentationLevel == SIZE_MAX)
        return false;

    ++currIdentationLevel;
    return true;
}

bool IrEntityStringifier::decrementIdentationLevel() noexcept {
    if (currIdentationLevel == 0)
        return false;

    --currIdentationLevel;
    return true;
}

std::optional<std::string> IrEntityStringifier::stringifyCollection(const Variable::vec& variables) const {
    if (variables.empty())
        return "";

    std::ostringstream stringifiedVariablesBuffer;
    for (std::size_t i = 0; i < variables.size(); ++i) {
        if (const std::optional<std::string>& stringifiedVariable = variables.at(i) ? stringify(*variables.at(i)) : std::nullopt; stringifiedVariable.has_value()) {
            stringifiedVariablesBuffer << *stringifiedVariable;
            if (i != variables.size() - 1)
                stringifiedVariablesBuffer << ", ";
        } else {
            return std::nullopt;
        }
    }
    return stringifiedVariablesBuffer.str();
}

std::optional<std::string> IrEntityStringifier::stringifyCollection(const Statement::vec& statements) const {
    if (statements.empty())
        return "";

    std::ostringstream stringifiedStatementsBuffer;
    for (std::size_t i = 0; i < statements.size(); ++i) {
        if (const std::optional<std::string>& stringifiedStatement = statements.at(i) ? stringify(*statements.at(i)) : std::nullopt; stringifiedStatement.has_value()) {
            if (!appendIdentationPaddingSequence(stringifiedStatementsBuffer, stringifiedIdentsLookup.identationCharacterSequence, currIdentationLevel))
                return std::nullopt;

            stringifiedStatementsBuffer << *stringifiedStatement;
            if (i != statements.size() - 1)
                stringifiedStatementsBuffer << ";" << stringifiedIdentsLookup.newLineIdent;
        } else {
            return std::nullopt;
        }
    }
    return stringifiedStatementsBuffer.str();
}

std::optional<std::string> IrEntityStringifier::stringifyVariableType(const Variable::Type variableType) const noexcept {
    switch (variableType) {
        case Variable::Type::In:
            return stringifiedIdentsLookup.inVariableTypeIdent;
        case Variable::Type::Out:
            return stringifiedIdentsLookup.outVariableTypeIdent;
        case Variable::Type::Inout:
            return stringifiedIdentsLookup.inoutVariableTypeIdent;
        case Variable::Type::Wire:
            return stringifiedIdentsLookup.wireVariableTypeIdent;
        case Variable::Type::State:
            return stringifiedIdentsLookup.stateVariableTypeIdent;
        default:
            return std::nullopt;
    }
}

std::optional<std::string> IrEntityStringifier::stringifyCallVariant(const std::string& operationKeyword, const std::string& callTargetName, const std::vector<std::string>& callTargetParameters) noexcept {
    std::ostringstream stringifiedUncallBuffer;
    if (!appendIdentationPaddingSequence(stringifiedUncallBuffer, stringifiedIdentsLookup.identationCharacterSequence, currIdentationLevel))
        return std::nullopt;

    if (callTargetParameters.empty()) {
        return operationKeyword + " " + callTargetName + "()";   
    }

    stringifiedUncallBuffer << operationKeyword << " " << callTargetName + "(";
    for (std::size_t i = 0; i < callTargetParameters.size() - 1; ++i) {
        if (callTargetParameters.at(i).empty())
            return std::nullopt;

        stringifiedUncallBuffer << callTargetParameters.at(i) << ", ";
    }

    if (callTargetParameters.back().empty())
        return std::nullopt;

    stringifiedUncallBuffer << callTargetParameters.back();
    return stringifiedUncallBuffer.str();
}

std::optional<std::string> IrEntityStringifier::stringifyOperation(const BinaryExpression::BinaryOperation operation) noexcept {
    switch (operation) {
        case BinaryExpression::BinaryOperation::Add:
            return "+";
        case BinaryExpression::BinaryOperation::BitwiseAnd:
            return "&";
        case BinaryExpression::BinaryOperation::BitwiseOr:
            return "|";
        case BinaryExpression::BinaryOperation::Divide:
            return "/";
        case BinaryExpression::BinaryOperation::Equals:
            return "==";
        case BinaryExpression::BinaryOperation::Exor:
            return "^";
        case BinaryExpression::BinaryOperation::FracDivide:
            return "*>";
        case BinaryExpression::BinaryOperation::GreaterEquals:
            return ">=";
        case BinaryExpression::BinaryOperation::GreaterThan:
            return ">";
        case BinaryExpression::BinaryOperation::LessEquals:
            return "<=";
        case BinaryExpression::BinaryOperation::LessThan:
            return "<";
        case BinaryExpression::BinaryOperation::LogicalAnd:
            return "&&";
        case BinaryExpression::BinaryOperation::LogicalOr:
            return "||";
        case BinaryExpression::BinaryOperation::Modulo:
            return "%";
        case BinaryExpression::BinaryOperation::Multiply:
            return "*";
        case BinaryExpression::BinaryOperation::NotEquals:
            return "!=";
        case BinaryExpression::BinaryOperation::Subtract:
            return "-";
        default:
            return std::nullopt;   
    }
}

std::optional<std::string> IrEntityStringifier::stringifyOperation(const AssignStatement::AssignOperation operation) noexcept {
    switch (operation) {
        case AssignStatement::AssignOperation::Add:
            return "+=";
        case AssignStatement::AssignOperation::Subtract:
            return "-=";
        case AssignStatement::AssignOperation::Exor:
            return "^=";
        default:
            return std::nullopt;
    }
}

std::optional<std::string> IrEntityStringifier::stringifyOperation(const UnaryStatement::UnaryOperation operation) noexcept {
    switch (operation) {
        case UnaryStatement::UnaryOperation::Increment:
            return "++=";
        case UnaryStatement::UnaryOperation::Decrement:
            return "--=";
        case UnaryStatement::UnaryOperation::Invert:
            return "~=";
        default:
            return std::nullopt;
    }
}

bool IrEntityStringifier::appendIdentationPaddingSequence(std::ostringstream& streamToAppendTo, const std::string& identiationSequence, std::string::size_type currentIdentationLevel) noexcept {
    if (identiationSequence.empty() || !currentIdentationLevel)
        return true;

    if (currentIdentationLevel == SIZE_MAX)
        return false;

    streamToAppendTo << std::string(identiationSequence, currentIdentationLevel);
    return true;
}