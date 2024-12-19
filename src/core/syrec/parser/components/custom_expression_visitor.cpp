#include "core/syrec/parser/components/custom_expression_visitor.hpp"

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
