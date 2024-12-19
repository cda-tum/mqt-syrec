#include "core/syrec/parser/components/custom_statement_visitor.hpp"

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
std::optional<std::string> CustomStatementVisitor::visitLoopVariableDefinitionTyped(TSyrecParser::LoopVariableDefinitionContext* ctx) {
    return std::nullopt;
}

std::optional<CustomStatementVisitor::LoopStepsizeDefinition> CustomStatementVisitor::visitLoopStepsizeDefinitionTyped(TSyrecParser::LoopStepsizeDefinitionContext* ctx) {
    return std::nullopt;
}

std::any CustomStatementVisitor::visitStatementList(TSyrecParser::StatementListContext* ctx) {
    return 0;
}

std::any CustomStatementVisitor::visitStatement(TSyrecParser::StatementContext* ctx) {
    return 0;
}

std::any CustomStatementVisitor::visitAssignStatement(TSyrecParser::AssignStatementContext* ctx) {
    return 0;
}

std::any CustomStatementVisitor::visitUnaryStatement(TSyrecParser::UnaryStatementContext* ctx) {
    return 0;
}

std::any CustomStatementVisitor::visitSwapStatement(TSyrecParser::SwapStatementContext* ctx) {
    return 0;
}

std::any CustomStatementVisitor::visitSkipStatement(TSyrecParser::SkipStatementContext* ctx) {
    return 0;
}

std::any CustomStatementVisitor::visitCallStatement(TSyrecParser::CallStatementContext* ctx) {
    return 0;
}

std::any CustomStatementVisitor::visitIfStatement(TSyrecParser::IfStatementContext* ctx) {
    return 0;
}

std::any CustomStatementVisitor::visitLoopVariableDefinition(TSyrecParser::LoopVariableDefinitionContext* ctx) {
    return 0;
}

std::any CustomStatementVisitor::visitLoopStepsizeDefinition(TSyrecParser::LoopStepsizeDefinitionContext* ctx) {
    return 0;
}

std::any CustomStatementVisitor::visitForStatement(TSyrecParser::ForStatementContext* ctx) {
    return 0;
}