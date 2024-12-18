#include "core/syrec/parser/components/custom_module_visitor.hpp"

using namespace syrecParser;

std::optional<std::unique_ptr<syrec::Program>> CustomModuleVisitor::parseProgram(TSyrecParser::ProgramContext* context) {
    return std::nullopt;
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