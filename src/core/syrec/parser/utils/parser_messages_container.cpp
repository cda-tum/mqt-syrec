#include "core/syrec/parser/utils/parser_messages_container.hpp"

using namespace syrecParser;

void ParserMessagesContainer::recordMessage(std::unique_ptr<Message> message) {
    if (message->type != Message::Type::Error)
        return;

    messagesPerType[message->type].emplace_back(std::move(message));
}

std::vector<Message::ptr> ParserMessagesContainer::getMessagesOfType(Message::Type messageType) const {
    if (!messagesPerType.count(messageType))
        return {};

    return messagesPerType.at(messageType);
}