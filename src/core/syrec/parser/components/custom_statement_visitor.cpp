#include "core/syrec/parser/components/custom_statement_visitor.hpp"

using namespace syrecParser;

// We currently have no other choice than to split the function header and its implementation into two separate but same anonymous namespace
// if we are planning to use said function before its implementation was provided in an anonymous namespace.
namespace {
    std::optional<bool> doExpressionsMatch(const syrec::Expression::ptr& lExpr, const syrec::Expression::ptr& rExpr);
}

namespace {
    std::optional<bool> doVariableInstancesMatch(const syrec::Variable::ptr& lVar, const syrec::Variable::ptr& rVar) {
        if (lVar == nullptr ^ rVar == nullptr)
            return false;
        if (!lVar)
            return std::nullopt;

        return lVar->name == rVar->name && lVar->type == rVar->type && lVar->bitwidth == rVar->bitwidth && lVar->dimensions.size() == rVar->dimensions.size() && std::equal(lVar->dimensions.cbegin(), lVar->dimensions.cend(), rVar->dimensions.cbegin(), rVar->dimensions.cend());
    }

    std::optional<bool> doExpressionComponentsMatch(const syrec::Number::ptr& lNumber, const syrec::Number::ptr& rNumber) {
        if (lNumber == nullptr ^ rNumber == nullptr)
            return false;
        if (!lNumber)
            return std::nullopt;

        if (lNumber->isConstant())
            return rNumber->isConstant() && lNumber->evaluate({}) == rNumber->evaluate({});
        if (lNumber->isLoopVariable())
            return rNumber->isLoopVariable() && lNumber->variableName() == rNumber->variableName();
        if (lNumber->isConstantExpression()) {
            const std::optional<syrec::Number::ConstantExpression>& lConstantExpression = lNumber->constantExpression();
            const std::optional<syrec::Number::ConstantExpression>& rConstantExpression = rNumber->constantExpression();
            if (lConstantExpression.has_value() != rConstantExpression.has_value())
                return false;
            if (!lConstantExpression.has_value())
                return std::nullopt;

            const std::optional<bool> doesLhsOperandMatch = doExpressionComponentsMatch(lConstantExpression->lhsOperand, rConstantExpression->lhsOperand);
            if (doesLhsOperandMatch.value_or(false)) {
                if (lConstantExpression->operation != rConstantExpression->operation)
                    return false;
                return doExpressionComponentsMatch(lConstantExpression->rhsOperand, rConstantExpression->rhsOperand);
            }
            return doesLhsOperandMatch;
        }
        return std::nullopt;
    }

    std::optional<bool> doExpressionComponentsMatch(const syrec::VariableAccess::ptr& lVarAccess, const syrec::VariableAccess::ptr& rVarAccess) {
        if (lVarAccess == nullptr ^ rVarAccess == nullptr)
            return false;
        if (!lVarAccess)
            return std::nullopt;

        std::optional<bool> containerForMatchResult = doVariableInstancesMatch(lVarAccess->var, rVarAccess->var);
        if (containerForMatchResult.value_or(false)) {
            if (lVarAccess->range.has_value() == rVarAccess->range.has_value()) {
                if (lVarAccess->range.has_value()) {
                    containerForMatchResult = doExpressionComponentsMatch(lVarAccess->range->first, rVarAccess->range->first);
                    if (containerForMatchResult.value_or(false)) {
                        containerForMatchResult = doExpressionComponentsMatch(lVarAccess->range->second, rVarAccess->range->second);
                    }
                }
            } else {
                containerForMatchResult.reset();
            }

            if (containerForMatchResult.value_or(false)) {
                if (lVarAccess->indexes.size() != rVarAccess->indexes.size())
                    return false;

                for (std::size_t i = 0; i < lVarAccess->indexes.size() && containerForMatchResult.value_or(false); ++i) {
                    containerForMatchResult = doExpressionsMatch(lVarAccess->indexes.at(i), rVarAccess->indexes.at(i));
                }
            }
        }
        return containerForMatchResult;
    }

