
// Generated from C:/School/MThesis/mqt-syrec-antlr/src/core/syrec/parser/antlr/grammar/TSyrecParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"


namespace syrec_parser {


class  TSyrecParser : public antlr4::Parser {
public:
  enum {
    OP_INCREMENT_ASSIGN = 1, OP_DECREMENT_ASSIGN = 2, OP_INVERT_ASSIGN = 3, 
    OP_ADD_ASSIGN = 4, OP_SUB_ASSIGN = 5, OP_XOR_ASSIGN = 6, OP_PLUS = 7, 
    OP_MINUS = 8, OP_MULTIPLY = 9, OP_UPPER_BIT_MULTIPLY = 10, OP_DIVISION = 11, 
    OP_MODULO = 12, OP_LEFT_SHIFT = 13, OP_RIGHT_SHIFT = 14, OP_SWAP = 15, 
    OP_GREATER_OR_EQUAL = 16, OP_LESS_OR_EQUAL = 17, OP_GREATER_THAN = 18, 
    OP_LESS_THAN = 19, OP_EQUAL = 20, OP_NOT_EQUAL = 21, OP_LOGICAL_AND = 22, 
    OP_LOGICAL_OR = 23, OP_LOGICAL_NEGATION = 24, OP_BITWISE_AND = 25, OP_BITWISE_NEGATION = 26, 
    OP_BITWISE_OR = 27, OP_BITWISE_XOR = 28, OP_CALL = 29, OP_UNCALL = 30, 
    VAR_TYPE_IN = 31, VAR_TYPE_OUT = 32, VAR_TYPE_INOUT = 33, VAR_TYPE_WIRE = 34, 
    VAR_TYPE_STATE = 35, LOOP_VARIABLE_PREFIX = 36, SIGNAL_WIDTH_PREFIX = 37, 
    STATEMENT_DELIMITER = 38, PARAMETER_DELIMITER = 39, OPEN_RBRACKET = 40, 
    CLOSE_RBRACKET = 41, OPEN_SBRACKET = 42, CLOSE_SBRACKET = 43, KEYWORD_MODULE = 44, 
    KEYWORD_FOR = 45, KEYWORD_DO = 46, KEYWORD_TO = 47, KEYWORD_STEP = 48, 
    KEYWORD_ROF = 49, KEYWORD_IF = 50, KEYWORD_THEN = 51, KEYWORD_ELSE = 52, 
    KEYWORD_FI = 53, KEYWORD_SKIP = 54, BITRANGE_START_PREFIX = 55, BITRANGE_END_PREFIX = 56, 
    SKIPABLEWHITSPACES = 57, LINE_COMMENT = 58, MULTI_LINE_COMMENT = 59, 
    IDENT = 60, INT = 61
  };

  enum {
    RuleNumber = 0, RuleProgram = 1, RuleModule = 2, RuleParameterList = 3, 
    RuleParameter = 4, RuleSignalList = 5, RuleSignalDeclaration = 6, RuleStatementList = 7, 
    RuleStatement = 8, RuleCallStatement = 9, RuleLoopVariableDefinition = 10, 
    RuleLoopStepsizeDefinition = 11, RuleForStatement = 12, RuleIfStatement = 13, 
    RuleUnaryStatement = 14, RuleAssignStatement = 15, RuleSwapStatement = 16, 
    RuleSkipStatement = 17, RuleSignal = 18, RuleExpression = 19, RuleBinaryExpression = 20, 
    RuleUnaryExpression = 21, RuleShiftExpression = 22
  };

  explicit TSyrecParser(antlr4::TokenStream *input);

  TSyrecParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~TSyrecParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class NumberContext;
  class ProgramContext;
  class ModuleContext;
  class ParameterListContext;
  class ParameterContext;
  class SignalListContext;
  class SignalDeclarationContext;
  class StatementListContext;
  class StatementContext;
  class CallStatementContext;
  class LoopVariableDefinitionContext;
  class LoopStepsizeDefinitionContext;
  class ForStatementContext;
  class IfStatementContext;
  class UnaryStatementContext;
  class AssignStatementContext;
  class SwapStatementContext;
  class SkipStatementContext;
  class SignalContext;
  class ExpressionContext;
  class BinaryExpressionContext;
  class UnaryExpressionContext;
  class ShiftExpressionContext; 

