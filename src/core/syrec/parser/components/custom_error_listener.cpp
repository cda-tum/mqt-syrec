#include "core/syrec/parser/components/custom_error_listener.hpp"

using namespace syrecParser;

void CustomErrorListener::syntaxError(antlr4::Recognizer*, antlr4::Token*, std::size_t line, std::size_t charPositionInLine, const std::string& msg, std::exception_ptr) {
    if (!sharedMessagesContainerInstance)
        return;

    sharedMessagesContainerInstance->recordMessage(std::make_unique<Message>(Message::Type::Error, "SYNTAX", Message::Position(line, charPositionInLine), msg));
}