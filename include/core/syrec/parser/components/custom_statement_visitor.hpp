#ifndef CORE_SYREC_PARSER_COMPONENTS_CUSTOM_STATEMENT_VISITOR_HPP
#define CORE_SYREC_PARSER_COMPONENTS_CUSTOM_STATEMENT_VISITOR_HPP
#pragma once

#include <core/syrec/statement.hpp>
#include <core/syrec/parser/components/custom_number_and_signal_visitor.hpp>
#include <core/syrec/parser/components/custom_expression_visitor.hpp>

namespace syrecParser {
    class CustomStatementVisitor: protected CustomBaseVisitor {
    public:
        CustomStatementVisitor(std::shared_ptr<ParserMessagesContainer> sharedMessagesContainerInstance):
            CustomBaseVisitor(sharedMessagesContainerInstance),
            expressionVisitorInstance(std::make_unique<CustomExpressionVisitor>(sharedMessagesContainerInstance)),
            numberAndSignalVisitorInstance(std::make_unique<CustomNumberAndSignalVisitor>(sharedMessagesContainerInstance)) {}
 
        [[nodiscard]] std::optional<syrec::Statement::vec> visitStatementListTyped(TSyrecParser::StatementListContext* ctx);
        [[nodiscard]] std::optional<syrec::Statement::ptr> visitStatementTyped(TSyrecParser::StatementContext* ctx);
        [[nodiscard]] std::optional<syrec::Statement::ptr> visitCallStatementTyped(TSyrecParser::CallStatementContext* ctx);
        [[nodiscard]] std::optional<syrec::Statement::ptr> visitForStatementTyped(TSyrecParser::ForStatementContext* ctx);
        [[nodiscard]] std::optional<syrec::Statement::ptr> visitIfStatementTyped(TSyrecParser::IfStatementContext* ctx);
        [[nodiscard]] std::optional<syrec::Statement::ptr> visitUnaryStatementTyped(TSyrecParser::UnaryStatementContext* ctx);
        [[nodiscard]] std::optional<syrec::Statement::ptr> visitAssignStatementTyped(TSyrecParser::AssignStatementContext* ctx);
        [[nodiscard]] std::optional<syrec::Statement::ptr> visitSwapStatementTyped(TSyrecParser::SwapStatementContext* ctx);
        [[nodiscard]] std::optional<syrec::Statement::ptr> visitSkipStatementTyped(TSyrecParser::SkipStatementContext* ctx);

    protected:
        std::unique_ptr<CustomExpressionVisitor> expressionVisitorInstance;
        std::unique_ptr<CustomNumberAndSignalVisitor> numberAndSignalVisitorInstance;

        struct LoopStepsizeDefinition {
            bool               hasMinusPrefix;
            syrec::Number::ptr stepsize;
        };

        [[nodiscard]] std::optional<std::string>            visitLoopVariableDefinitionTyped(TSyrecParser::LoopVariableDefinitionContext* ctx);
        [[nodiscard]] std::optional<LoopStepsizeDefinition> visitLoopStepsizeDefinitionTyped(TSyrecParser::LoopStepsizeDefinitionContext* ctx);

        std::any visitStatementList(TSyrecParser::StatementListContext* ctx) override;
        std::any visitStatement(TSyrecParser::StatementContext* ctx) override;
        std::any visitCallStatement(TSyrecParser::CallStatementContext* ctx) override;
        std::any visitLoopVariableDefinition(TSyrecParser::LoopVariableDefinitionContext* ctx) override;
        std::any visitLoopStepsizeDefinition(TSyrecParser::LoopStepsizeDefinitionContext* ctx) override;
        std::any visitForStatement(TSyrecParser::ForStatementContext* ctx) override;
        std::any visitIfStatement(TSyrecParser::IfStatementContext* ctx) override;
        std::any visitUnaryStatement(TSyrecParser::UnaryStatementContext* ctx) override;
        std::any visitAssignStatement(TSyrecParser::AssignStatementContext* ctx) override;
        std::any visitSwapStatement(TSyrecParser::SwapStatementContext* ctx) override;
        std::any visitSkipStatement(TSyrecParser::SkipStatementContext* ctx) override;
    };
} // namespace TSyrecParser
#endif