#include "core/syrec/parser/components/custom_statement_visitor.hpp"
#include "core/utils/base_syrec_ir_entity_stringifier.hpp"

using namespace syrecParser;

std::optional<syrec::Statement::vec> CustomStatementVisitor::visitStatementListTyped(TSyrecParser::StatementListContext* ctx) {
    return visitNonTerminalSymbolWithManyResults<syrec::Statement>(ctx);
}

std::optional<syrec::Statement::ptr> CustomStatementVisitor::visitStatementTyped(TSyrecParser::StatementContext* ctx) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Statement>(ctx);
}

std::optional<syrec::Statement::ptr> CustomStatementVisitor::visitAssignStatementTyped(TSyrecParser::AssignStatementContext* ctx) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Statement>(ctx);
}

std::optional<syrec::Statement::ptr> CustomStatementVisitor::visitUnaryStatementTyped(TSyrecParser::UnaryStatementContext* ctx) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Statement>(ctx);
}

std::optional<syrec::Statement::ptr> CustomStatementVisitor::visitSwapStatementTyped(TSyrecParser::SwapStatementContext* ctx) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Statement>(ctx);
}

std::optional<syrec::Statement::ptr> CustomStatementVisitor::visitSkipStatementTyped(TSyrecParser::SkipStatementContext* ctx) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Statement>(ctx);
}

std::optional<syrec::Statement::ptr> CustomStatementVisitor::visitCallStatementTyped(TSyrecParser::CallStatementContext* ctx) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Statement>(ctx);
}

std::optional<syrec::Statement::ptr> CustomStatementVisitor::visitIfStatementTyped(TSyrecParser::IfStatementContext* ctx) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Statement>(ctx);
}

std::optional<syrec::Statement::ptr> CustomStatementVisitor::visitForStatementTyped(TSyrecParser::ForStatementContext* ctx) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Statement>(ctx);
}

// START OF NON-PUBLIC FUNCTIONALITY
std::optional<std::string> CustomStatementVisitor::visitLoopVariableDefinitionTyped(TSyrecParser::LoopVariableDefinitionContext* ctx) const {
    if (!ctx || !ctx->IDENT())
        return std::nullopt;

    std::string loopVariableIdentifier = "$" + ctx->IDENT()->getText();
    if (const std::optional<utils::TemporaryVariableScope::ptr> activeSymbolTableScope = symbolTable->getActiveTemporaryScope(); activeSymbolTableScope.has_value() && activeSymbolTableScope->get()->getVariableByName(loopVariableIdentifier))
        recordSemanticError<SemanticError::DuplicateVariableDeclaration>(mapTokenPositionToMessagePosition(*ctx->IDENT()->getSymbol()), loopVariableIdentifier);

    return loopVariableIdentifier;
}

std::optional<CustomStatementVisitor::LoopStepsizeDefinition> CustomStatementVisitor::visitLoopStepsizeDefinitionTyped(TSyrecParser::LoopStepsizeDefinitionContext* ctx) {
    if (!ctx || ctx->number())
        return std::nullopt;

    if (ctx->OP_MINUS())
        recordSemanticError<SemanticError::NegativeStepsizeValueNotAllowed>(mapTokenPositionToMessagePosition(*ctx->OP_MINUS()->getSymbol()));

    return LoopStepsizeDefinition({ctx->OP_MINUS() != nullptr, visitNonTerminalSymbolWithSingleResult<syrec::Number>(ctx->number()).value_or(nullptr)});
}

std::any CustomStatementVisitor::visitStatementList(TSyrecParser::StatementListContext* ctx) {
    if (!ctx)
        return std::nullopt;

    syrec::Statement::vec statements;
    statements.reserve(ctx->stmts.size());

    for (const auto& antlrStatementContext: ctx->stmts)
        if (const std::optional<syrec::Statement::ptr> generatedStatement = visitStatementTyped(antlrStatementContext); generatedStatement.has_value() && !generatedStatement.value())
            statements.emplace_back(*generatedStatement);

    return statements;
}

std::any CustomStatementVisitor::visitStatement(TSyrecParser::StatementContext* ctx) {
    if (ctx->callStatement())
        return visitCallStatementTyped(ctx->callStatement());
    if (ctx->forStatement())
        return visitForStatementTyped(ctx->forStatement());
    if (ctx->ifStatement())
        return visitIfStatementTyped(ctx->ifStatement());
    if (ctx->unaryStatement())
        return visitUnaryStatementTyped(ctx->unaryStatement());
    if (ctx->assignStatement())
        return visitAssignStatementTyped(ctx->assignStatement());
    if (ctx->swapStatement())
        return visitSwapStatementTyped(ctx->swapStatement());
    if (ctx->skipStatement())
        return visitSkipStatementTyped(ctx->skipStatement());
    return std::nullopt;
}

