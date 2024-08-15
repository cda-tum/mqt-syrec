#ifndef CUSTOM_BASE_VISISTOR_HPP
#define CUSTOM_BASE_VISISTOR_HPP
#pragma once

#include "antlr4-runtime.h"
#include "TSyrecParserBaseVisitor.h"
#include <core/syrec/parser/utils/parser_messages_container.hpp>

#include <memory>

namespace syrecParser {
    class CustomBaseVisitor: TSyrecParserBaseVisitor {
    public:
        CustomBaseVisitor(std::shared_ptr<ParserMessagesContainer> messageGenerator):
            messageGenerator(std::move(messageGenerator)) {}

    protected:
        std::shared_ptr<ParserMessagesContainer> messageGenerator;

        std::any visitSignal(TSyrecParser::SignalContext* context) override;
        std::any visitNumberFromConstant(TSyrecParser::NumberFromConstantContext* context) override;
        std::any visitNumberFromSignalwidth(TSyrecParser::NumberFromSignalwidthContext* context) override;
        std::any visitNumberFromExpression(TSyrecParser::NumberFromExpressionContext* context) override;
        std::any visitNumberFromLoopVariable(TSyrecParser::NumberFromLoopVariableContext* context) override;
    };
} // namespace syrecParser
#endif