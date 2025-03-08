#ifndef CORE_SYREC_PARSER_UTILS_PARSER_MESSAGES_CONTAINER_HPP
#define CORE_SYREC_PARSER_UTILS_PARSER_MESSAGES_CONTAINER_HPP
#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace syrec_parser {
    struct Message {
        using ptr = std::shared_ptr<Message>;
        struct Position {
            std::size_t line;
            std::size_t column;

            explicit Position(std::size_t line, std::size_t column):
                line(line), column(column) {}
        };

        enum class Type {
            Error,
            Information,
            Warning
        };

        Type        type;
        std::string id;
        Position    position;
        std::string message;

        Message(Type type, std::string id, const Position position, std::string message):
            type(type), id(std::move(id)), position(position), message(std::move(message)) {}

        [[nodiscard]] std::string stringify() const {
            return "-- line " + std::to_string(position.line) + " col " + std::to_string(position.column) + ": " + message;
        }
    };

    class ParserMessagesContainer {
    public:
        ParserMessagesContainer() {
            messagesPerType.emplace(std::make_pair(Message::Type::Error, std::vector<Message::ptr>()));
            messagesPerType.emplace(std::make_pair(Message::Type::Information, std::vector<Message::ptr>()));
            messagesPerType.emplace(std::make_pair(Message::Type::Warning, std::vector<Message::ptr>()));
        }

        void                                    recordMessage(std::unique_ptr<Message> message);
        [[nodiscard]] std::vector<Message::ptr> getMessagesOfType(Message::Type messageType) const;
        [[maybe_unused]] bool                   setFilterForToBeRecordedMessages(const std::string& messageIdToPassFilter);
        void                                    clearFilterForToBeRecordedMessages();
        void                                    sortRecordedMessagesOfTypeInAscendingOrder(Message::Type messageType);

    protected:
        std::unordered_map<Message::Type, std::vector<Message::ptr>> messagesPerType;
        std::optional<std::string>                                   temporaryFilterForToBeRecordedMessages;
    };
} // namespace syrec_parser
#endif