    std::optional<bool> doExpressionsMatch(const syrec::BinaryExpression& lExpr, const syrec::BinaryExpression& rExpr) {
        const std::optional<bool> doLhsOperandsMatch = doExpressionsMatch(lExpr.lhs, rExpr.lhs);
        if (doLhsOperandsMatch.value_or(false)) {
            if (lExpr.binaryOperation != rExpr.binaryOperation)
                return false;
            return doExpressionsMatch(lExpr.rhs, rExpr.rhs);
        }
        return doLhsOperandsMatch;
    }

    std::optional<bool> doExpressionsMatch(const syrec::ShiftExpression& lExpr, const syrec::ShiftExpression& rExpr) {
        const std::optional<bool> doShiftedExpressionsMatch = doExpressionsMatch(lExpr.lhs, rExpr.lhs);
        if (doShiftedExpressionsMatch.value_or(false)) {
            if (lExpr.shiftOperation != rExpr.shiftOperation)
                return false;
            return doExpressionComponentsMatch(lExpr.rhs, rExpr.rhs);
        }
        return doShiftedExpressionsMatch;
    }

    std::optional<bool> doExpressionsMatch(const syrec::NumericExpression& lExpr, const syrec::NumericExpression& rExpr) {
        return doExpressionComponentsMatch(lExpr.value, rExpr.value);
    }

    std::optional<bool> doExpressionsMatch(const syrec::VariableExpression& lExpr, const syrec::VariableExpression& rExpr) {
        return doExpressionComponentsMatch(lExpr.var, rExpr.var);
    }

    std::optional<bool> doExpressionsMatch(const syrec::Expression::ptr& lExpr, const syrec::Expression::ptr& rExpr) {
        if (lExpr == nullptr ^ rExpr == nullptr)
            return false;
        if (!lExpr)
            return std::nullopt;

        bool matcherForExprTypeDefined = true;
        if (const auto& lExprAsBinaryOne = dynamic_cast<const syrec::BinaryExpression*>(&*lExpr)) {
            if (const auto& rExprAsBinaryOne = dynamic_cast<const syrec::BinaryExpression*>(&*rExpr); lExprAsBinaryOne && rExprAsBinaryOne) {
                return doExpressionsMatch(*lExprAsBinaryOne, *rExprAsBinaryOne);
            }
        } else if (const auto& lExprAsShiftOne = dynamic_cast<const syrec::ShiftExpression*>(&*lExpr)) {
            if (const auto& rExprAsShiftOne = dynamic_cast<const syrec::ShiftExpression*>(&*rExpr); lExprAsShiftOne && rExprAsShiftOne) {
                return doExpressionsMatch(*lExprAsShiftOne, *rExprAsShiftOne);
            }
        } else if (const auto& lExprAsVarExpr = dynamic_cast<const syrec::VariableExpression*>(&*lExpr)) {
            if (const auto& rExprAsVarExpr = dynamic_cast<const syrec::VariableExpression*>(&*rExpr); lExprAsVarExpr && rExprAsVarExpr) {
                return doExpressionsMatch(*lExprAsVarExpr, *rExprAsVarExpr);
            }
        } else if (const auto& lExprAsNumericOne = dynamic_cast<const syrec::NumericExpression*>(&*lExpr)) {
            if (const auto& rExprAsNumericOne = dynamic_cast<const syrec::NumericExpression*>(&*rExpr); lExprAsNumericOne && rExprAsNumericOne) {
                return doExpressionsMatch(*lExprAsNumericOne, *rExprAsNumericOne);
            }
        } else {
            matcherForExprTypeDefined = false;
        }
        return !matcherForExprTypeDefined ? std::nullopt : std::make_optional(false);
    }
} // namespace

std::optional<syrec::Statement::vec> CustomStatementVisitor::visitStatementListTyped(const TSyrecParser::StatementListContext* ctx) {
    if (!ctx)
        return std::nullopt;

    syrec::Statement::vec statements;
    statements.reserve(ctx->stmts.size());

    for (const auto& antlrStatementContext: ctx->stmts)
        if (const std::optional<syrec::Statement::ptr> generatedStatement = visitStatementTyped(antlrStatementContext); generatedStatement.has_value() && generatedStatement.value())
            statements.emplace_back(*generatedStatement);

    return statements;
}

