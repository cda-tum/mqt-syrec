#ifndef CORE_SYREC_PARSER_ANTLR_TSYRECPARSER_H
#define CORE_SYREC_PARSER_ANTLR_TSYRECPARSER_H
#pragma once

// Generated from C:/School/MThesis/mqt-syrec-antlr/src/core/syrec/parser/antlr/grammar/TSyrecParser.g4 by ANTLR 4.13.2
// We need to include the CommenTokenStream header file as otherwise, X would be
#include "Parser.h"
#include "ParserRuleContext.h"
#include "Token.h"
#include "TokenStream.h"
#include "Vocabulary.h"
#include "atn/ATN.h"
#include "atn/ParserATNSimulatorOptions.h"
#include "atn/SerializedATNView.h"
#include "tree/ParseTreeVisitor.h"
#include "tree/TerminalNode.h"

#include <any>
#include <cstddef>
#include <string>
#include <vector>

namespace syrec_parser {
    class TSyrecParser: public antlr4::Parser {
    public:
        enum {
            OP_INCREMENT_ASSIGN   = 1,
            OP_DECREMENT_ASSIGN   = 2,
            OP_INVERT_ASSIGN      = 3,
            OP_ADD_ASSIGN         = 4,
            OP_SUB_ASSIGN         = 5,
            OP_XOR_ASSIGN         = 6,
            OP_PLUS               = 7,
            OP_MINUS              = 8,
            OP_MULTIPLY           = 9,
            OP_UPPER_BIT_MULTIPLY = 10,
            OP_DIVISION           = 11,
            OP_MODULO             = 12,
            OP_LEFT_SHIFT         = 13,
            OP_RIGHT_SHIFT        = 14,
            OP_SWAP               = 15,
            OP_GREATER_OR_EQUAL   = 16,
            OP_LESS_OR_EQUAL      = 17,
            OP_GREATER_THAN       = 18,
            OP_LESS_THAN          = 19,
            OP_EQUAL              = 20,
            OP_NOT_EQUAL          = 21,
            OP_LOGICAL_AND        = 22,
            OP_LOGICAL_OR         = 23,
            OP_LOGICAL_NEGATION   = 24,
            OP_BITWISE_AND        = 25,
            OP_BITWISE_NEGATION   = 26,
            OP_BITWISE_OR         = 27,
            OP_BITWISE_XOR        = 28,
            OP_CALL               = 29,
            OP_UNCALL             = 30,
            VAR_TYPE_IN           = 31,
            VAR_TYPE_OUT          = 32,
            VAR_TYPE_INOUT        = 33,
            VAR_TYPE_WIRE         = 34,
            VAR_TYPE_STATE        = 35,
            LOOP_VARIABLE_PREFIX  = 36,
            SIGNAL_WIDTH_PREFIX   = 37,
            STATEMENT_DELIMITER   = 38,
            PARAMETER_DELIMITER   = 39,
            OPEN_RBRACKET         = 40,
            CLOSE_RBRACKET        = 41,
            OPEN_SBRACKET         = 42,
            CLOSE_SBRACKET        = 43,
            KEYWORD_MODULE        = 44,
            KEYWORD_FOR           = 45,
            KEYWORD_DO            = 46,
            KEYWORD_TO            = 47,
            KEYWORD_STEP          = 48,
            KEYWORD_ROF           = 49,
            KEYWORD_IF            = 50,
            KEYWORD_THEN          = 51,
            KEYWORD_ELSE          = 52,
            KEYWORD_FI            = 53,
            KEYWORD_SKIP          = 54,
            BITRANGE_START_PREFIX = 55,
            BITRANGE_END_PREFIX   = 56,
            SKIPABLEWHITSPACES    = 57,
            LINE_COMMENT          = 58,
            MULTI_LINE_COMMENT    = 59,
            IDENT                 = 60,
            INT                   = 61
        };

        enum {
            RuleNumber                 = 0,
            RuleProgram                = 1,
            RuleModule                 = 2,
            RuleParameterList          = 3,
            RuleParameter              = 4,
            RuleSignalList             = 5,
            RuleSignalDeclaration      = 6,
            RuleStatementList          = 7,
            RuleStatement              = 8,
            RuleCallStatement          = 9,
            RuleLoopVariableDefinition = 10,
            RuleLoopStepsizeDefinition = 11,
            RuleForStatement           = 12,
            RuleIfStatement            = 13,
            RuleUnaryStatement         = 14,
            RuleAssignStatement        = 15,
            RuleSwapStatement          = 16,
            RuleSkipStatement          = 17,
            RuleSignal                 = 18,
            RuleExpression             = 19,
            RuleBinaryExpression       = 20,
            RuleUnaryExpression        = 21,
            RuleShiftExpression        = 22
        };

