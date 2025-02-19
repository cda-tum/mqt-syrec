#include "core/syrec/parser/utils/parser_messages_container.hpp"

#include <memory>
#include <string>
#include <vector>
#include <utility>

using namespace syrec_parser;

void ParserMessagesContainer::recordMessage(std::unique_ptr<Message> message) {
    if (message->type != Message::Type::Error || (temporaryFilterForToBeRecordedMessages.has_value() && temporaryFilterForToBeRecordedMessages.value() != message->id)) {
        return;
    }
    messagesPerType[message->type].emplace_back(std::move(message));
}

std::vector<Message::ptr> ParserMessagesContainer::getMessagesOfType(Message::Type messageType) const {
    return messagesPerType.count(messageType) > 0 ? messagesPerType.at(messageType) : std::vector<Message::ptr>();
}

bool ParserMessagesContainer::setFilterForToBeRecordedMessages(const std::string& messageIdToPassFilter) {
    if (messageIdToPassFilter.empty()) {
        return false;
    }
    temporaryFilterForToBeRecordedMessages = messageIdToPassFilter;
    return true;
}

void ParserMessagesContainer::clearFilterForToBeRecordedMessages() {
    temporaryFilterForToBeRecordedMessages.reset();
}