std::optional<syrec::Statement::ptr> CustomStatementVisitor::visitStatementTyped(TSyrecParser::StatementContext* ctx) {
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
    // We should not have to report an error at this position since the tokenizer should already report an error if the currently processed token is
    // not in the union of the FIRST sets of the potential alternatives.
    return std::nullopt;
}

std::optional<syrec::Statement::ptr> CustomStatementVisitor::visitAssignStatementTyped(TSyrecParser::AssignStatementContext* ctx) const {
    if (!ctx)
        return std::nullopt;

    const std::optional<syrec::VariableAccess::ptr> assignmentLhsOperand = expressionVisitorInstance->visitSignalTyped(ctx->signal());
    if (assignmentLhsOperand.has_value()) {
        recordErrorIfAssignmentToReadonlyVariableIsPerformed(*assignmentLhsOperand->get()->var, *ctx->signal()->start);
        expressionVisitorInstance->setRestrictionOnVariableAccesses(*assignmentLhsOperand);
        if (const auto& accessedBitrangeOfLhsOperand = assignmentLhsOperand.value()->var ? tryDetermineAccessedBitrangeOfVariableAccess(**assignmentLhsOperand) : std::nullopt; accessedBitrangeOfLhsOperand.has_value())
            expressionVisitorInstance->setExpectedBitwidthForAnyProcessedEntity(getLengthOfAccessedBitrange(*accessedBitrangeOfLhsOperand));
    }
    const std::optional<syrec::AssignStatement::AssignOperation> assignmentOperation  = ctx->assignmentOp ? deserializeAssignmentOperationFromString(ctx->assignmentOp->getText()) : std::nullopt;
    const std::optional<syrec::Expression::ptr>                  assignmentRhsOperand = expressionVisitorInstance->visitExpressionTyped(ctx->expression());
    expressionVisitorInstance->clearRestrictionOnVariableAccesses();
    expressionVisitorInstance->clearExpectedBitwidthForAnyProcessedEntity();

    return assignmentLhsOperand.has_value() && assignmentOperation.has_value() && assignmentRhsOperand.has_value() ? std::optional(std::make_shared<syrec::AssignStatement>(*assignmentLhsOperand, *assignmentOperation, *assignmentRhsOperand)) : std::nullopt;
}

std::optional<syrec::Statement::ptr> CustomStatementVisitor::visitUnaryStatementTyped(TSyrecParser::UnaryStatementContext* ctx) const {
    if (!ctx)
        return std::nullopt;

    const std::optional<syrec::VariableAccess::ptr> assignedToVariable = expressionVisitorInstance->visitSignalTyped(ctx->signal());
    if (assignedToVariable.has_value())
        recordErrorIfAssignmentToReadonlyVariableIsPerformed(*assignedToVariable->get()->var, *ctx->signal()->start);

    expressionVisitorInstance->clearRestrictionOnVariableAccesses();
    expressionVisitorInstance->clearExpectedBitwidthForAnyProcessedEntity();

    const std::optional<syrec::UnaryStatement::UnaryOperation> assignmentOperation = deserializeUnaryAssignmentOperationFromString(ctx->unaryOp->getText());
    return assignedToVariable.has_value() && assignmentOperation.has_value() ? std::make_optional(std::make_shared<syrec::UnaryStatement>(*assignmentOperation, *assignedToVariable)) : std::nullopt;
}

