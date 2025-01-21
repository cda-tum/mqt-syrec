#ifndef CORE_SYREC_PARSER_COMPONENTS_CUSTOM_MODULE_VISITOR_HPP
#define CORE_SYREC_PARSER_COMPONENTS_CUSTOM_MODULE_VISITOR_HPP
#pragma once

#include <core/syrec/program.hpp>
#include <core/syrec/parser/components/custom_base_visitor.hpp>
#include <core/syrec/parser/components/custom_statement_visitor.hpp>

namespace syrecParser {
    class CustomModuleVisitor: protected CustomBaseVisitor {
    public:
        CustomModuleVisitor(const std::shared_ptr<ParserMessagesContainer>& sharedMessagesContainerInstance, const syrec::ReadProgramSettings& userProvidedParserSettings):
            CustomBaseVisitor(sharedMessagesContainerInstance, std::make_shared<utils::BaseSymbolTable>()),
            defaultVariableBitwidth(userProvidedParserSettings.defaultBitwidth),
            statementVisitorInstance(std::make_unique<CustomStatementVisitor>(sharedGeneratedMessageContainerInstance, this->symbolTable)) {}

        [[maybe_unused]] std::optional<std::shared_ptr<syrec::Program>> parseProgram(TSyrecParser::ProgramContext* context) const; 

    protected:
        static constexpr unsigned int MAX_SUPPORTED_SIGNAL_BITWIDTH = 32;
        unsigned int                  defaultVariableBitwidth;

        std::unique_ptr<CustomStatementVisitor> statementVisitorInstance;
        [[nodiscard]] std::optional<std::shared_ptr<syrec::Program>>    visitProgramTyped(TSyrecParser::ProgramContext* context) const;
        [[nodiscard]] std::optional<syrec::Module::ptr>                 visitModuleTyped(TSyrecParser::ModuleContext* context) const;
        [[nodiscard]] std::optional<std::vector<syrec::Variable::ptr>>  visitParameterListTyped(TSyrecParser::ParameterListContext* context) const;
        [[nodiscard]] std::optional<syrec::Variable::ptr>               visitParameterTyped(TSyrecParser::ParameterContext* context) const;
        [[nodiscard]] std::optional<std::vector<syrec::Variable::ptr>>  visitSignalListTyped(TSyrecParser::SignalListContext* context) const;
        [[nodiscard]] std::optional<syrec::Variable::ptr>               visitSignalDeclarationTyped(TSyrecParser::SignalDeclarationContext* context) const;
        [[nodiscard]] std::optional<std::vector<syrec::Statement::ptr>> visitStatementListTyped(const TSyrecParser::StatementListContext* context) const;

        [[nodiscard]] static bool doVariablesMatch(const syrec::Variable& lVariable, const syrec::Variable& rVariable);
        [[nodiscard]] static bool doVariableCollectionsMatch(const syrec::Variable::vec& lVariableCollection, const syrec::Variable::vec& rVariableCollection);
    };
} // namespace syrecParser
#endif