std::any CustomStatementVisitor::visitAssignStatement(TSyrecParser::AssignStatementContext* ctx) {
    if (!ctx)
        return std::nullopt;

    const std::optional<syrec::VariableAccess::ptr> assignmentLhsOperand = visitNonTerminalSymbolWithSingleResult<syrec::VariableAccess>(ctx->signal());
    if (assignmentLhsOperand.has_value() && !doesVariableTypeAllowAssignment(assignmentLhsOperand->get()->var->type))
        recordSemanticError<SemanticError::AssignmentToReadonlyVariable>(mapTokenPositionToMessagePosition(*ctx->signal()->start), assignmentLhsOperand->get()->var->name);

    const std::optional<syrec::AssignStatement::AssignOperation> assignmentOperation  = ctx->assignmentOp ? deserializeAssignmentOperationFromString(ctx->assignmentOp->getText()) : std::nullopt;

    // TODO: Setting expected operand bitwidth from lhs operand
    // TODO: Perform overlap checks (use class member and check in expression visitor)
    const std::optional<syrec::Expression::ptr> assignmentRhsOperand = visitNonTerminalSymbolWithSingleResult<syrec::Expression>(ctx->expression());

    return assignmentLhsOperand.has_value() && assignmentOperation.has_value() && assignmentRhsOperand.has_value()
        ? std::optional(std::make_shared<syrec::AssignStatement>(*assignmentLhsOperand, *assignmentOperation, *assignmentRhsOperand))
        : std::nullopt;
}

std::any CustomStatementVisitor::visitUnaryStatement(TSyrecParser::UnaryStatementContext* ctx) {
    if (!ctx)
        return std::nullopt;

    const std::optional<syrec::VariableAccess::ptr> assignedToVariable = visitNonTerminalSymbolWithSingleResult<syrec::VariableAccess>(ctx->signal());
    if (assignedToVariable.has_value())
        recordErrorIfAssignmentToReadonlyVariableIsPerformed(*assignedToVariable->get()->var, *ctx->signal()->start);

    const std::optional<syrec::UnaryStatement::UnaryOperation> assignmentOperation = deserializeUnaryAssignmentOperationFromString(ctx->unaryOp->getText());
    return assignedToVariable.has_value() && assignmentOperation.has_value()
        ? std::make_optional(std::make_shared<syrec::UnaryStatement>(*assignmentOperation, *assignedToVariable))
        : std::nullopt;
}

std::any CustomStatementVisitor::visitSwapStatement(TSyrecParser::SwapStatementContext* ctx) {
    if (!ctx)
        return std::nullopt;

    const std::optional<syrec::VariableAccess::ptr> swapLhsOperand = visitNonTerminalSymbolWithSingleResult<syrec::VariableAccess>(ctx->lhsOperand);
    if (swapLhsOperand.has_value())
        recordErrorIfAssignmentToReadonlyVariableIsPerformed(*swapLhsOperand->get()->var, *ctx->lhsOperand->getStart());
    
    // TODO: Setting expected operand bitwidth from lhs operand
    // TODO: Perform overlap checks (use class member and check in expression visitor)
    const std::optional<syrec::VariableAccess::ptr> swapRhsOperand = visitNonTerminalSymbolWithSingleResult<syrec::VariableAccess>(ctx->rhsOperand);
    if (swapRhsOperand.has_value())
        recordErrorIfAssignmentToReadonlyVariableIsPerformed(*swapRhsOperand->get()->var, *ctx->rhsOperand->getStart());

    return swapLhsOperand.has_value() && swapRhsOperand.has_value()
        ? std::make_optional(std::make_shared<syrec::SwapStatement>(*swapLhsOperand, *swapRhsOperand))
        : std::nullopt;
}

std::any CustomStatementVisitor::visitSkipStatement(TSyrecParser::SkipStatementContext* ctx) {
    return std::make_shared<syrec::SkipStatement>();
}