std::optional<syrec::Statement::ptr> CustomStatementVisitor::visitSwapStatementTyped(const TSyrecParser::SwapStatementContext* ctx) const {
    if (!ctx)
        return std::nullopt;

    const std::optional<syrec::VariableAccess::ptr> swapLhsOperand = expressionVisitorInstance->visitSignalTyped(ctx->lhsOperand);
    if (swapLhsOperand.has_value()) {
        recordErrorIfAssignmentToReadonlyVariableIsPerformed(*swapLhsOperand->get()->var, *ctx->lhsOperand->getStart());
        expressionVisitorInstance->setRestrictionOnVariableAccesses(*swapLhsOperand);
        if (const auto& accessedBitrangeOfLhsOperand = swapLhsOperand.value()->var ? tryDetermineAccessedBitrangeOfVariableAccess(**swapLhsOperand) : std::nullopt; accessedBitrangeOfLhsOperand.has_value())
            expressionVisitorInstance->setExpectedBitwidthForAnyProcessedEntity(getLengthOfAccessedBitrange(*accessedBitrangeOfLhsOperand));
    }
    const std::optional<syrec::VariableAccess::ptr> swapRhsOperand = expressionVisitorInstance->visitSignalTyped(ctx->rhsOperand);
    if (swapRhsOperand.has_value())
        recordErrorIfAssignmentToReadonlyVariableIsPerformed(*swapRhsOperand->get()->var, *ctx->rhsOperand->getStart());

    expressionVisitorInstance->clearRestrictionOnVariableAccesses();
    expressionVisitorInstance->clearExpectedBitwidthForAnyProcessedEntity();

    return swapLhsOperand.has_value() && swapRhsOperand.has_value() ? std::make_optional(std::make_shared<syrec::SwapStatement>(*swapLhsOperand, *swapRhsOperand)) : std::nullopt;
}

std::optional<syrec::Statement::ptr> CustomStatementVisitor::visitSkipStatementTyped(TSyrecParser::SkipStatementContext*) {
    return std::make_shared<syrec::SkipStatement>();
}

std::optional<syrec::Statement::ptr> CustomStatementVisitor::visitCallStatementTyped(TSyrecParser::CallStatementContext* ctx) {
    if (!ctx)
        return std::nullopt;

    const std::optional<std::string>                        calledModuleIdentifier = ctx->moduleIdent ? std::optional(ctx->moduleIdent->getText()) : std::nullopt;
    const std::optional<utils::TemporaryVariableScope::ptr> activeSymbolTableScope = symbolTable->getActiveTemporaryScope();
    // Should rename callee arguments to caller arguments in grammar
    syrec::Variable::vec symbolTableEntryPerCallerArgument;
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
                symbolTableEntryPerCallerArgument.emplace_back(std::make_shared<syrec::Variable>(**matchingSymbolTableEntryForCallerArgument->get()->getReadonlyVariableData()));
        } else {
            recordSemanticError<SemanticError::NoVariableMatchingIdentifier>(mapTokenPositionToMessagePosition(*antlrCallerArgumentToken), antlrCallerArgumentToken->getText());
        }
    }

    if (!calledModuleIdentifier.has_value())
        return std::nullopt;

    NotOverloadResolutedCallStatementScope*                                             activeModuleScopeRecordingCallStatements = getActiveModuleScopeRecordingCallStatements();
    std::optional<NotOverloadResolutedCallStatementScope::CallStatementInstanceVariant> callStatementInstanceVariant;
    if (ctx->OP_CALL())
        callStatementInstanceVariant = std::make_shared<syrec::CallStatement>(nullptr, callerArgumentVariableIdentifiers);
    else if (ctx->OP_UNCALL())
        callStatementInstanceVariant = std::make_shared<syrec::UncallStatement>(nullptr, callerArgumentVariableIdentifiers);

    if (callStatementInstanceVariant.has_value()) {
        if (activeModuleScopeRecordingCallStatements)
            activeModuleScopeRecordingCallStatements->callStatementsToPerformOverloadResolutionOn.emplace_back(*callStatementInstanceVariant, *calledModuleIdentifier, symbolTableEntryPerCallerArgument, ctx->moduleIdent->getLine(), ctx->moduleIdent->getCharPositionInLine());
        else
            recordCustomError(Message::Position(ctx->moduleIdent->getLine(), ctx->moduleIdent->getCharPositionInLine()), "Cannot record call statement variant due to no scope to record such statements is open! This is an internal error that should not happen");

        if (ctx->OP_CALL())
            return std::get<std::shared_ptr<syrec::CallStatement>>(*callStatementInstanceVariant);
        if (ctx->OP_UNCALL())
            return std::get<std::shared_ptr<syrec::UncallStatement>>(*callStatementInstanceVariant);
    }
    return std::nullopt;
}

