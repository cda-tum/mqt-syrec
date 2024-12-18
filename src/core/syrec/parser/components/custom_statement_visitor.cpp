#include "core/syrec/parser/components/custom_statement_visitor.hpp"

using namespace syrecParser;

// START OF NON-PUBLIC FUNCTIONALITY
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