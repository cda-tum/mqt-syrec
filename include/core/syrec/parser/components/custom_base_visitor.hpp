#ifndef CORE_SYREC_COMPONENTS_CUSTOM_BASE_VISITOR_HPP
#define CORE_SYREC_COMPONENTS_CUSTOM_BASE_VISITOR_HPP

#include "antlr4-runtime.h"
#include "TSyrecParserBaseVisitor.h"

#include <core/syrec/parser/utils/parser_messages_container.hpp>
#include <fmt/core.h>

namespace syrecParser {
    class CustomBaseVisitor: protected TSyrecParserBaseVisitor {
    public:
        CustomBaseVisitor(std::shared_ptr<ParserMessagesContainer> sharedGeneratedMessageContainerInstance):
            sharedGeneratedMessageContainerInstance(std::move(sharedGeneratedMessageContainerInstance)) {}

    protected:
        std::shared_ptr<ParserMessagesContainer> sharedGeneratedMessageContainerInstance;

        template<typename... T>
        void recordMessage(Message::MessageType messageType, Message::Position messagePosition, const std::string& messageFormatString, T&&... args) {
            if (!sharedGeneratedMessageContainerInstance)
                return;

            sharedGeneratedMessageContainerInstance->recordMessage(Message({messageType, messagePosition, fmt::format(FMT_STRING(messageFormatString), std::forward<T>(args)...)}));
        }
    };
}

#endif