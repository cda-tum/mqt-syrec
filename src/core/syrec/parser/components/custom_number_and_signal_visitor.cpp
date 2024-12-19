#include "core/syrec/parser/components/custom_number_and_signal_visitor.hpp"

using namespace syrecParser;

std::optional<syrec::Number::ptr> CustomNumberAndSignalVisitor::visitNumberFromConstantTyped(TSyrecParser::NumberFromConstantContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Number>(context);
}

std::optional<syrec::Number::ptr> CustomNumberAndSignalVisitor::visitNumberFromExpressionTyped(TSyrecParser::NumberFromExpressionContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Number>(context);
}

std::optional<syrec::Number::ptr> CustomNumberAndSignalVisitor::visitNumberFromLoopVariableTyped(TSyrecParser::NumberFromLoopVariableContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Number>(context);
}

std::optional<syrec::Number::ptr> CustomNumberAndSignalVisitor::visitNumberFromSignalwidthTyped(TSyrecParser::NumberFromSignalwidthContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Number>(context);
}

std::optional<syrec::Variable::ptr> CustomNumberAndSignalVisitor::visitSignalTyped(TSyrecParser::SignalContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Variable>(context);
}

// START OF NON-PUBLIC FUNCTIONALITY
std::any CustomNumberAndSignalVisitor::visitNumberFromConstant(TSyrecParser::NumberFromConstantContext* context) {
    return 0;
}

std::any CustomNumberAndSignalVisitor::visitNumberFromExpression(TSyrecParser::NumberFromExpressionContext* context) {
    return 0;
}

std::any CustomNumberAndSignalVisitor::visitNumberFromLoopVariable(TSyrecParser::NumberFromLoopVariableContext* context) {
    return 0;
}

std::any CustomNumberAndSignalVisitor::visitNumberFromSignalwidth(TSyrecParser::NumberFromSignalwidthContext* context) {
    return 0;
}

std::any CustomNumberAndSignalVisitor::visitSignal(TSyrecParser::SignalContext* context) {
    return 0;
}