#include "core/syrec/parser/components/custom_base_visitor.hpp"

using namespace syrecParser;

std::any CustomBaseVisitor::visitNumberFromConstant(TSyrecParser::NumberFromConstantContext* context) {
    return 0;
}

std::any CustomBaseVisitor::visitNumberFromExpression(TSyrecParser::NumberFromExpressionContext* context) {
    return 0;
}

std::any CustomBaseVisitor::visitNumberFromLoopVariable(TSyrecParser::NumberFromLoopVariableContext* context) {
    return 0;
}

std::any CustomBaseVisitor::visitNumberFromSignalwidth(TSyrecParser::NumberFromSignalwidthContext* context) {
    return 0;
}

std::any CustomBaseVisitor::visitSignal(TSyrecParser::SignalContext* context) {
    return 0;
}