#include "core/syrec/parser/components/custom_module_visitor.hpp"

using namespace syrecParser;

std::optional<std::unique_ptr<syrec::Program>> CustomModuleVisitor::parseProgram(TSyrecParser::ProgramContext* context) {
    return visitProgramTyped(context);
}

std::optional<std::unique_ptr<syrec::Program>> CustomModuleVisitor::visitProgramTyped(TSyrecParser::ProgramContext* context) {
    return std::nullopt;
}

std::optional<syrec::Module::ptr> CustomModuleVisitor::visitModuleTyped(TSyrecParser::ModuleContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Module>(context);
}

std::optional<std::vector<syrec::Variable::ptr>> CustomModuleVisitor::visitParameterListTyped(TSyrecParser::ParameterListContext* context) {
    return visitNonTerminalSymbolWithManyResults<syrec::Variable>(context);
}

std::optional<syrec::Variable::ptr> CustomModuleVisitor::visitParameterTyped(TSyrecParser::ParameterContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Variable>(context);
}

std::optional<std::vector<syrec::Variable::ptr>> CustomModuleVisitor::visitSignalListTyped(TSyrecParser::SignalListContext* context) {
    return visitNonTerminalSymbolWithManyResults<syrec::Variable>(context);
}

std::optional<syrec::Variable::ptr> CustomModuleVisitor::visitSignalDeclarationTyped(TSyrecParser::SignalDeclarationContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Variable>(context);
}

std::optional<std::vector<syrec::Statement::ptr>> CustomModuleVisitor::visitStatementListTyped(TSyrecParser::StatementListContext* context) {
    return visitNonTerminalSymbolWithManyResults<syrec::Statement>(context);
}

// START OF NON-PUBLIC FUNCTIONALITY
std::any CustomModuleVisitor::visitProgram(TSyrecParser::ProgramContext* context) {
    return 0;
}

std::any CustomModuleVisitor::visitModule(TSyrecParser::ModuleContext* context) {
    return 0;
}

std::any CustomModuleVisitor::visitParameterList(TSyrecParser::ParameterListContext* context) {
    return 0;
}

std::any CustomModuleVisitor::visitParameter(TSyrecParser::ParameterContext* context) {
    return 0;
}

std::any CustomModuleVisitor::visitSignalList(TSyrecParser::SignalListContext* context) {
    return 0;
}

std::any CustomModuleVisitor::visitSignalDeclaration(TSyrecParser::SignalDeclarationContext* context) {
    return 0;
}

std::any CustomModuleVisitor::visitStatementList(TSyrecParser::StatementListContext* context) {
    return 0;
}