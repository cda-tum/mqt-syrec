#ifndef CORE_SYREC_COMPONENTS_CUSTOM_BASE_VISITOR_HPP
#define CORE_SYREC_COMPONENTS_CUSTOM_BASE_VISITOR_HPP
#pragma once

#include "antlr4-runtime.h"
#include "TSyrecParserBaseVisitor.h"

#include <core/syrec/parser/components/generic_visitor_result.hpp>
#include <core/syrec/parser/utils/parser_messages_container.hpp>
#include <fmt/core.h>

#include <optional>

namespace syrecParser {
    class CustomBaseVisitor: protected TSyrecParserBaseVisitor {
    public:
        CustomBaseVisitor(std::shared_ptr<ParserMessagesContainer> sharedGeneratedMessageContainerInstance):
            sharedGeneratedMessageContainerInstance(std::move(sharedGeneratedMessageContainerInstance)) {}

    protected:
        std::shared_ptr<ParserMessagesContainer> sharedGeneratedMessageContainerInstance;

        template<typename... T>
        void recordMessage(Message::MessageType messageType, Message::Position messagePosition, const std::string& messageFormatString, T&&... args) {
            if (!sharedGeneratedMessageContainerInstance)
                return;

            sharedGeneratedMessageContainerInstance->recordMessage(Message({messageType, messagePosition, fmt::format(FMT_STRING(messageFormatString), std::forward<T>(args)...)}));
        }

        template<typename ExpectedResultType>
        [[nodiscard]] std::optional<std::shared_ptr<ExpectedResultType>> visitNonTerminalSymbolWithSingleResult(antlr4::ParserRuleContext* ruleContext) {
            if (const GenericVisitorResult<ExpectedResultType>* genericProductionResult = std::any_cast<GenericVisitorResult<ExpectedResultType>*>(visit(ruleContext)); genericProductionResult)
                return genericProductionResult->getData();
            return std::nullopt;
        }

        template<typename ExpectedResultType>
        [[nodiscard]] std::optional<std::vector<std::shared_ptr<ExpectedResultType>>> visitNonTerminalSymbolWithManyResults(antlr4::ParserRuleContext* ruleContext) {
            const std::vector<GenericVisitorResult<ExpectedResultType>>* genericProductionResult = std::any_cast<std::vector<GenericVisitorResult<ExpectedResultType>>*>(visit(ruleContext));
            std::vector<std::shared_ptr<ExpectedResultType>>             aggregateContainer(visitorResults.size(), nullptr);

            bool                                                         aggregationSuccessful = true;
            for (std::size_t i = 0; aggregationSuccessful && i < visitorResults.size(); ++i) {
                aggregateContainer[i] = visitorResults.at(i);
                aggregationSuccessful &= visitorResults.at(i);
            }
            return aggregationSuccessful ? std::make_optional(aggregateContainer) : std::nullopt;
        }
    };
}

#endif