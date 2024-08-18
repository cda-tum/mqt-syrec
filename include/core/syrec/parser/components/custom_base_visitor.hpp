#ifndef CUSTOM_BASE_VISISTOR_HPP
#define CUSTOM_BASE_VISISTOR_HPP
#pragma once

#include "antlr4-runtime.h"
#include "TSyrecParserBaseVisitor.h"
#include <core/syrec/parser/utils/parser_messages_container.hpp>
#include <memory>
#include <fmt/core.h>

namespace syrecParser {
    class CustomBaseVisitor: TSyrecParserBaseVisitor {
    public:
        CustomBaseVisitor(std::shared_ptr<ParserMessagesContainer> generatedMessagesContainer):
            generatedMessagesContainer(std::move(generatedMessagesContainer)) {}

    protected:
        std::shared_ptr<ParserMessagesContainer> generatedMessagesContainer;

        std::any visitSignal(TSyrecParser::SignalContext* context) override;
        std::any visitNumberFromConstant(TSyrecParser::NumberFromConstantContext* context) override;
        std::any visitNumberFromSignalwidth(TSyrecParser::NumberFromSignalwidthContext* context) override;
        std::any visitNumberFromExpression(TSyrecParser::NumberFromExpressionContext* context) override;
        std::any visitNumberFromLoopVariable(TSyrecParser::NumberFromLoopVariableContext* context) override;

        template<typename ...T>
        void recordMessage(Message::MessageType messageType, Message::Position messagePosition, const std::string& messageFormatString, T&&... args) {
            if (!generatedMessagesContainer)
                return;

            generatedMessagesContainer->recordMessage(Message({messageType, messagePosition, fmt::format(FMT_STRING(messageFormatString), std::forward<T>(args)...)}));
        }
    };
} // namespace syrecParser
#endif