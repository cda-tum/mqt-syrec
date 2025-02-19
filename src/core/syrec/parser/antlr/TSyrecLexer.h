
// Generated from C:/School/MThesis/mqt-syrec-antlr/src/core/syrec/parser/antlr/grammar/TSyrecLexer.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"


namespace syrec_parser {


class  TSyrecLexer : public antlr4::Lexer {
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

  explicit TSyrecLexer(antlr4::CharStream *input);

  ~TSyrecLexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};

}  // namespace syrec_parser
