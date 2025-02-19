#include "core/syrec/parser/components/custom_error_listener.hpp"
#include "core/syrec/parser/utils/parser_messages_container.hpp"

#include "Recognizer.h"
#include "Token.h"

#include <cstddef>
#include <exception>
#include <memory>
#include <string>

using namespace syrec_parser;

void CustomErrorListener::syntaxError([[maybe_unused]] antlr4::Recognizer* recognizer, [[maybe_unused]] antlr4::Token* offendingSymbol, std::size_t line, std::size_t charPositionInLine, const std::string& msg, [[maybe_unused]] std::exception_ptr e) {
    if (!sharedMessagesContainerInstance) {
        return;
    }
    sharedMessagesContainerInstance->recordMessage(std::make_unique<Message>(Message::Type::Error, "SYNTAX", Message::Position(line, charPositionInLine), msg));
}