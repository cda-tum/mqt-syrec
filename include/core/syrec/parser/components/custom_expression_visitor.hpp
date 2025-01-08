#ifndef CORE_SYREC_PARSER_COMPONENTS_CUSTOM_EXPRESSION_VISITOR_HPP
#define CORE_SYREC_PARSER_COMPONENTS_CUSTOM_EXPRESSION_VISITOR_HPP
#pragma once

#include <core/syrec/expression.hpp>
#include <core/syrec/parser/components/custom_base_visitor.hpp>

namespace syrecParser {
    class CustomExpressionVisitor: protected CustomBaseVisitor {
    public:
        CustomExpressionVisitor(const std::shared_ptr<ParserMessagesContainer>& sharedMessagesContainerInstance):
            CustomBaseVisitor(sharedMessagesContainerInstance) {}

        [[nodiscard]] std::optional<syrec::Expression::ptr> visitExpressionFromNumberTyped(TSyrecParser::ExpressionFromNumberContext* context);
        [[nodiscard]] std::optional<syrec::Expression::ptr> visitExpressionFromSignalTyped(TSyrecParser::ExpressionFromSignalContext* context);
        [[nodiscard]] std::optional<syrec::Expression::ptr> visitBinaryExpressionTyped(TSyrecParser::BinaryExpressionContext* context);
        [[nodiscard]] std::optional<syrec::Expression::ptr> visitUnaryExpressionTyped(TSyrecParser::UnaryExpressionContext* context);
        [[nodiscard]] std::optional<syrec::Expression::ptr> visitShiftExpressionTyped(TSyrecParser::ShiftExpressionContext* context);

        [[nodiscard]] std::optional<syrec::Variable::ptr> visitSignalTyped(TSyrecParser::SignalContext* context);
        [[nodiscard]] std::optional<syrec::Number::ptr>   visitNumberFromConstantTyped(TSyrecParser::NumberFromConstantContext* context);
        [[nodiscard]] std::optional<syrec::Number::ptr>   visitNumberFromSignalwidthTyped(TSyrecParser::NumberFromSignalwidthContext* context);
        [[nodiscard]] std::optional<syrec::Number::ptr>   visitNumberFromExpressionTyped(TSyrecParser::NumberFromExpressionContext* context);
        [[nodiscard]] std::optional<syrec::Number::ptr>   visitNumberFromLoopVariableTyped(TSyrecParser::NumberFromLoopVariableContext* context);

    protected:
        std::any visitExpressionFromNumber(TSyrecParser::ExpressionFromNumberContext* context) override;
        std::any visitExpressionFromSignal(TSyrecParser::ExpressionFromSignalContext* context) override;
        std::any visitBinaryExpression(TSyrecParser::BinaryExpressionContext* context) override;
        std::any visitUnaryExpression(TSyrecParser::UnaryExpressionContext* context) override;
        std::any visitShiftExpression(TSyrecParser::ShiftExpressionContext* context) override;

        std::any visitSignal(TSyrecParser::SignalContext* context) override;
        std::any visitNumberFromConstant(TSyrecParser::NumberFromConstantContext* context) override;
        std::any visitNumberFromSignalwidth(TSyrecParser::NumberFromSignalwidthContext* context) override;
        std::any visitNumberFromExpression(TSyrecParser::NumberFromExpressionContext* context) override;
        std::any visitNumberFromLoopVariable(TSyrecParser::NumberFromLoopVariableContext* context) override;

        [[nodiscard]] static std::optional<unsigned int>                             deserializeConstantFromString(const std::string& stringifiedConstantValue);
        [[nodiscard]] static std::optional<syrec::BinaryExpression::BinaryOperation> deserializeBinaryOperationFromString(const std::string_view& stringifiedOperation);
        [[nodiscard]] static std::optional<syrec::ShiftExpression::ShiftOperation>   deserializeShiftOperationFromString(const std::string_view& stringifiedOperation);
    };
} // namespace syrecParser
#endif