std::optional<syrec::Statement::ptr> CustomStatementVisitor::visitIfStatementTyped(const TSyrecParser::IfStatementContext* ctx) {
    if (!ctx)
        return std::nullopt;

    auto generatedIfStatement = std::make_shared<syrec::IfStatement>();
    generatedIfStatement->setCondition(expressionVisitorInstance->visitExpressionTyped(ctx->guardCondition).value_or(nullptr));
    expressionVisitorInstance->clearExpectedBitwidthForAnyProcessedEntity();

    generatedIfStatement->thenStatements = visitStatementListTyped(ctx->trueBranchStmts).value_or(syrec::Statement::vec());
    generatedIfStatement->elseStatements = visitStatementListTyped(ctx->falseBranchStmts).value_or(syrec::Statement::vec());

    generatedIfStatement->setFiCondition(expressionVisitorInstance->visitExpressionTyped(ctx->matchingGuardExpression).value_or(nullptr));
    expressionVisitorInstance->clearExpectedBitwidthForAnyProcessedEntity();

    if (const std::optional<bool> doConditionsMatch = generatedIfStatement->condition && generatedIfStatement->fiCondition ? doExpressionsMatch(generatedIfStatement->condition, generatedIfStatement->fiCondition) : std::nullopt; doConditionsMatch.has_value() && !*doConditionsMatch)
        recordSemanticError<SemanticError::IfGuardExpressionMissmatch>(mapTokenPositionToMessagePosition(*ctx->guardCondition->getStart()));

    // TODO: Similarily to the processing of a for statement we need to determine how the parser processes empty statements list provided by the user (will this visitor function be called or not)?
    return generatedIfStatement;
}

std::optional<syrec::Statement::ptr> CustomStatementVisitor::visitForStatementTyped(TSyrecParser::ForStatementContext* ctx) {
    if (!ctx)
        return std::nullopt;

    const std::optional<utils::TemporaryVariableScope::ptr> activeSymbolTableScope = symbolTable->getActiveTemporaryScope();
    const std::optional<std::string>                        loopVariableIdentifier = visitLoopVariableDefinitionTyped(ctx->loopVariableDefinition());
    if (loopVariableIdentifier.has_value()) {
        if (activeSymbolTableScope.has_value())
            activeSymbolTableScope->get()->recordLoopVariable(std::make_shared<syrec::Number>(*loopVariableIdentifier));
        expressionVisitorInstance->setRestrictionOnLoopVariablesUsableInFutureLoopVariableValueInitializations(*loopVariableIdentifier);
    }

    const std::optional<syrec::Number::ptr> iterationRangeStartValue = expressionVisitorInstance->visitNumberTyped(ctx->startValue);
    expressionVisitorInstance->clearRestrictionOnLoopVariablesUsableInFutureLoopVariableValueInitializations();

    const std::optional<syrec::Number::ptr> iterationRangeEndValue      = expressionVisitorInstance->visitNumberTyped(ctx->endValue);
    const std::optional<syrec::Number::ptr> iterationRangeStepSizeValue = visitLoopStepsizeDefinitionTyped(ctx->loopStepsizeDefinition());

    auto generatedForStatement          = std::make_shared<syrec::ForStatement>();
    generatedForStatement->loopVariable = loopVariableIdentifier.value_or("");

    if (iterationRangeStartValue.has_value())
        generatedForStatement->range = ctx->endValue != nullptr ? std::make_pair(*iterationRangeStartValue, iterationRangeEndValue.value_or(nullptr)) : std::make_pair(*iterationRangeStartValue, *iterationRangeStartValue);
    else if (iterationRangeEndValue.has_value())
        generatedForStatement->range = std::make_pair(std::make_shared<syrec::Number>(0), *iterationRangeEndValue);

    generatedForStatement->step = iterationRangeStepSizeValue.value_or(std::make_shared<syrec::Number>(1));

    if (const std::optional<unsigned int> evaluatedValueOfStepsize = iterationRangeStepSizeValue.has_value() ? iterationRangeStepSizeValue.value()->tryEvaluate({}) : std::nullopt; 
        ctx->endValue && evaluatedValueOfStepsize.has_value() && !*evaluatedValueOfStepsize 
        && generatedForStatement->range.first && generatedForStatement->range.second) {
        const std::optional<unsigned int> evaluatedValueOfIterationRangeStart = generatedForStatement->range.first->tryEvaluate({});
        const std::optional<unsigned int> evaluatedValueOfIterationRangeEnd   = evaluatedValueOfIterationRangeStart.has_value() ? generatedForStatement->range.second->tryEvaluate({}) : std::nullopt;
        if (evaluatedValueOfIterationRangeStart.has_value() && evaluatedValueOfIterationRangeEnd.has_value() && *evaluatedValueOfIterationRangeStart != *evaluatedValueOfIterationRangeEnd)
            recordSemanticError<SemanticError::InfiniteLoopDetected>(
                Message::Position(mapTokenPositionToMessagePosition(*ctx->KEYWORD_FOR()->getSymbol())), 
                *evaluatedValueOfIterationRangeStart, *evaluatedValueOfIterationRangeEnd, *evaluatedValueOfStepsize);
    }

    // TODO: Does the parser prevent the execution of this function for loops with an empty statement body (due to the statement list production being required to consist of at least one statement) or do we need to check the validity of the statement body manually?
    // TODO: Make value range of loop variable available for index checks in expressions
    if (ctx->statementList())
        generatedForStatement->statements = visitStatementListTyped(ctx->statementList()).value_or(syrec::Statement::vec());

    if (loopVariableIdentifier.has_value() && activeSymbolTableScope.has_value())
        activeSymbolTableScope->get()->removeVariable(*loopVariableIdentifier);

    return generatedForStatement->step && generatedForStatement->range.first && generatedForStatement->range.second ? std::make_optional(generatedForStatement) : std::nullopt;
}

