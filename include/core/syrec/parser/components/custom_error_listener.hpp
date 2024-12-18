#ifndef CORE_SYREC_PARSER_COMPONENTS_CUSTOM_ERROR_LISTENER_HPP
#define CORE_SYREC_PARSER_COMPONENTS_CUSTOM_ERROR_LISTENER_HPP

#include "BaseErrorListener.h"
#include <core/syrec/parser/utils/parser_messages_container.hpp>

namespace syrecParser {
    class CustomErrorListener: public antlr4::BaseErrorListener {
    public:
        CustomErrorListener(std::shared_ptr<ParserMessagesContainer> sharedMessagesContainerInstance):
            sharedMessagesContainerInstance(std::move(sharedMessagesContainerInstance)) {}

        void syntaxError(antlr4::Recognizer* /*recognizer*/, antlr4::Token* /*offendingSymbol*/, size_t line,
                         size_t charPositionInLine, const std::string& msg, std::exception_ptr /*e*/) override;

        protected:
        std::shared_ptr<ParserMessagesContainer> sharedMessagesContainerInstance;
    };
} // namespace parser

#endif