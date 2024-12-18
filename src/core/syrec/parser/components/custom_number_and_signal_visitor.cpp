#include "core/syrec/parser/components/custom_number_and_signal_visitor.hpp"

using namespace syrecParser;

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