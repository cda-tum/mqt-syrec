#ifndef PARSER_MESSAGES_CONTAINER_HPP
#define PARSER_MESSAGES_CONTAINER_HPP

#include <cstddef>
#include <memory>
#include <vector>
#include <string>

namespace syrecParser {
    enum class MessageType {
        Error,
        Information,
        Warning
    };

    struct Message {
        MessageType mType;
        std::size_t line;
        std::size_t column;
        std::string content;
    };

    class ParserMessagesContainer {
    public:
        using ptr = std::shared_ptr<ParserMessagesContainer>;

        ParserMessagesContainer(bool recordAtMostOneError)
            : recordAtMostOneError(recordAtMostOneError) {}

        void                               recordMessage(Message&& message);
        [[nodiscard]] std::vector<Message> getMessagesOfType(MessageType messageType) const;

    protected:
        std::vector<Message> errorMessages;
        bool                 recordAtMostOneError;
    };
} // namespace syrecParser
#endif