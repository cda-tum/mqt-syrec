#ifndef CORE_SYREC_PARSER_COMPONENTS_CUSTOM_NUMBER_AND_SIGNAL_VISITOR_HPP
#define CORE_SYREC_PARSER_COMPONENTS_CUSTOM_NUMBER_AND_SIGNAL_VISITOR_HPP
#pragma once

#include <core/syrec/number.hpp>
#include <core/syrec/variable.hpp>
#include <core/syrec/parser/components/custom_base_visitor.hpp>

namespace syrecParser {
    class CustomNumberAndSignalVisitor: protected CustomBaseVisitor {
    public:
        CustomNumberAndSignalVisitor(std::shared_ptr<ParserMessagesContainer> sharedMessagesContainerInstance):
            CustomBaseVisitor(std::move(sharedMessagesContainerInstance)) {}

        [[nodiscard]] std::optional<syrec::Variable::ptr> visitSignalTyped(TSyrecParser::SignalContext* context);
        [[nodiscard]] std::optional<syrec::Number::ptr>   visitNumberFromConstantTyped(TSyrecParser::NumberFromConstantContext* context);
        [[nodiscard]] std::optional<syrec::Number::ptr>   visitNumberFromSignalwidthTyped(TSyrecParser::NumberFromSignalwidthContext* context);
        [[nodiscard]] std::optional<syrec::Number::ptr>   visitNumberFromExpressionTyped(TSyrecParser::NumberFromExpressionContext* context);
        [[nodiscard]] std::optional<syrec::Number::ptr>   visitNumberFromLoopVariableTyped(TSyrecParser::NumberFromLoopVariableContext* context);

    protected:
        std::any visitSignal(TSyrecParser::SignalContext* context) override;
        std::any visitNumberFromConstant(TSyrecParser::NumberFromConstantContext* context) override;
        std::any visitNumberFromSignalwidth(TSyrecParser::NumberFromSignalwidthContext* context) override;
        std::any visitNumberFromExpression(TSyrecParser::NumberFromExpressionContext* context) override;
        std::any visitNumberFromLoopVariable(TSyrecParser::NumberFromLoopVariableContext* context) override;

    };
} // namespace syrecParser
#endif