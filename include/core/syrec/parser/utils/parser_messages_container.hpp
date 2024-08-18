#ifndef PARSER_MESSAGES_CONTAINER_HPP
#define PARSER_MESSAGES_CONTAINER_HPP

#include <cstddef>
#include <memory>
#include <vector>
#include <string>

namespace syrecParser {
    struct Message {
        virtual ~Message() = default;

        struct Position {
            std::size_t line;
            std::size_t column;
        
        };

        enum class MessageType {
            Error,
            Information,
            Warning
        };
    
        MessageType mType;
        Position    position;
        std::string content;

        Message(const MessageType mType, const Position position, std::string content):
            mType(mType), position(position), content(std::move(content)) {}

        [[nodiscard]] virtual std::string stringify() const {
            return "-- line " + std::to_string(position.line) + " col " + std::to_string(position.column) + ": " + content;
        }
    };

    class ParserMessagesContainer {
    public:
        using ptr = std::shared_ptr<ParserMessagesContainer>;

        ParserMessagesContainer(const bool recordAtMostOneError)
            : recordAtMostOneError(recordAtMostOneError) {}

        void                               recordMessage(Message&& message);
        [[nodiscard]] std::vector<Message> getMessagesOfType(Message::MessageType messageType) const;

    protected:
        std::vector<Message> errorMessages;
        bool                 recordAtMostOneError;
    };
} // namespace syrecParser
#endif