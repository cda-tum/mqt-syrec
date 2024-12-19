#ifndef CORE_SYREC_PARSER_COMPONENTS_CUSTOM_MODULE_VISITOR_HPP
#define CORE_SYREC_PARSER_COMPONENTS_CUSTOM_MODULE_VISITOR_HPP
#pragma once

#include <core/syrec/program.hpp>
#include <core/syrec/parser/components/custom_base_visitor.hpp>
#include <core/syrec/parser/components/custom_statement_visitor.hpp>

namespace syrecParser {
    class CustomModuleVisitor: protected CustomBaseVisitor {
    public:
        CustomModuleVisitor(std::shared_ptr<ParserMessagesContainer> sharedMessagesContainerInstance):
            CustomBaseVisitor(sharedMessagesContainerInstance),
            statementVisitorInstance(std::make_unique<CustomStatementVisitor>(sharedMessagesContainerInstance)) {}

        [[maybe_unused]] std::optional<std::unique_ptr<syrec::Program>> parseProgram(TSyrecParser::ProgramContext* context); 

    protected:
        std::unique_ptr<CustomStatementVisitor> statementVisitorInstance;

        [[nodiscard]] std::optional<std::unique_ptr<syrec::Program>>    visitProgramTyped(TSyrecParser::ProgramContext* context);
        [[nodiscard]] std::optional<syrec::Module::ptr>                 visitModuleTyped(TSyrecParser::ModuleContext* context);
        [[nodiscard]] std::optional<std::vector<syrec::Variable::ptr>>  visitParameterListTyped(TSyrecParser::ParameterListContext* context);
        [[nodiscard]] std::optional<syrec::Variable::ptr>               visitParameterTyped(TSyrecParser::ParameterContext* context);
        [[nodiscard]] std::optional<std::vector<syrec::Variable::ptr>>  visitSignalListTyped(TSyrecParser::SignalListContext* context);
        [[nodiscard]] std::optional<syrec::Variable::ptr>               visitSignalDeclarationTyped(TSyrecParser::SignalDeclarationContext* context);
        [[nodiscard]] std::optional<std::vector<syrec::Statement::ptr>> visitStatementListTyped(TSyrecParser::StatementListContext* context);

    private:
        std::any visitProgram(TSyrecParser::ProgramContext* context) override;
        std::any visitModule(TSyrecParser::ModuleContext* context) override;
        std::any visitParameterList(TSyrecParser::ParameterListContext* context) override;
        std::any visitParameter(TSyrecParser::ParameterContext* context) override;
        std::any visitSignalList(TSyrecParser::SignalListContext* context) override;
        std::any visitSignalDeclaration(TSyrecParser::SignalDeclarationContext* context) override;
        std::any visitStatementList(TSyrecParser::StatementListContext* context) override;
    };
} // namespace syrecParser
#endif