std::any CustomStatementVisitor::visitCallStatement(TSyrecParser::CallStatementContext* ctx) {
    if (!ctx)
        return std::nullopt;

    const std::optional<std::string>                        calledModuleIdentifier = ctx->moduleIdent ? std::optional(ctx->moduleIdent->getText()) : std::nullopt;
    const std::optional<utils::TemporaryVariableScope::ptr> activeSymbolTableScope = symbolTable->getActiveTemporaryScope();
    // Should rename callee arguments to caller arguments in grammar
    std::vector<std::shared_ptr<const syrec::Variable>> symbolTableEntryPerCallerArgument;
    symbolTableEntryPerCallerArgument.reserve(ctx->calleeArguments.size());

    std::vector<std::string> callerArgumentVariableIdentifiers;
    callerArgumentVariableIdentifiers.reserve(ctx->calleeArguments.size());

    for (const auto& antlrCallerArgumentToken: ctx->calleeArguments) {
        if (!antlrCallerArgumentToken || !activeSymbolTableScope.has_value())
            continue;

        callerArgumentVariableIdentifiers.emplace_back(antlrCallerArgumentToken->getText());
        if (const std::optional<utils::TemporaryVariableScope::ScopeEntry::readOnylPtr> matchingSymbolTableEntryForCallerArgument = activeSymbolTableScope->get()->getVariableByName(antlrCallerArgumentToken->getText()); matchingSymbolTableEntryForCallerArgument.has_value()) {
            // TODO: Technically we should not have to check whether the requested variable data exists when a matching entry in the symbol table is found since no loop variable can match the same variable identifier due to the loop variable prefix
            if (matchingSymbolTableEntryForCallerArgument->get()->getReadonlyVariableData().has_value())
                symbolTableEntryPerCallerArgument.emplace_back(matchingSymbolTableEntryForCallerArgument->get()->getReadonlyVariableData().value());
        } else {
            recordSemanticError<SemanticError::NoVariableMatchingIdentifier>(mapTokenPositionToMessagePosition(*antlrCallerArgumentToken), antlrCallerArgumentToken->getText());
        }
    }

    // TODO: Provide correct caller arguments
    // TODO: Should overload resolution be performed if any of the provided caller arguments had no matching symbol table entry
    const utils::BaseSymbolTable::ModuleOverloadResolutionResult overloadResolutionResult = symbolTable->getModulesMatchingSignature(*calledModuleIdentifier, {});
    if (overloadResolutionResult.resolutionResult == utils::BaseSymbolTable::ModuleOverloadResolutionResult::SingleMatchFound) {
        if (overloadResolutionResult.moduleMatchingSignature.has_value()) {
            //return std::make_shared<syrec::CallStatement>(overloadResolutionResult.moduleMatchingSignature.value(), std::vector<std::string>());
            if (ctx->OP_CALL())
                return std::make_shared<syrec::CallStatement>(nullptr, callerArgumentVariableIdentifiers);
            return std::make_shared<syrec::UncallStatement>(nullptr, callerArgumentVariableIdentifiers);
        }
    } else {
        // TODO: Overload error handling
    }

    // TODO: Add tests for this behaviour
    // TODO: Cannot call module with identifier 'main' if such a module was defined (cannot be determined here)
    // TODO: Cannot call main module last defined in syrec module (cannot be determined here)
    // TODO: Cannot perform recursive call to itself
    return std::nullopt;
}

std::any CustomStatementVisitor::visitIfStatement(TSyrecParser::IfStatementContext* ctx) {
    if (!ctx)
        return std::nullopt;

    auto generatedIfStatement = std::make_shared<syrec::IfStatement>();
    generatedIfStatement->setCondition(visitNonTerminalSymbolWithSingleResult<syrec::Expression>(ctx->guardCondition).value_or(nullptr));
    generatedIfStatement->thenStatements = visitNonTerminalSymbolWithManyResults<syrec::Statement>(ctx->trueBranchStmts).value_or(syrec::Statement::vec());
    generatedIfStatement->elseStatements = visitNonTerminalSymbolWithManyResults<syrec::Statement>(ctx->falseBranchStmts).value_or(syrec::Statement::vec());
    generatedIfStatement->setFiCondition(visitNonTerminalSymbolWithSingleResult<syrec::Expression>(ctx->matchingGuardExpression).value_or(nullptr));

    if (generatedIfStatement->condition && generatedIfStatement->fiCondition) {
        std::ostringstream stringifiedGuardConditionContainer;
        std::ostringstream stringifiedClosingGuardConditionContainer;
        // TODO: Should we use a single instance stored as a class member or can we create a new instance for every IfStatement 
        auto expressionStringifier = utils::BaseSyrecIrEntityStringifier(std::nullopt);
        // TODO: Replace with expressions which requires that the stringification functionality for expressions needs to be public (could also involve making the stringification of statements to be public)
        if (expressionStringifier.stringify(stringifiedGuardConditionContainer, syrec::Program()) && expressionStringifier.stringify(stringifiedClosingGuardConditionContainer, syrec::Program())
            && stringifiedGuardConditionContainer.str() != stringifiedClosingGuardConditionContainer.str()) {
            recordSemanticError<SemanticError::IfGuardExpressionMissmatch>(mapTokenPositionToMessagePosition(*ctx->guardCondition->getStart()));
        }
    }

    // TODO: Similarily to the processing of a for statement we need to determine how the parser processes empty statements list provided by the user (will this visitor function be called or not)?
    return generatedIfStatement;
}

