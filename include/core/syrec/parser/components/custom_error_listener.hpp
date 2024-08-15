#ifndef CUSTOM_ERROR_LISTENER_HPP
#define CUSTOM_ERROR_LISTENER_HPP

#include "BaseErrorListener.h"

namespace syrecParser {
    class CustomErrorListener: public antlr4::BaseErrorListener {
    public:
        void syntaxError(antlr4::Recognizer* /*recognizer*/, antlr4::Token* /*offendingSymbol*/, size_t line,
                         size_t charPositionInLine, const std::string& msg, std::exception_ptr /*e*/) override;
    };
} // namespace parser

#endif