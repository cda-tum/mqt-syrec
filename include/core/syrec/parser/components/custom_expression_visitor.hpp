#ifndef CORE_SYREC_PARSER_COMPONENTS_CUSTOM_EXPRESSION_VISITOR_HPP
#define CORE_SYREC_PARSER_COMPONENTS_CUSTOM_EXPRESSION_VISITOR_HPP
#pragma once

#include <core/syrec/parser/components/custom_number_and_signal_visitor.hpp>

namespace syrecParser {
    class CustomExpressionVisitor: protected CustomBaseVisitor {
    public:
        CustomExpressionVisitor(std::shared_ptr<ParserMessagesContainer> sharedMessagesContainerInstance):
            CustomBaseVisitor(sharedMessagesContainerInstance), numberAndSignalVisitorInstance(std::make_unique<CustomNumberAndSignalVisitor>(sharedMessagesContainerInstance)) {}

    protected:
        std::unique_ptr<CustomNumberAndSignalVisitor> numberAndSignalVisitorInstance;

        std::any visitExpressionFromNumber(TSyrecParser::ExpressionFromNumberContext* context) override;
        std::any visitExpressionFromSignal(TSyrecParser::ExpressionFromSignalContext* context) override;
        std::any visitBinaryExpression(TSyrecParser::BinaryExpressionContext* context) override;
        std::any visitUnaryExpression(TSyrecParser::UnaryExpressionContext* context) override;
        std::any visitShiftExpression(TSyrecParser::ShiftExpressionContext* context) override;
    };
} // namespace syrecParser
#endif