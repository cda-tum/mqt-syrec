#ifndef CORE_SYREC_COMPONENTS_CUSTOM_BASE_VISITOR_HPP
#define CORE_SYREC_COMPONENTS_CUSTOM_BASE_VISITOR_HPP
#pragma once

#include "core/syrec/expression.hpp"
#include "core/syrec/program.hpp"
#include "core/syrec/variable.hpp"
#include "core/syrec/parser/utils/custom_error_mesages.hpp"
#include "core/syrec/parser/utils/parser_messages_container.hpp"
#include "core/syrec/parser/utils/symbolTable/base_symbol_table.hpp"

#include "Token.h"

#include <fmt/format.h>
#include <cerrno>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace syrecParser {
    /**
     * The base class containing data structure and utility functions required in more specialized visitors.
     *
     * Note that this class does not derive from the TSyrecParserVisitor class, defining the potential visitor functions for the SyReC grammar, since we are providing
     * type-safe overload for the visitor functions instead of relying on complex conversion from the std::any type to the expected return type of the visitor function.
     * The problem with the std::any type is that the user must know the exact type that is stored in the value of the std::any to be able to access it. This std::any_cast<T>
     * operation does not support polymorphism and other convinent behaviour that one can use std::optional<T>. Additionally, we can avoid the dynamic dispatch mechanism to
     * determine the correct visitor function (while still requiring dynamic_cast cascades) which now requires for future extensions of the grammar that the developer correctly
     * defines the handling of these new types in the visitors (instead of relying on the dynamic dispatch mechanism determine which visitor function overload to call).
     *
     * An example:
     * struct Base { virtual std::string getName() = 0; };
     * struct Derived : Base { std::string getName() { return "Derived"; };
     *
     * std::any polymorphicReturn(int i) {
     *      if (i > 1)
     *          return std::make_shared<Base>();
     *      else if (!i)
     *          return std::make_shared<Derived>();
     *      return std::nullopt;
     * }
     *
     * void usePolymorphicReturn(){
     *      // Throws a std::bad_cast exception since the type stored in the std::any is std::shared_ptr<Derived>;
     *      const std::optional<std::shared_ptr<Base>> x = std::any_cast<std::shared_ptr<Base>>(polymorphicReturn(0);
     *      // Throws a std::bad_cast exception since the type stored in the std::any is std::shared_ptr<Derived>;
     *      const std::optional<std::shared_ptr<Derived>> x = std::any_cast<std::shared_ptr<Base>>(polymorphicReturn(0);
     * }
     */
    class CustomBaseVisitor {
    public:
        CustomBaseVisitor(const std::shared_ptr<ParserMessagesContainer>& sharedGeneratedMessageContainerInstance, const std::shared_ptr<utils::BaseSymbolTable>& sharedSymbolTableInstance, const syrec::ReadProgramSettings& parserConfiguration):
            sharedGeneratedMessageContainerInstance(sharedGeneratedMessageContainerInstance), symbolTable(sharedSymbolTableInstance), parserConfiguration(parserConfiguration) {}

    protected:
        static constexpr unsigned int             DEFAULT_EXPRESSION_BITWIDTH   = 32;
        static constexpr unsigned int             MAX_SUPPORTED_SIGNAL_BITWIDTH = 32;

        std::shared_ptr<ParserMessagesContainer>  sharedGeneratedMessageContainerInstance;
        std::shared_ptr<utils::BaseSymbolTable>   symbolTable;
        syrec::ReadProgramSettings                parserConfiguration;

        [[nodiscard]] static Message::Position mapTokenPositionToMessagePosition(const antlr4::Token& token) {
            return Message::Position(token.getLine(), token.getCharPositionInLine());
        }

        [[nodiscard]] static std::optional<unsigned int> deserializeConstantFromString(const std::string& stringifiedConstantValue, bool* didDeserializationFailDueToOverflow) {
            char* pointerToLastCharacterInString = nullptr;
            // Need to reset errno to not reuse already set values
            errno = 0;

            // Using this conversion method for any user provided constant value forces the maximum possible value of a constant that can be specified
            // by the user in a SyReC circuit to 2^32. Larger values are not truncated but reported as an error instead.
            const unsigned int constantValue = std::strtoul(stringifiedConstantValue.c_str(), &pointerToLastCharacterInString, 10);
            // Using these error conditions checks will detect strings of the form "0 " as not valid while " 0" is considered valid.
            if (didDeserializationFailDueToOverflow != nullptr && errno == ERANGE) {
                *didDeserializationFailDueToOverflow = true;
            }

            if (stringifiedConstantValue.c_str() == pointerToLastCharacterInString || errno == ERANGE || pointerToLastCharacterInString != nullptr) {
                return std::nullopt;
            }

            return constantValue;
        }

        [[nodiscard]] static std::optional<unsigned int> tryGetConstantValueOfExpression(const syrec::Expression& expression) {
            if (const auto& expressionAsNumericOne = dynamic_cast<const syrec::NumericExpression*>(&expression); expressionAsNumericOne != nullptr && expressionAsNumericOne->value && expressionAsNumericOne->value) {
                return expressionAsNumericOne->value->tryEvaluate({});
            }
            return std::nullopt;
        }

        [[nodiscard]] static std::optional<std::pair<unsigned int, unsigned int>> tryDetermineAccessedBitrangeOfVariableAccess(const syrec::VariableAccess& variableAccess) {
            if (!variableAccess.range.has_value()) {
                return std::make_pair(0, variableAccess.bitwidth() - 1);
            }

            if (!variableAccess.range->first || !variableAccess.range->second) {
                return std::nullopt;
            }

            const std::optional<unsigned int> accessedBitrangeStart = variableAccess.range->first->tryEvaluate({});
            const std::optional<unsigned int> accessedBitRangeEnd   = accessedBitrangeStart.has_value() ? variableAccess.range->second->tryEvaluate({}) : std::nullopt;
            if (!accessedBitRangeEnd.has_value()) {
                return std::nullopt;
            }

            return std::make_pair(*accessedBitrangeStart, *accessedBitRangeEnd);
        }

        [[nodiscard]] static unsigned int getLengthOfAccessedBitrange(const std::pair<unsigned int, unsigned int> accessedBitrange) {
            return (accessedBitrange.first > accessedBitrange.second
                ? accessedBitrange.first - accessedBitrange.second
                : accessedBitrange.second - accessedBitrange.first) + 1;
        }

        template<SemanticError semanticError, typename... T>
        void recordSemanticError(Message::Position messagePosition, T&&... args) const {
            if (!sharedGeneratedMessageContainerInstance) {
                return;
            }

            static_assert(!getFormatForSemanticErrorMessage<semanticError>().empty(), "No format for message of semantic error found!");
            static_assert(!getIdentifierForSemanticError<semanticError>().empty(), "No identifiers for semantic error found!");

            constexpr std::string_view identifierForSemanticError = getIdentifierForSemanticError<semanticError>();
            // TODO: How should runtime errors be handled?
            sharedGeneratedMessageContainerInstance->recordMessage(std::make_unique<Message>(Message::Type::Error, std::string(identifierForSemanticError), messagePosition, fmt::format(FMT_STRING(getFormatForSemanticErrorMessage<semanticError>()), std::forward<T>(args)...)));
        }

        template<SemanticError semanticError>
        void recordSemanticError(Message::Position messagePosition) const {
            if (!sharedGeneratedMessageContainerInstance) {
                return;
            }

            static_assert(!getFormatForSemanticErrorMessage<semanticError>().empty(), "No format for message of semantic error found!");
            static_assert(!getIdentifierForSemanticError<semanticError>().empty(), "No identifiers for semantic error found!");

            constexpr std::string_view identifierForSemanticError = getIdentifierForSemanticError<semanticError>();
            // TODO: How should runtime errors be handled?
            sharedGeneratedMessageContainerInstance->recordMessage(std::make_unique<Message>(Message::Type::Error, std::string(identifierForSemanticError), messagePosition,  std::string(getFormatForSemanticErrorMessage<semanticError>())));
        }

        void recordCustomError(Message::Position messagePosition, const std::string& errorMessage) const {
            sharedGeneratedMessageContainerInstance->recordMessage(std::make_unique<Message>(Message::Type::Error, "UNKNOWN", messagePosition, errorMessage));
        }
    };
} // namespace syrecParser
#endif