#ifndef CORE_SYREC_PARSER_COMPONENTS_CUSTOM_NUMBER_AND_SIGNAL_VISITOR_HPP
#define CORE_SYREC_PARSER_COMPONENTS_CUSTOM_NUMBER_AND_SIGNAL_VISITOR_HPP
#pragma once

#include <core/syrec/parser/components/custom_base_visitor.hpp>

namespace syrecParser {
    class CustomNumberAndSignalVisitor: protected CustomBaseVisitor {
    public:
        CustomNumberAndSignalVisitor(std::shared_ptr<ParserMessagesContainer> sharedMessagesContainerInstance):
            CustomBaseVisitor(std::move(sharedMessagesContainerInstance)) {}

    protected:
        std::any visitSignal(TSyrecParser::SignalContext* context) override;
        std::any visitNumberFromConstant(TSyrecParser::NumberFromConstantContext* context) override;
        std::any visitNumberFromSignalwidth(TSyrecParser::NumberFromSignalwidthContext* context) override;
        std::any visitNumberFromExpression(TSyrecParser::NumberFromExpressionContext* context) override;
        std::any visitNumberFromLoopVariable(TSyrecParser::NumberFromLoopVariableContext* context) override;

    };
} // namespace syrecParser
#endif