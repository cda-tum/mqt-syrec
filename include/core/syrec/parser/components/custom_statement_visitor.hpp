#ifndef CUSTOM_STATEMENT_VISITOR_HPP
#define CUSTOM_STATEMENT_VISITOR_HPP
#pragma once

#include "custom_base_visitor.hpp"

namespace syrecParser {
    class CustomStatementVisitor: public CustomBaseVisitor {
    public:
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