        explicit TSyrecParser(antlr4::TokenStream* input);

        TSyrecParser(antlr4::TokenStream* input, const antlr4::atn::ParserATNSimulatorOptions& options);
        ~TSyrecParser() override;

        [[nodiscard]] std::string                     getGrammarFileName() const override;
        [[nodiscard]] const antlr4::atn::ATN&         getATN() const override;
        [[nodiscard]] const std::vector<std::string>& getRuleNames() const override;
        [[nodiscard]] const antlr4::dfa::Vocabulary&  getVocabulary() const override;
        [[nodiscard]] antlr4::atn::SerializedATNView  getSerializedATN() const override;

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

        class NumberContext: public antlr4::ParserRuleContext {
        public:
            NumberContext(antlr4::ParserRuleContext* parent, size_t invokingState);

            NumberContext() = default;
            void copyFrom(NumberContext* ctx);
            using antlr4::ParserRuleContext::copyFrom;

            [[nodiscard]] size_t getRuleIndex() const override;
        };

        class NumberFromSignalwidthContext: public NumberContext {
        public:
            explicit NumberFromSignalwidthContext(NumberContext* ctx);

            [[nodiscard]] antlr4::tree::TerminalNode* SIGNAL_WIDTH_PREFIX() const;
            [[nodiscard]] antlr4::tree::TerminalNode* IDENT() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        class NumberFromLoopVariableContext: public NumberContext {
        public:
            explicit NumberFromLoopVariableContext(NumberContext* ctx);

            [[nodiscard]] antlr4::tree::TerminalNode* LOOP_VARIABLE_PREFIX() const;
            [[nodiscard]] antlr4::tree::TerminalNode* IDENT() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        class NumberFromConstantContext: public NumberContext {
        public:
            explicit NumberFromConstantContext(NumberContext* ctx);

            [[nodiscard]] antlr4::tree::TerminalNode* INT() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        class NumberFromExpressionContext: public NumberContext {
        public:
            explicit NumberFromExpressionContext(NumberContext* ctx);

            TSyrecParser::NumberContext*              lhsOperand = nullptr;
            antlr4::Token*                            op         = nullptr;
            TSyrecParser::NumberContext*              rhsOperand = nullptr;
            [[nodiscard]] antlr4::tree::TerminalNode* OPEN_RBRACKET() const;
            [[nodiscard]] antlr4::tree::TerminalNode* CLOSE_RBRACKET() const;
            [[nodiscard]] std::vector<NumberContext*> number() const;
            [[nodiscard]] NumberContext*              number(size_t i) const;
            [[nodiscard]] antlr4::tree::TerminalNode* OP_PLUS() const;
            [[nodiscard]] antlr4::tree::TerminalNode* OP_MINUS() const;
            [[nodiscard]] antlr4::tree::TerminalNode* OP_MULTIPLY() const;
            [[nodiscard]] antlr4::tree::TerminalNode* OP_DIVISION() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        NumberContext* number();

        class ProgramContext: public antlr4::ParserRuleContext {
        public:
            ProgramContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                      getRuleIndex() const override;
            [[nodiscard]] antlr4::tree::TerminalNode* EOF() const;
            [[nodiscard]] std::vector<ModuleContext*> module() const;
            [[nodiscard]] ModuleContext*              module(size_t i) const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        ProgramContext* program();

        class ModuleContext: public antlr4::ParserRuleContext {
        public:
            ModuleContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                          getRuleIndex() const override;
            [[nodiscard]] antlr4::tree::TerminalNode*     KEYWORD_MODULE() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     IDENT() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     OPEN_RBRACKET() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     CLOSE_RBRACKET() const;
            [[nodiscard]] StatementListContext*           statementList() const;
            [[nodiscard]] ParameterListContext*           parameterList() const;
            [[nodiscard]] std::vector<SignalListContext*> signalList() const;
            [[nodiscard]] SignalListContext*              signalList(size_t i) const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        ModuleContext* module();

        class ParameterListContext: public antlr4::ParserRuleContext {
        public:
            ParameterListContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                                   getRuleIndex() const override;
            [[nodiscard]] std::vector<ParameterContext*>           parameter() const;
            [[nodiscard]] ParameterContext*                        parameter(size_t i) const;
            [[nodiscard]] std::vector<antlr4::tree::TerminalNode*> PARAMETER_DELIMITER() const;
            [[nodiscard]] antlr4::tree::TerminalNode*              PARAMETER_DELIMITER(size_t i) const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        ParameterListContext* parameterList();

        class ParameterContext: public antlr4::ParserRuleContext {
        public:
            ParameterContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                      getRuleIndex() const override;
            [[nodiscard]] SignalDeclarationContext*   signalDeclaration() const;
            [[nodiscard]] antlr4::tree::TerminalNode* VAR_TYPE_IN() const;
            [[nodiscard]] antlr4::tree::TerminalNode* VAR_TYPE_OUT() const;
            [[nodiscard]] antlr4::tree::TerminalNode* VAR_TYPE_INOUT() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        ParameterContext* parameter();

        class SignalListContext: public antlr4::ParserRuleContext {
        public:
            SignalListContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                                   getRuleIndex() const override;
            [[nodiscard]] std::vector<SignalDeclarationContext*>   signalDeclaration() const;
            [[nodiscard]] SignalDeclarationContext*                signalDeclaration(size_t i) const;
            [[nodiscard]] antlr4::tree::TerminalNode*              VAR_TYPE_WIRE() const;
            [[nodiscard]] antlr4::tree::TerminalNode*              VAR_TYPE_STATE() const;
            [[nodiscard]] std::vector<antlr4::tree::TerminalNode*> PARAMETER_DELIMITER() const;
            [[nodiscard]] antlr4::tree::TerminalNode*              PARAMETER_DELIMITER(size_t i) const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        SignalListContext* signalList();

        class SignalDeclarationContext: public antlr4::ParserRuleContext {
        public:
            antlr4::Token*              intToken = nullptr;
            std::vector<antlr4::Token*> dimensionTokens;
            antlr4::Token*              signalWidthToken = nullptr;
            SignalDeclarationContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                                   getRuleIndex() const override;
            [[nodiscard]] antlr4::tree::TerminalNode*              IDENT() const;
            [[nodiscard]] std::vector<antlr4::tree::TerminalNode*> OPEN_SBRACKET() const;
            [[nodiscard]] antlr4::tree::TerminalNode*              OPEN_SBRACKET(size_t i) const;
            [[nodiscard]] std::vector<antlr4::tree::TerminalNode*> CLOSE_SBRACKET() const;
            [[nodiscard]] antlr4::tree::TerminalNode*              CLOSE_SBRACKET(size_t i) const;
            [[nodiscard]] antlr4::tree::TerminalNode*              OPEN_RBRACKET() const;
            [[nodiscard]] antlr4::tree::TerminalNode*              CLOSE_RBRACKET() const;
            [[nodiscard]] std::vector<antlr4::tree::TerminalNode*> INT() const;
            [[nodiscard]] antlr4::tree::TerminalNode*              INT(size_t i) const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        SignalDeclarationContext* signalDeclaration();

        class StatementListContext: public antlr4::ParserRuleContext {
        public:
            TSyrecParser::StatementContext* statementContext = nullptr;
            std::vector<StatementContext*>  stmts;
            StatementListContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                                   getRuleIndex() const override;
            [[nodiscard]] std::vector<StatementContext*>           statement() const;
            [[nodiscard]] StatementContext*                        statement(size_t i) const;
            [[nodiscard]] std::vector<antlr4::tree::TerminalNode*> STATEMENT_DELIMITER() const;
            [[nodiscard]] antlr4::tree::TerminalNode*              STATEMENT_DELIMITER(size_t i) const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        StatementListContext* statementList();

        class StatementContext: public antlr4::ParserRuleContext {
        public:
            StatementContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                  getRuleIndex() const override;
            [[nodiscard]] CallStatementContext*   callStatement() const;
            [[nodiscard]] ForStatementContext*    forStatement() const;
            [[nodiscard]] IfStatementContext*     ifStatement() const;
            [[nodiscard]] UnaryStatementContext*  unaryStatement() const;
            [[nodiscard]] AssignStatementContext* assignStatement() const;
            [[nodiscard]] SwapStatementContext*   swapStatement() const;
            [[nodiscard]] SkipStatementContext*   skipStatement() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        StatementContext* statement();

        class CallStatementContext: public antlr4::ParserRuleContext {
        public:
            antlr4::Token*              moduleIdent = nullptr;
            antlr4::Token*              identToken  = nullptr;
            std::vector<antlr4::Token*> callerArguments;
            CallStatementContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                                   getRuleIndex() const override;
            [[nodiscard]] antlr4::tree::TerminalNode*              OPEN_RBRACKET() const;
            [[nodiscard]] antlr4::tree::TerminalNode*              CLOSE_RBRACKET() const;
            [[nodiscard]] antlr4::tree::TerminalNode*              OP_CALL() const;
            [[nodiscard]] antlr4::tree::TerminalNode*              OP_UNCALL() const;
            [[nodiscard]] std::vector<antlr4::tree::TerminalNode*> IDENT() const;
            [[nodiscard]] antlr4::tree::TerminalNode*              IDENT(size_t i) const;
            [[nodiscard]] std::vector<antlr4::tree::TerminalNode*> PARAMETER_DELIMITER() const;
            [[nodiscard]] antlr4::tree::TerminalNode*              PARAMETER_DELIMITER(size_t i) const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        CallStatementContext* callStatement();

        class LoopVariableDefinitionContext: public antlr4::ParserRuleContext {
        public:
            antlr4::Token* variableIdent = nullptr;
            LoopVariableDefinitionContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                      getRuleIndex() const override;
            [[nodiscard]] antlr4::tree::TerminalNode* LOOP_VARIABLE_PREFIX() const;
            [[nodiscard]] antlr4::tree::TerminalNode* OP_EQUAL() const;
            [[nodiscard]] antlr4::tree::TerminalNode* IDENT() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        LoopVariableDefinitionContext* loopVariableDefinition();

        class LoopStepsizeDefinitionContext: public antlr4::ParserRuleContext {
        public:
            LoopStepsizeDefinitionContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                      getRuleIndex() const override;
            [[nodiscard]] antlr4::tree::TerminalNode* KEYWORD_STEP() const;
            [[nodiscard]] NumberContext*              number() const;
            [[nodiscard]] antlr4::tree::TerminalNode* OP_MINUS() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        LoopStepsizeDefinitionContext* loopStepsizeDefinition();

        class ForStatementContext: public antlr4::ParserRuleContext {
        public:
            TSyrecParser::NumberContext* startValue = nullptr;
            TSyrecParser::NumberContext* endValue   = nullptr;
            ForStatementContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                         getRuleIndex() const override;
            [[nodiscard]] antlr4::tree::TerminalNode*    KEYWORD_FOR() const;
            [[nodiscard]] antlr4::tree::TerminalNode*    KEYWORD_DO() const;
            [[nodiscard]] StatementListContext*          statementList() const;
            [[nodiscard]] antlr4::tree::TerminalNode*    KEYWORD_ROF() const;
            [[nodiscard]] std::vector<NumberContext*>    number() const;
            [[nodiscard]] NumberContext*                 number(size_t i) const;
            [[nodiscard]] antlr4::tree::TerminalNode*    KEYWORD_TO() const;
            [[nodiscard]] LoopStepsizeDefinitionContext* loopStepsizeDefinition() const;
            [[nodiscard]] LoopVariableDefinitionContext* loopVariableDefinition() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        ForStatementContext* forStatement();

        class IfStatementContext: public antlr4::ParserRuleContext {
        public:
            TSyrecParser::ExpressionContext*    guardCondition          = nullptr;
            TSyrecParser::StatementListContext* trueBranchStmts         = nullptr;
            TSyrecParser::StatementListContext* falseBranchStmts        = nullptr;
            TSyrecParser::ExpressionContext*    matchingGuardExpression = nullptr;
            IfStatementContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                             getRuleIndex() const override;
            [[nodiscard]] antlr4::tree::TerminalNode*        KEYWORD_IF() const;
            [[nodiscard]] antlr4::tree::TerminalNode*        KEYWORD_THEN() const;
            [[nodiscard]] antlr4::tree::TerminalNode*        KEYWORD_ELSE() const;
            [[nodiscard]] antlr4::tree::TerminalNode*        KEYWORD_FI() const;
            [[nodiscard]] std::vector<ExpressionContext*>    expression() const;
            [[nodiscard]] ExpressionContext*                 expression(size_t i) const;
            [[nodiscard]] std::vector<StatementListContext*> statementList() const;
            [[nodiscard]] StatementListContext*              statementList(size_t i) const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        IfStatementContext* ifStatement();

        class UnaryStatementContext: public antlr4::ParserRuleContext {
        public:
            antlr4::Token* unaryOp = nullptr;
            UnaryStatementContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                      getRuleIndex() const override;
            [[nodiscard]] SignalContext*              signal() const;
            [[nodiscard]] antlr4::tree::TerminalNode* OP_INVERT_ASSIGN() const;
            [[nodiscard]] antlr4::tree::TerminalNode* OP_INCREMENT_ASSIGN() const;
            [[nodiscard]] antlr4::tree::TerminalNode* OP_DECREMENT_ASSIGN() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        UnaryStatementContext* unaryStatement();

        class AssignStatementContext: public antlr4::ParserRuleContext {
        public:
            antlr4::Token* assignmentOp = nullptr;
            AssignStatementContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                      getRuleIndex() const override;
            [[nodiscard]] SignalContext*              signal() const;
            [[nodiscard]] ExpressionContext*          expression() const;
            [[nodiscard]] antlr4::tree::TerminalNode* OP_ADD_ASSIGN() const;
            [[nodiscard]] antlr4::tree::TerminalNode* OP_SUB_ASSIGN() const;
            [[nodiscard]] antlr4::tree::TerminalNode* OP_XOR_ASSIGN() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        AssignStatementContext* assignStatement();

        class SwapStatementContext: public antlr4::ParserRuleContext {
        public:
            TSyrecParser::SignalContext* lhsOperand = nullptr;
            TSyrecParser::SignalContext* rhsOperand = nullptr;
            SwapStatementContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                      getRuleIndex() const override;
            [[nodiscard]] antlr4::tree::TerminalNode* OP_SWAP() const;
            [[nodiscard]] std::vector<SignalContext*> signal() const;
            [[nodiscard]] SignalContext*              signal(size_t i) const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        SwapStatementContext* swapStatement();

        class SkipStatementContext: public antlr4::ParserRuleContext {
        public:
            SkipStatementContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                      getRuleIndex() const override;
            [[nodiscard]] antlr4::tree::TerminalNode* KEYWORD_SKIP() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        SkipStatementContext* skipStatement();

        class SignalContext: public antlr4::ParserRuleContext {
        public:
            TSyrecParser::ExpressionContext* expressionContext = nullptr;
            std::vector<ExpressionContext*>  accessedDimensions;
            TSyrecParser::NumberContext*     bitStart    = nullptr;
            TSyrecParser::NumberContext*     bitRangeEnd = nullptr;
            SignalContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                                   getRuleIndex() const override;
            [[nodiscard]] antlr4::tree::TerminalNode*              IDENT() const;
            [[nodiscard]] std::vector<antlr4::tree::TerminalNode*> OPEN_SBRACKET() const;
            [[nodiscard]] antlr4::tree::TerminalNode*              OPEN_SBRACKET(size_t i) const;
            [[nodiscard]] std::vector<antlr4::tree::TerminalNode*> CLOSE_SBRACKET() const;
            [[nodiscard]] antlr4::tree::TerminalNode*              CLOSE_SBRACKET(size_t i) const;
            [[nodiscard]] antlr4::tree::TerminalNode*              BITRANGE_START_PREFIX() const;
            [[nodiscard]] std::vector<ExpressionContext*>          expression() const;
            [[nodiscard]] ExpressionContext*                       expression(size_t i) const;
            [[nodiscard]] std::vector<NumberContext*>              number() const;
            [[nodiscard]] NumberContext*                           number(size_t i) const;
            [[nodiscard]] antlr4::tree::TerminalNode*              BITRANGE_END_PREFIX() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        SignalContext* signal();

        class ExpressionContext: public antlr4::ParserRuleContext {
        public:
            ExpressionContext(antlr4::ParserRuleContext* parent, size_t invokingState);

            ExpressionContext() = default;
            void copyFrom(ExpressionContext* ctx);
            using antlr4::ParserRuleContext::copyFrom;

            [[nodiscard]] size_t getRuleIndex() const override;
        };

        class ExpressionFromSignalContext: public ExpressionContext {
        public:
            explicit ExpressionFromSignalContext(ExpressionContext* ctx);

            [[nodiscard]] SignalContext* signal() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        class ExpressionFromBinaryExpressionContext: public ExpressionContext {
        public:
            explicit ExpressionFromBinaryExpressionContext(ExpressionContext* ctx);

            [[nodiscard]] BinaryExpressionContext* binaryExpression() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        class ExpressionFromNumberContext: public ExpressionContext {
        public:
            explicit ExpressionFromNumberContext(ExpressionContext* ctx);

            [[nodiscard]] NumberContext* number() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        class ExpressionFromUnaryExpressionContext: public ExpressionContext {
        public:
            explicit ExpressionFromUnaryExpressionContext(ExpressionContext* ctx);

            [[nodiscard]] UnaryExpressionContext* unaryExpression() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        class ExpressionFromShiftExpressionContext: public ExpressionContext {
        public:
            explicit ExpressionFromShiftExpressionContext(ExpressionContext* ctx);

            [[nodiscard]] ShiftExpressionContext* shiftExpression() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        ExpressionContext* expression();

        class BinaryExpressionContext: public antlr4::ParserRuleContext {
        public:
            TSyrecParser::ExpressionContext* lhsOperand      = nullptr;
            antlr4::Token*                   binaryOperation = nullptr;
            TSyrecParser::ExpressionContext* rhsOperand      = nullptr;
            BinaryExpressionContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                          getRuleIndex() const override;
            [[nodiscard]] antlr4::tree::TerminalNode*     OPEN_RBRACKET() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     CLOSE_RBRACKET() const;
            [[nodiscard]] std::vector<ExpressionContext*> expression() const;
            [[nodiscard]] ExpressionContext*              expression(size_t i) const;
            [[nodiscard]] antlr4::tree::TerminalNode*     OP_PLUS() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     OP_MINUS() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     OP_MULTIPLY() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     OP_DIVISION() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     OP_MODULO() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     OP_UPPER_BIT_MULTIPLY() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     OP_LOGICAL_AND() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     OP_LOGICAL_OR() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     OP_BITWISE_AND() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     OP_BITWISE_OR() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     OP_BITWISE_XOR() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     OP_LESS_THAN() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     OP_GREATER_THAN() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     OP_EQUAL() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     OP_NOT_EQUAL() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     OP_LESS_OR_EQUAL() const;
            [[nodiscard]] antlr4::tree::TerminalNode*     OP_GREATER_OR_EQUAL() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        BinaryExpressionContext* binaryExpression();

        class UnaryExpressionContext: public antlr4::ParserRuleContext {
        public:
            antlr4::Token* unaryOperation = nullptr;
            UnaryExpressionContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                      getRuleIndex() const override;
            [[nodiscard]] ExpressionContext*          expression() const;
            [[nodiscard]] antlr4::tree::TerminalNode* OP_LOGICAL_NEGATION() const;
            [[nodiscard]] antlr4::tree::TerminalNode* OP_BITWISE_NEGATION() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        UnaryExpressionContext* unaryExpression();

        class ShiftExpressionContext: public antlr4::ParserRuleContext {
        public:
            antlr4::Token* shiftOperation = nullptr;
            ShiftExpressionContext(antlr4::ParserRuleContext* parent, size_t invokingState);
            [[nodiscard]] size_t                      getRuleIndex() const override;
            [[nodiscard]] antlr4::tree::TerminalNode* OPEN_RBRACKET() const;
            [[nodiscard]] ExpressionContext*          expression() const;
            [[nodiscard]] NumberContext*              number() const;
            [[nodiscard]] antlr4::tree::TerminalNode* CLOSE_RBRACKET() const;
            [[nodiscard]] antlr4::tree::TerminalNode* OP_RIGHT_SHIFT() const;
            [[nodiscard]] antlr4::tree::TerminalNode* OP_LEFT_SHIFT() const;

            std::any accept(antlr4::tree::ParseTreeVisitor* visitor) override;
        };

        ShiftExpressionContext* shiftExpression();

        // By default the static state used to implement the parser is lazily initialized during the first
        // call to the constructor. You can call this function if you wish to initialize the static state
        // ahead of time.
        static void initialize();
    };
} // namespace syrec_parser
#endif
