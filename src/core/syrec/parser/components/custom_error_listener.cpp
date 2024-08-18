#include "core/syrec/parser/components/custom_error_listener.hpp"

using namespace syrecParser;

void CustomErrorListener::syntaxError(antlr4::Recognizer*, antlr4::Token*, size_t line, size_t charPositionInLine, const std::string& msg, std::exception_ptr) {
    if (!parserMessagesGenerator)
        return;

    parserMessagesGenerator->recordMessage(Message({Message::MessageType::Error, {line, charPositionInLine}, msg}));
}