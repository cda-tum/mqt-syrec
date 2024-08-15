
// Generated from C:/School/MThesis/mqt-syrec-antlr/src/core/syrec/parser/antlr/grammar/TSyrecParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "TSyrecParser.h"


namespace syrecParser {

/**
 * This class defines an abstract visitor for a parse tree
 * produced by TSyrecParser.
 */
class  TSyrecParserVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by TSyrecParser.
   */
    virtual std::any visitNumberFromConstant(TSyrecParser::NumberFromConstantContext *context) = 0;

    virtual std::any visitNumberFromSignalwidth(TSyrecParser::NumberFromSignalwidthContext *context) = 0;

    virtual std::any visitNumberFromLoopVariable(TSyrecParser::NumberFromLoopVariableContext *context) = 0;

    virtual std::any visitNumberFromExpression(TSyrecParser::NumberFromExpressionContext *context) = 0;

    virtual std::any visitProgram(TSyrecParser::ProgramContext *context) = 0;

    virtual std::any visitModule(TSyrecParser::ModuleContext *context) = 0;

    virtual std::any visitParameterList(TSyrecParser::ParameterListContext *context) = 0;

    virtual std::any visitParameter(TSyrecParser::ParameterContext *context) = 0;

    virtual std::any visitSignalList(TSyrecParser::SignalListContext *context) = 0;

    virtual std::any visitSignalDeclaration(TSyrecParser::SignalDeclarationContext *context) = 0;

    virtual std::any visitStatementList(TSyrecParser::StatementListContext *context) = 0;

    virtual std::any visitStatement(TSyrecParser::StatementContext *context) = 0;

    virtual std::any visitCallStatement(TSyrecParser::CallStatementContext *context) = 0;

    virtual std::any visitLoopVariableDefinition(TSyrecParser::LoopVariableDefinitionContext *context) = 0;

    virtual std::any visitLoopStepsizeDefinition(TSyrecParser::LoopStepsizeDefinitionContext *context) = 0;

    virtual std::any visitForStatement(TSyrecParser::ForStatementContext *context) = 0;

    virtual std::any visitIfStatement(TSyrecParser::IfStatementContext *context) = 0;

    virtual std::any visitUnaryStatement(TSyrecParser::UnaryStatementContext *context) = 0;

    virtual std::any visitAssignStatement(TSyrecParser::AssignStatementContext *context) = 0;

    virtual std::any visitSwapStatement(TSyrecParser::SwapStatementContext *context) = 0;

    virtual std::any visitSkipStatement(TSyrecParser::SkipStatementContext *context) = 0;

    virtual std::any visitSignal(TSyrecParser::SignalContext *context) = 0;

    virtual std::any visitExpressionFromNumber(TSyrecParser::ExpressionFromNumberContext *context) = 0;

    virtual std::any visitExpressionFromSignal(TSyrecParser::ExpressionFromSignalContext *context) = 0;

    virtual std::any visitExpressionFromBinaryExpression(TSyrecParser::ExpressionFromBinaryExpressionContext *context) = 0;

    virtual std::any visitExpressionFromUnaryExpression(TSyrecParser::ExpressionFromUnaryExpressionContext *context) = 0;

    virtual std::any visitExpressionFromShiftExpression(TSyrecParser::ExpressionFromShiftExpressionContext *context) = 0;

    virtual std::any visitBinaryExpression(TSyrecParser::BinaryExpressionContext *context) = 0;

    virtual std::any visitUnaryExpression(TSyrecParser::UnaryExpressionContext *context) = 0;

    virtual std::any visitShiftExpression(TSyrecParser::ShiftExpressionContext *context) = 0;


};

}  // namespace syrecParser
