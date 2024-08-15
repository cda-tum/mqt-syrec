#ifndef CUSTOM_EXPRESSION_VISITOR_HPP
#define CUSTOM_EXPRESSION_VISITOR_HPP
#pragma once

#include "custom_base_visitor.hpp"

namespace syrecParser {
    class CustomExpressionVisitor: CustomBaseVisitor {
        std::any visitExpressionFromNumber(TSyrecParser::ExpressionFromNumberContext* context) override;
        std::any visitExpressionFromSignal(TSyrecParser::ExpressionFromSignalContext* context) override;
        std::any visitBinaryExpression(TSyrecParser::BinaryExpressionContext* context) override;
        std::any visitUnaryExpression(TSyrecParser::UnaryExpressionContext* context) override;
        std::any visitShiftExpression(TSyrecParser::ShiftExpressionContext* context) override;
    };
} // namespace syrecParser
#endif