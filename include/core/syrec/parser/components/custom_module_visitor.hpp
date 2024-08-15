#ifndef CUSTOM_MODULE_VISITOR_HPP
#define CUSTOM_MODULE_VISITOR_HPP
#pragma once

#include "custom_base_visitor.hpp"

namespace syrecParser {
    class CustomModuleVisitor: CustomBaseVisitor {
    public:
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