
// Generated from C:/School/MThesis/mqt-syrec-antlr/src/core/syrec/parser/antlr/grammar/TSyrecParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "TSyrecParserVisitor.h"


namespace syrecParser {

/**
 * This class provides an empty implementation of TSyrecParserVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  TSyrecParserBaseVisitor : public TSyrecParserVisitor {
public:

  virtual std::any visitNumberFromConstant(TSyrecParser::NumberFromConstantContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNumberFromSignalwidth(TSyrecParser::NumberFromSignalwidthContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNumberFromLoopVariable(TSyrecParser::NumberFromLoopVariableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNumberFromExpression(TSyrecParser::NumberFromExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitProgram(TSyrecParser::ProgramContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitModule(TSyrecParser::ModuleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParameterList(TSyrecParser::ParameterListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParameter(TSyrecParser::ParameterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSignalList(TSyrecParser::SignalListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSignalDeclaration(TSyrecParser::SignalDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStatementList(TSyrecParser::StatementListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStatement(TSyrecParser::StatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCallStatement(TSyrecParser::CallStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLoopVariableDefinition(TSyrecParser::LoopVariableDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLoopStepsizeDefinition(TSyrecParser::LoopStepsizeDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForStatement(TSyrecParser::ForStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIfStatement(TSyrecParser::IfStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryStatement(TSyrecParser::UnaryStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAssignStatement(TSyrecParser::AssignStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSwapStatement(TSyrecParser::SwapStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSkipStatement(TSyrecParser::SkipStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSignal(TSyrecParser::SignalContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpressionFromNumber(TSyrecParser::ExpressionFromNumberContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpressionFromSignal(TSyrecParser::ExpressionFromSignalContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpressionFromBinaryExpression(TSyrecParser::ExpressionFromBinaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpressionFromUnaryExpression(TSyrecParser::ExpressionFromUnaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpressionFromShiftExpression(TSyrecParser::ExpressionFromShiftExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBinaryExpression(TSyrecParser::BinaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryExpression(TSyrecParser::UnaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShiftExpression(TSyrecParser::ShiftExpressionContext *ctx) override {
    return visitChildren(ctx);
  }


};

}  // namespace syrecParser