std::vector<CustomStatementVisitor::NotOverloadResolutedCallStatementScope> CustomStatementVisitor::getCallStatementsWithNotPerformedOverloadResolution() const {
    return callStatementsWithNotPerformedOverloadResolutionScopes;
}

void CustomStatementVisitor::openNewScopeToRecordCallStatementsInModule(const NotOverloadResolutedCallStatementScope::DeclaredModuleSignature& enclosingModuleSignature) {
    callStatementsWithNotPerformedOverloadResolutionScopes.emplace_back(enclosingModuleSignature);
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

std::optional<syrec::Number::ptr> CustomStatementVisitor::visitLoopStepsizeDefinitionTyped(TSyrecParser::LoopStepsizeDefinitionContext* ctx) const {
    if (!ctx)
        return std::nullopt;

    std::optional<syrec::Number::ptr> userDefinedStepsizeValue = expressionVisitorInstance->visitNumberTyped(ctx->number());
    if (!userDefinedStepsizeValue.has_value() || !userDefinedStepsizeValue.value())
        return std::nullopt;

    if (ctx->OP_MINUS()) {
        if (const std::optional<unsigned int> evaluatedValueForStepsize = userDefinedStepsizeValue.value()->tryEvaluate({}); evaluatedValueForStepsize.has_value())
            return std::make_shared<syrec::Number>(-*evaluatedValueForStepsize);

        // Since we cannot store an 'expression' of the form -(<Number>) in the IR representation, a constant expression (0 - <Number>) is used instead.
        return std::make_shared<syrec::Number>(syrec::Number::ConstantExpression(
                std::make_shared<syrec::Number>(0),
                syrec::Number::ConstantExpression::Operation::Subtraction,
                *userDefinedStepsizeValue));
    }
    return userDefinedStepsizeValue;
}

void CustomStatementVisitor::recordErrorIfAssignmentToReadonlyVariableIsPerformed(const syrec::Variable& accessedVariable, const antlr4::Token& reportedErrorPosition) const {
    if (!doesVariableTypeAllowAssignment(accessedVariable.type))
        recordSemanticError<SemanticError::AssignmentToReadonlyVariable>(mapTokenPositionToMessagePosition(reportedErrorPosition), accessedVariable.name);
}

CustomStatementVisitor::NotOverloadResolutedCallStatementScope* CustomStatementVisitor::getActiveModuleScopeRecordingCallStatements() {
    if (callStatementsWithNotPerformedOverloadResolutionScopes.empty())
        return nullptr;

    return &callStatementsWithNotPerformedOverloadResolutionScopes.back();
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