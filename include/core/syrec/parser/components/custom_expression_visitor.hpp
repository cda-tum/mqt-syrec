#ifndef CORE_SYREC_PARSER_COMPONENTS_CUSTOM_EXPRESSION_VISITOR_HPP
#define CORE_SYREC_PARSER_COMPONENTS_CUSTOM_EXPRESSION_VISITOR_HPP
#pragma once

#include "TSyrecParser.h"
#include "core/syrec/parser/utils/if_statement_expression_components_recorder.hpp"

#include <core/syrec/expression.hpp>
#include <core/syrec/parser/components/custom_base_visitor.hpp>

namespace syrecParser {
    class CustomExpressionVisitor: protected CustomBaseVisitor {
    public:
        CustomExpressionVisitor(const std::shared_ptr<ParserMessagesContainer>& sharedMessagesContainerInstance, const std::shared_ptr<utils::BaseSymbolTable>& sharedSymbolTableInstance, utils::IntegerConstantTruncationOperation integerConstantTruncationOperation):
            CustomBaseVisitor(sharedMessagesContainerInstance, sharedSymbolTableInstance, integerConstantTruncationOperation) {}

        [[nodiscard]] std::optional<syrec::Expression::ptr> visitExpressionTyped(TSyrecParser::ExpressionContext* context);
        [[nodiscard]] std::optional<syrec::Expression::ptr> visitExpressionFromNumberTyped(TSyrecParser::ExpressionFromNumberContext* context) const;
        [[nodiscard]] std::optional<syrec::Expression::ptr> visitExpressionFromSignalTyped(TSyrecParser::ExpressionFromSignalContext* context);
        [[nodiscard]] std::optional<syrec::Expression::ptr> visitBinaryExpressionTyped(const TSyrecParser::BinaryExpressionContext* context);
        [[nodiscard]] std::optional<syrec::Expression::ptr> visitUnaryExpressionTyped(const TSyrecParser::UnaryExpressionContext* context) const;
        [[nodiscard]] std::optional<syrec::Expression::ptr> visitShiftExpressionTyped(TSyrecParser::ShiftExpressionContext* context);

        [[nodiscard]] std::optional<syrec::VariableAccess::ptr> visitSignalTyped(TSyrecParser::SignalContext* context);
        [[nodiscard]] std::optional<syrec::Number::ptr>         visitNumberTyped(TSyrecParser::NumberContext* context) const;
        [[nodiscard]] std::optional<syrec::Number::ptr>         visitNumberFromConstantTyped(TSyrecParser::NumberFromConstantContext* context) const;
        [[nodiscard]] std::optional<syrec::Number::ptr>         visitNumberFromSignalwidthTyped(TSyrecParser::NumberFromSignalwidthContext* context) const;
        [[nodiscard]] std::optional<syrec::Number::ptr>         visitNumberFromExpressionTyped(const TSyrecParser::NumberFromExpressionContext* context) const;
        [[nodiscard]] std::optional<syrec::Number::ptr>         visitNumberFromLoopVariableTyped(TSyrecParser::NumberFromLoopVariableContext* context) const;

        void setExpectedBitwidthForAnyProcessedEntity(unsigned int bitwidth);
        void clearExpectedBitwidthForAnyProcessedEntity();

        [[maybe_unused]] bool setRestrictionOnVariableAccesses(const syrec::VariableAccess::ptr& notAccessiblePartsForFutureVariableAccesses);
        void                  clearRestrictionOnVariableAccesses();

        void setRestrictionOnLoopVariablesUsableInFutureLoopVariableValueInitializations(const std::string_view& loopVariableIdentifier);
        void clearRestrictionOnLoopVariablesUsableInFutureLoopVariableValueInitializations();

        void setIfStatementExpressionComponentsRecorder(const utils::IfStatementExpressionComponentsRecorder::ptr& ifStatementExpressionComponentsRecorder);
        void clearIfStatementExpressionComponentsRecorder();

        [[nodiscard]] std::optional<unsigned int> getCurrentExpectedBitwidthForAnyProcessedEntity() const;
        static void                               truncateConstantValuesInAnyBinaryExpression(syrec::Expression& expression, unsigned int expectedBitwidthOfOperandsInExpression, utils::IntegerConstantTruncationOperation integerConstantTruncationOperation);

    protected:
        std::optional<unsigned int>                                                    optionalExpectedBitwidthForAnyProcessedEntity;
        std::optional<syrec::VariableAccess::ptr>                                      optionalRestrictionOnVariableAccesses;
        std::optional<std::string_view>                                                optionalRestrictionOnLoopVariableUsageInLoopVariableValueInitialization;
        std::optional<utils::IfStatementExpressionComponentsRecorder::ptr>             optionalIfStatementExpressionComponentsRecorder;

        void recordExpressionComponent(const utils::IfStatementExpressionComponentsRecorder::ExpressionComponent& expressionComponent) const;

        [[nodiscard]] static std::optional<syrec::BinaryExpression::BinaryOperation>         deserializeBinaryOperationFromString(const std::string_view& stringifiedOperation);
        [[nodiscard]] static std::optional<syrec::ShiftExpression::ShiftOperation>           deserializeShiftOperationFromString(const std::string_view& stringifiedOperation);
        [[nodiscard]] static std::optional<syrec::Number::ConstantExpression::Operation>     deserializeConstantExpressionOperationFromString(const std::string_view& stringifiedOperation);
        [[nodiscard]] static std::optional<syrec::Expression::ptr>                           trySimplifyBinaryExpressionWithConstantValueOfOneOperandKnown(unsigned int knownOperandValue, syrec::BinaryExpression::BinaryOperation binaryOperation, const syrec::Expression::ptr& unknownOperandValue, bool isValueOfLhsOperandKnown);
    };
} // namespace syrecParser
#endif