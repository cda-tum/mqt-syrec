#ifndef CUSTOM_ERROR_LISTENER_HPP
#define CUSTOM_ERROR_LISTENER_HPP

#include "BaseErrorListener.h"
#include "core/syrec/parser/utils/parser_messages_container.hpp"

namespace syrecParser {
    class CustomErrorListener: public antlr4::BaseErrorListener {
    public:
        CustomErrorListener(std::shared_ptr<ParserMessagesContainer> parserMessagesGenerator)
            : parserMessagesGenerator(std::move(parserMessagesGenerator)) {}

        void syntaxError(antlr4::Recognizer* /*recognizer*/, antlr4::Token* /*offendingSymbol*/, size_t line,
                         size_t charPositionInLine, const std::string& msg, std::exception_ptr /*e*/) override;

        protected:
            std::shared_ptr<ParserMessagesContainer> parserMessagesGenerator;
    };
} // namespace parser

#endif