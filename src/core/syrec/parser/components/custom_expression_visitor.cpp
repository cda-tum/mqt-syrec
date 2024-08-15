#include "core/syrec/parser/components/custom_expression_visitor.hpp"

using namespace syrecParser;

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