std::any CustomStatementVisitor::visitLoopVariableDefinition(TSyrecParser::LoopVariableDefinitionContext* ctx) {
    return visitLoopVariableDefinitionTyped(ctx);
}

std::any CustomStatementVisitor::visitLoopStepsizeDefinition(TSyrecParser::LoopStepsizeDefinitionContext* ctx) {
    return visitLoopStepsizeDefinitionTyped(ctx);
}

std::any CustomStatementVisitor::visitForStatement(TSyrecParser::ForStatementContext* ctx) {
    if (!ctx)
        return std::nullopt;

    const std::optional<utils::TemporaryVariableScope::ptr> activeSymbolTableScope = symbolTable->getActiveTemporaryScope();
    const std::optional<std::string>                        loopVariableIdentifier = visitLoopVariableDefinitionTyped(ctx->loopVariableDefinition());

    const std::optional<syrec::Number::ptr>     iterationRangeStartValue         = visitNonTerminalSymbolWithSingleResult<syrec::Number>(ctx->startValue);
    const std::optional<syrec::Number::ptr>     iterationRangeEndValue           = visitNonTerminalSymbolWithSingleResult<syrec::Number>(ctx->endValue);
    const std::optional<LoopStepsizeDefinition> iterationRangeStepSizeDefinition = visitLoopStepsizeDefinitionTyped(ctx->loopStepsizeDefinition());
    const std::optional<syrec::Number::ptr>     iterationRangeStepsizeValue      = iterationRangeStepSizeDefinition.has_value() ? std::make_optional(iterationRangeStepSizeDefinition->stepsize) : std::nullopt;

    auto generatedForStatement = std::make_shared<syrec::ForStatement>();
    generatedForStatement->loopVariable = loopVariableIdentifier.value_or("");
    if (!loopVariableIdentifier.has_value() && activeSymbolTableScope.has_value())
        activeSymbolTableScope->get()->recordLoopVariable(std::make_shared<syrec::Number>(*loopVariableIdentifier));

    if (iterationRangeStartValue.has_value())
        generatedForStatement->range        = ctx->endValue != nullptr
            ? std::make_pair(*iterationRangeStartValue, iterationRangeEndValue.value_or(nullptr))
            : std::make_pair(*iterationRangeStartValue, *iterationRangeStartValue);

    generatedForStatement->step         = iterationRangeStepsizeValue.value_or(std::make_shared<syrec::Number>(1));

    // TODO: Does the parser prevent the execution of this function for loops with an empty statement body (due to the statement list production being required to consist of at least one statement) or do we need to check the validity of the statement body manually?
    // TODO: Make value range of loop variable available for index checks in expressions
    if (ctx->statementList())
        generatedForStatement->statements = visitNonTerminalSymbolWithManyResults<syrec::Statement>(ctx->statementList()).value_or(syrec::Statement::vec());

    if (!loopVariableIdentifier.has_value() && activeSymbolTableScope.has_value())
        activeSymbolTableScope->get()->removeVariable(*loopVariableIdentifier);

    return  generatedForStatement->step && generatedForStatement->range.first && generatedForStatement->range.second
        ? std::make_optional(generatedForStatement)
        : std::nullopt;
}

void CustomStatementVisitor::recordErrorIfAssignmentToReadonlyVariableIsPerformed(const syrec::Variable& accessedVariable, const antlr4::Token& reportedErrorPosition) const {
    if (!doesVariableTypeAllowAssignment(accessedVariable.type))
        recordSemanticError<SemanticError::AssignmentToReadonlyVariable>(mapTokenPositionToMessagePosition(reportedErrorPosition), accessedVariable.name);
}

std::optional<syrec::AssignStatement::AssignOperation> CustomStatementVisitor::deserializeAssignmentOperationFromString(const std::string_view& stringifiedAssignmentOperation) {
    if (stringifiedAssignmentOperation == "+=")
        return syrec::AssignStatement::AssignOperation::Add;
    if (stringifiedAssignmentOperation == "-=")
        return syrec::AssignStatement::AssignOperation::Subtract;
    if (stringifiedAssignmentOperation == "^=")
        return syrec::AssignStatement::AssignOperation::Exor;
    return std::nullopt;
}

std::optional<syrec::UnaryStatement::UnaryOperation> CustomStatementVisitor::deserializeUnaryAssignmentOperationFromString(const std::string_view& stringifiedUnaryAssignmentOperation) {
    if (stringifiedUnaryAssignmentOperation == "++=")
        return syrec::UnaryStatement::UnaryOperation::Increment;
    if (stringifiedUnaryAssignmentOperation == "--=")
        return syrec::UnaryStatement::UnaryOperation::Decrement;
    if (stringifiedUnaryAssignmentOperation == "~=")
        return syrec::UnaryStatement::UnaryOperation::Invert;
    return std::nullopt;
}