  class  NumberContext : public antlr4::ParserRuleContext {
  public:
    NumberContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    NumberContext() = default;
    void copyFrom(NumberContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  NumberFromSignalwidthContext : public NumberContext {
  public:
    NumberFromSignalwidthContext(NumberContext *ctx);

    antlr4::tree::TerminalNode *SIGNAL_WIDTH_PREFIX();
    antlr4::tree::TerminalNode *IDENT();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  NumberFromLoopVariableContext : public NumberContext {
  public:
    NumberFromLoopVariableContext(NumberContext *ctx);

    antlr4::tree::TerminalNode *LOOP_VARIABLE_PREFIX();
    antlr4::tree::TerminalNode *IDENT();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  NumberFromConstantContext : public NumberContext {
  public:
    NumberFromConstantContext(NumberContext *ctx);

    antlr4::tree::TerminalNode *INT();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  NumberFromExpressionContext : public NumberContext {
  public:
    NumberFromExpressionContext(NumberContext *ctx);

    TSyrecParser::NumberContext *lhsOperand = nullptr;
    antlr4::Token *op = nullptr;
    TSyrecParser::NumberContext *rhsOperand = nullptr;
    antlr4::tree::TerminalNode *OPEN_RBRACKET();
    antlr4::tree::TerminalNode *CLOSE_RBRACKET();
    std::vector<NumberContext *> number();
    NumberContext* number(size_t i);
    antlr4::tree::TerminalNode *OP_PLUS();
    antlr4::tree::TerminalNode *OP_MINUS();
    antlr4::tree::TerminalNode *OP_MULTIPLY();
    antlr4::tree::TerminalNode *OP_DIVISION();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  NumberContext* number();

  class  ProgramContext : public antlr4::ParserRuleContext {
  public:
    ProgramContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<ModuleContext *> module();
    ModuleContext* module(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ProgramContext* program();

  class  ModuleContext : public antlr4::ParserRuleContext {
  public:
    ModuleContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *KEYWORD_MODULE();
    antlr4::tree::TerminalNode *IDENT();
    antlr4::tree::TerminalNode *OPEN_RBRACKET();
    antlr4::tree::TerminalNode *CLOSE_RBRACKET();
    StatementListContext *statementList();
    ParameterListContext *parameterList();
    std::vector<SignalListContext *> signalList();
    SignalListContext* signalList(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ModuleContext* module();

  class  ParameterListContext : public antlr4::ParserRuleContext {
  public:
    ParameterListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ParameterContext *> parameter();
    ParameterContext* parameter(size_t i);
    std::vector<antlr4::tree::TerminalNode *> PARAMETER_DELIMITER();
    antlr4::tree::TerminalNode* PARAMETER_DELIMITER(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ParameterListContext* parameterList();

  class  ParameterContext : public antlr4::ParserRuleContext {
  public:
    ParameterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    SignalDeclarationContext *signalDeclaration();
    antlr4::tree::TerminalNode *VAR_TYPE_IN();
    antlr4::tree::TerminalNode *VAR_TYPE_OUT();
    antlr4::tree::TerminalNode *VAR_TYPE_INOUT();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ParameterContext* parameter();

  class  SignalListContext : public antlr4::ParserRuleContext {
  public:
    SignalListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<SignalDeclarationContext *> signalDeclaration();
    SignalDeclarationContext* signalDeclaration(size_t i);
    antlr4::tree::TerminalNode *VAR_TYPE_WIRE();
    antlr4::tree::TerminalNode *VAR_TYPE_STATE();
    std::vector<antlr4::tree::TerminalNode *> PARAMETER_DELIMITER();
    antlr4::tree::TerminalNode* PARAMETER_DELIMITER(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SignalListContext* signalList();

  class  SignalDeclarationContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *intToken = nullptr;
    std::vector<antlr4::Token *> dimensionTokens;
    antlr4::Token *signalWidthToken = nullptr;
    SignalDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENT();
    std::vector<antlr4::tree::TerminalNode *> OPEN_SBRACKET();
    antlr4::tree::TerminalNode* OPEN_SBRACKET(size_t i);
    std::vector<antlr4::tree::TerminalNode *> CLOSE_SBRACKET();
    antlr4::tree::TerminalNode* CLOSE_SBRACKET(size_t i);
    antlr4::tree::TerminalNode *OPEN_RBRACKET();
    antlr4::tree::TerminalNode *CLOSE_RBRACKET();
    std::vector<antlr4::tree::TerminalNode *> INT();
    antlr4::tree::TerminalNode* INT(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SignalDeclarationContext* signalDeclaration();

  class  StatementListContext : public antlr4::ParserRuleContext {
  public:
    TSyrecParser::StatementContext *statementContext = nullptr;
    std::vector<StatementContext *> stmts;
    StatementListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<StatementContext *> statement();
    StatementContext* statement(size_t i);
    std::vector<antlr4::tree::TerminalNode *> STATEMENT_DELIMITER();
    antlr4::tree::TerminalNode* STATEMENT_DELIMITER(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StatementListContext* statementList();

  class  StatementContext : public antlr4::ParserRuleContext {
  public:
    StatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    CallStatementContext *callStatement();
    ForStatementContext *forStatement();
    IfStatementContext *ifStatement();
    UnaryStatementContext *unaryStatement();
    AssignStatementContext *assignStatement();
    SwapStatementContext *swapStatement();
    SkipStatementContext *skipStatement();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StatementContext* statement();

  class  CallStatementContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *moduleIdent = nullptr;
    antlr4::Token *identToken = nullptr;
    std::vector<antlr4::Token *> callerArguments;
    CallStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OPEN_RBRACKET();
    antlr4::tree::TerminalNode *CLOSE_RBRACKET();
    antlr4::tree::TerminalNode *OP_CALL();
    antlr4::tree::TerminalNode *OP_UNCALL();
    std::vector<antlr4::tree::TerminalNode *> IDENT();
    antlr4::tree::TerminalNode* IDENT(size_t i);
    std::vector<antlr4::tree::TerminalNode *> PARAMETER_DELIMITER();
    antlr4::tree::TerminalNode* PARAMETER_DELIMITER(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CallStatementContext* callStatement();

  class  LoopVariableDefinitionContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *variableIdent = nullptr;
    LoopVariableDefinitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LOOP_VARIABLE_PREFIX();
    antlr4::tree::TerminalNode *OP_EQUAL();
    antlr4::tree::TerminalNode *IDENT();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LoopVariableDefinitionContext* loopVariableDefinition();

  class  LoopStepsizeDefinitionContext : public antlr4::ParserRuleContext {
  public:
    LoopStepsizeDefinitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *KEYWORD_STEP();
    NumberContext *number();
    antlr4::tree::TerminalNode *OP_MINUS();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LoopStepsizeDefinitionContext* loopStepsizeDefinition();

  class  ForStatementContext : public antlr4::ParserRuleContext {
  public:
    TSyrecParser::NumberContext *startValue = nullptr;
    TSyrecParser::NumberContext *endValue = nullptr;
    ForStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *KEYWORD_FOR();
    antlr4::tree::TerminalNode *KEYWORD_DO();
    StatementListContext *statementList();
    antlr4::tree::TerminalNode *KEYWORD_ROF();
    std::vector<NumberContext *> number();
    NumberContext* number(size_t i);
    antlr4::tree::TerminalNode *KEYWORD_TO();
    LoopStepsizeDefinitionContext *loopStepsizeDefinition();
    LoopVariableDefinitionContext *loopVariableDefinition();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ForStatementContext* forStatement();

  class  IfStatementContext : public antlr4::ParserRuleContext {
  public:
    TSyrecParser::ExpressionContext *guardCondition = nullptr;
    TSyrecParser::StatementListContext *trueBranchStmts = nullptr;
    TSyrecParser::StatementListContext *falseBranchStmts = nullptr;
    TSyrecParser::ExpressionContext *matchingGuardExpression = nullptr;
    IfStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *KEYWORD_IF();
    antlr4::tree::TerminalNode *KEYWORD_THEN();
    antlr4::tree::TerminalNode *KEYWORD_ELSE();
    antlr4::tree::TerminalNode *KEYWORD_FI();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    std::vector<StatementListContext *> statementList();
    StatementListContext* statementList(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IfStatementContext* ifStatement();

  class  UnaryStatementContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *unaryOp = nullptr;
    UnaryStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    SignalContext *signal();
    antlr4::tree::TerminalNode *OP_INVERT_ASSIGN();
    antlr4::tree::TerminalNode *OP_INCREMENT_ASSIGN();
    antlr4::tree::TerminalNode *OP_DECREMENT_ASSIGN();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnaryStatementContext* unaryStatement();

  class  AssignStatementContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *assignmentOp = nullptr;
    AssignStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    SignalContext *signal();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *OP_ADD_ASSIGN();
    antlr4::tree::TerminalNode *OP_SUB_ASSIGN();
    antlr4::tree::TerminalNode *OP_XOR_ASSIGN();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AssignStatementContext* assignStatement();

  class  SwapStatementContext : public antlr4::ParserRuleContext {
  public:
    TSyrecParser::SignalContext *lhsOperand = nullptr;
    TSyrecParser::SignalContext *rhsOperand = nullptr;
    SwapStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OP_SWAP();
    std::vector<SignalContext *> signal();
    SignalContext* signal(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SwapStatementContext* swapStatement();

  class  SkipStatementContext : public antlr4::ParserRuleContext {
  public:
    SkipStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *KEYWORD_SKIP();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SkipStatementContext* skipStatement();

  class  SignalContext : public antlr4::ParserRuleContext {
  public:
    TSyrecParser::ExpressionContext *expressionContext = nullptr;
    std::vector<ExpressionContext *> accessedDimensions;
    TSyrecParser::NumberContext *bitStart = nullptr;
    TSyrecParser::NumberContext *bitRangeEnd = nullptr;
    SignalContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENT();
    std::vector<antlr4::tree::TerminalNode *> OPEN_SBRACKET();
    antlr4::tree::TerminalNode* OPEN_SBRACKET(size_t i);
    std::vector<antlr4::tree::TerminalNode *> CLOSE_SBRACKET();
    antlr4::tree::TerminalNode* CLOSE_SBRACKET(size_t i);
    antlr4::tree::TerminalNode *BITRANGE_START_PREFIX();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    std::vector<NumberContext *> number();
    NumberContext* number(size_t i);
    antlr4::tree::TerminalNode *BITRANGE_END_PREFIX();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SignalContext* signal();

  class  ExpressionContext : public antlr4::ParserRuleContext {
  public:
    ExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    ExpressionContext() = default;
    void copyFrom(ExpressionContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  ExpressionFromSignalContext : public ExpressionContext {
  public:
    ExpressionFromSignalContext(ExpressionContext *ctx);

    SignalContext *signal();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ExpressionFromBinaryExpressionContext : public ExpressionContext {
  public:
    ExpressionFromBinaryExpressionContext(ExpressionContext *ctx);

    BinaryExpressionContext *binaryExpression();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ExpressionFromNumberContext : public ExpressionContext {
  public:
    ExpressionFromNumberContext(ExpressionContext *ctx);

    NumberContext *number();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ExpressionFromUnaryExpressionContext : public ExpressionContext {
  public:
    ExpressionFromUnaryExpressionContext(ExpressionContext *ctx);

    UnaryExpressionContext *unaryExpression();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ExpressionFromShiftExpressionContext : public ExpressionContext {
  public:
    ExpressionFromShiftExpressionContext(ExpressionContext *ctx);

    ShiftExpressionContext *shiftExpression();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  ExpressionContext* expression();

  class  BinaryExpressionContext : public antlr4::ParserRuleContext {
  public:
    TSyrecParser::ExpressionContext *lhsOperand = nullptr;
    antlr4::Token *binaryOperation = nullptr;
    TSyrecParser::ExpressionContext *rhsOperand = nullptr;
    BinaryExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OPEN_RBRACKET();
    antlr4::tree::TerminalNode *CLOSE_RBRACKET();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *OP_PLUS();
    antlr4::tree::TerminalNode *OP_MINUS();
    antlr4::tree::TerminalNode *OP_MULTIPLY();
    antlr4::tree::TerminalNode *OP_DIVISION();
    antlr4::tree::TerminalNode *OP_MODULO();
    antlr4::tree::TerminalNode *OP_UPPER_BIT_MULTIPLY();
    antlr4::tree::TerminalNode *OP_LOGICAL_AND();
    antlr4::tree::TerminalNode *OP_LOGICAL_OR();
    antlr4::tree::TerminalNode *OP_BITWISE_AND();
    antlr4::tree::TerminalNode *OP_BITWISE_OR();
    antlr4::tree::TerminalNode *OP_BITWISE_XOR();
    antlr4::tree::TerminalNode *OP_LESS_THAN();
    antlr4::tree::TerminalNode *OP_GREATER_THAN();
    antlr4::tree::TerminalNode *OP_EQUAL();
    antlr4::tree::TerminalNode *OP_NOT_EQUAL();
    antlr4::tree::TerminalNode *OP_LESS_OR_EQUAL();
    antlr4::tree::TerminalNode *OP_GREATER_OR_EQUAL();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BinaryExpressionContext* binaryExpression();

  class  UnaryExpressionContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *unaryOperation = nullptr;
    UnaryExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *OP_LOGICAL_NEGATION();
    antlr4::tree::TerminalNode *OP_BITWISE_NEGATION();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnaryExpressionContext* unaryExpression();

  class  ShiftExpressionContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *shiftOperation = nullptr;
    ShiftExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OPEN_RBRACKET();
    ExpressionContext *expression();
    NumberContext *number();
    antlr4::tree::TerminalNode *CLOSE_RBRACKET();
    antlr4::tree::TerminalNode *OP_RIGHT_SHIFT();
    antlr4::tree::TerminalNode *OP_LEFT_SHIFT();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ShiftExpressionContext* shiftExpression();


  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

}  // namespace syrec_parser
