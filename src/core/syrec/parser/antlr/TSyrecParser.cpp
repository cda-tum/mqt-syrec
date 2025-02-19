
// Generated from C:/School/MThesis/mqt-syrec-antlr/src/core/syrec/parser/antlr/grammar/TSyrecParser.g4 by ANTLR 4.13.2


#include "TSyrecParserVisitor.h"

#include "TSyrecParser.h"


using namespace antlrcpp;
using namespace syrec_parser;

using namespace antlr4;

namespace {

struct TSyrecParserStaticData final {
  TSyrecParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  TSyrecParserStaticData(const TSyrecParserStaticData&) = delete;
  TSyrecParserStaticData(TSyrecParserStaticData&&) = delete;
  TSyrecParserStaticData& operator=(const TSyrecParserStaticData&) = delete;
  TSyrecParserStaticData& operator=(TSyrecParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag tsyrecparserParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<TSyrecParserStaticData> tsyrecparserParserStaticData = nullptr;

void tsyrecparserParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (tsyrecparserParserStaticData != nullptr) {
    return;
  }
#else
  assert(tsyrecparserParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<TSyrecParserStaticData>(
    std::vector<std::string>{
      "number", "program", "module", "parameterList", "parameter", "signalList", 
      "signalDeclaration", "statementList", "statement", "callStatement", 
      "loopVariableDefinition", "loopStepsizeDefinition", "forStatement", 
      "ifStatement", "unaryStatement", "assignStatement", "swapStatement", 
      "skipStatement", "signal", "expression", "binaryExpression", "unaryExpression", 
      "shiftExpression"
    },
    std::vector<std::string>{
      "", "'++='", "'--='", "'~='", "'+='", "'-='", "'^='", "'+'", "'-'", 
      "'*'", "'*>'", "'/'", "'%'", "'<<'", "'>>'", "'<=>'", "'>='", "'<='", 
      "'>'", "'<'", "'='", "'!='", "'&&'", "'||'", "'!'", "'&'", "'~'", 
      "'|'", "'^'", "'call'", "'uncall'", "'in'", "'out'", "'inout'", "'wire'", 
      "'state'", "'$'", "'#'", "';'", "','", "'('", "')'", "'['", "']'", 
      "'module'", "'for'", "'do'", "'to'", "'step'", "'rof'", "'if'", "'then'", 
      "'else'", "'fi'", "'skip'", "'.'", "':'"
    },
    std::vector<std::string>{
      "", "OP_INCREMENT_ASSIGN", "OP_DECREMENT_ASSIGN", "OP_INVERT_ASSIGN", 
      "OP_ADD_ASSIGN", "OP_SUB_ASSIGN", "OP_XOR_ASSIGN", "OP_PLUS", "OP_MINUS", 
      "OP_MULTIPLY", "OP_UPPER_BIT_MULTIPLY", "OP_DIVISION", "OP_MODULO", 
      "OP_LEFT_SHIFT", "OP_RIGHT_SHIFT", "OP_SWAP", "OP_GREATER_OR_EQUAL", 
      "OP_LESS_OR_EQUAL", "OP_GREATER_THAN", "OP_LESS_THAN", "OP_EQUAL", 
      "OP_NOT_EQUAL", "OP_LOGICAL_AND", "OP_LOGICAL_OR", "OP_LOGICAL_NEGATION", 
      "OP_BITWISE_AND", "OP_BITWISE_NEGATION", "OP_BITWISE_OR", "OP_BITWISE_XOR", 
      "OP_CALL", "OP_UNCALL", "VAR_TYPE_IN", "VAR_TYPE_OUT", "VAR_TYPE_INOUT", 
      "VAR_TYPE_WIRE", "VAR_TYPE_STATE", "LOOP_VARIABLE_PREFIX", "SIGNAL_WIDTH_PREFIX", 
      "STATEMENT_DELIMITER", "PARAMETER_DELIMITER", "OPEN_RBRACKET", "CLOSE_RBRACKET", 
      "OPEN_SBRACKET", "CLOSE_SBRACKET", "KEYWORD_MODULE", "KEYWORD_FOR", 
      "KEYWORD_DO", "KEYWORD_TO", "KEYWORD_STEP", "KEYWORD_ROF", "KEYWORD_IF", 
      "KEYWORD_THEN", "KEYWORD_ELSE", "KEYWORD_FI", "KEYWORD_SKIP", "BITRANGE_START_PREFIX", 
      "BITRANGE_END_PREFIX", "SKIPABLEWHITSPACES", "LINE_COMMENT", "MULTI_LINE_COMMENT", 
      "IDENT", "INT"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,61,235,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,3,0,58,8,0,1,
  	1,4,1,61,8,1,11,1,12,1,62,1,1,1,1,1,2,1,2,1,2,1,2,3,2,71,8,2,1,2,1,2,
  	5,2,75,8,2,10,2,12,2,78,9,2,1,2,1,2,1,3,1,3,1,3,5,3,85,8,3,10,3,12,3,
  	88,9,3,1,4,1,4,1,4,1,5,1,5,1,5,1,5,5,5,97,8,5,10,5,12,5,100,9,5,1,6,1,
  	6,1,6,1,6,5,6,106,8,6,10,6,12,6,109,9,6,1,6,1,6,1,6,3,6,114,8,6,1,7,1,
  	7,1,7,5,7,119,8,7,10,7,12,7,122,9,7,1,8,1,8,1,8,1,8,1,8,1,8,1,8,3,8,131,
  	8,8,1,9,1,9,1,9,1,9,1,9,1,9,5,9,139,8,9,10,9,12,9,142,9,9,1,9,1,9,1,10,
  	1,10,1,10,1,10,1,11,1,11,3,11,152,8,11,1,11,1,11,1,12,1,12,3,12,158,8,
  	12,1,12,1,12,1,12,3,12,163,8,12,1,12,1,12,3,12,167,8,12,1,12,1,12,1,12,
  	1,12,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,14,1,14,1,14,1,15,
  	1,15,1,15,1,15,1,16,1,16,1,16,1,16,1,17,1,17,1,18,1,18,1,18,1,18,1,18,
  	5,18,200,8,18,10,18,12,18,203,9,18,1,18,1,18,1,18,1,18,3,18,209,8,18,
  	3,18,211,8,18,1,19,1,19,1,19,1,19,1,19,3,19,218,8,19,1,20,1,20,1,20,1,
  	20,1,20,1,20,1,21,1,21,1,21,1,22,1,22,1,22,1,22,1,22,1,22,1,22,0,0,23,
  	0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,0,9,2,
  	0,7,9,11,11,1,0,31,33,1,0,34,35,1,0,29,30,1,0,1,3,1,0,4,6,4,0,7,12,16,
  	23,25,25,27,28,2,0,24,24,26,26,1,0,13,14,240,0,57,1,0,0,0,2,60,1,0,0,
  	0,4,66,1,0,0,0,6,81,1,0,0,0,8,89,1,0,0,0,10,92,1,0,0,0,12,101,1,0,0,0,
  	14,115,1,0,0,0,16,130,1,0,0,0,18,132,1,0,0,0,20,145,1,0,0,0,22,149,1,
  	0,0,0,24,155,1,0,0,0,26,172,1,0,0,0,28,181,1,0,0,0,30,184,1,0,0,0,32,
  	188,1,0,0,0,34,192,1,0,0,0,36,194,1,0,0,0,38,217,1,0,0,0,40,219,1,0,0,
  	0,42,225,1,0,0,0,44,228,1,0,0,0,46,58,5,61,0,0,47,48,5,37,0,0,48,58,5,
  	60,0,0,49,50,5,36,0,0,50,58,5,60,0,0,51,52,5,40,0,0,52,53,3,0,0,0,53,
  	54,7,0,0,0,54,55,3,0,0,0,55,56,5,41,0,0,56,58,1,0,0,0,57,46,1,0,0,0,57,
  	47,1,0,0,0,57,49,1,0,0,0,57,51,1,0,0,0,58,1,1,0,0,0,59,61,3,4,2,0,60,
  	59,1,0,0,0,61,62,1,0,0,0,62,60,1,0,0,0,62,63,1,0,0,0,63,64,1,0,0,0,64,
  	65,5,0,0,1,65,3,1,0,0,0,66,67,5,44,0,0,67,68,5,60,0,0,68,70,5,40,0,0,
  	69,71,3,6,3,0,70,69,1,0,0,0,70,71,1,0,0,0,71,72,1,0,0,0,72,76,5,41,0,
  	0,73,75,3,10,5,0,74,73,1,0,0,0,75,78,1,0,0,0,76,74,1,0,0,0,76,77,1,0,
  	0,0,77,79,1,0,0,0,78,76,1,0,0,0,79,80,3,14,7,0,80,5,1,0,0,0,81,86,3,8,
  	4,0,82,83,5,39,0,0,83,85,3,8,4,0,84,82,1,0,0,0,85,88,1,0,0,0,86,84,1,
  	0,0,0,86,87,1,0,0,0,87,7,1,0,0,0,88,86,1,0,0,0,89,90,7,1,0,0,90,91,3,
  	12,6,0,91,9,1,0,0,0,92,93,7,2,0,0,93,98,3,12,6,0,94,95,5,39,0,0,95,97,
  	3,12,6,0,96,94,1,0,0,0,97,100,1,0,0,0,98,96,1,0,0,0,98,99,1,0,0,0,99,
  	11,1,0,0,0,100,98,1,0,0,0,101,107,5,60,0,0,102,103,5,42,0,0,103,104,5,
  	61,0,0,104,106,5,43,0,0,105,102,1,0,0,0,106,109,1,0,0,0,107,105,1,0,0,
  	0,107,108,1,0,0,0,108,113,1,0,0,0,109,107,1,0,0,0,110,111,5,40,0,0,111,
  	112,5,61,0,0,112,114,5,41,0,0,113,110,1,0,0,0,113,114,1,0,0,0,114,13,
  	1,0,0,0,115,120,3,16,8,0,116,117,5,38,0,0,117,119,3,16,8,0,118,116,1,
  	0,0,0,119,122,1,0,0,0,120,118,1,0,0,0,120,121,1,0,0,0,121,15,1,0,0,0,
  	122,120,1,0,0,0,123,131,3,18,9,0,124,131,3,24,12,0,125,131,3,26,13,0,
  	126,131,3,28,14,0,127,131,3,30,15,0,128,131,3,32,16,0,129,131,3,34,17,
  	0,130,123,1,0,0,0,130,124,1,0,0,0,130,125,1,0,0,0,130,126,1,0,0,0,130,
  	127,1,0,0,0,130,128,1,0,0,0,130,129,1,0,0,0,131,17,1,0,0,0,132,133,7,
  	3,0,0,133,134,5,60,0,0,134,135,5,40,0,0,135,140,5,60,0,0,136,137,5,39,
  	0,0,137,139,5,60,0,0,138,136,1,0,0,0,139,142,1,0,0,0,140,138,1,0,0,0,
  	140,141,1,0,0,0,141,143,1,0,0,0,142,140,1,0,0,0,143,144,5,41,0,0,144,
  	19,1,0,0,0,145,146,5,36,0,0,146,147,5,60,0,0,147,148,5,20,0,0,148,21,
  	1,0,0,0,149,151,5,48,0,0,150,152,5,8,0,0,151,150,1,0,0,0,151,152,1,0,
  	0,0,152,153,1,0,0,0,153,154,3,0,0,0,154,23,1,0,0,0,155,162,5,45,0,0,156,
  	158,3,20,10,0,157,156,1,0,0,0,157,158,1,0,0,0,158,159,1,0,0,0,159,160,
  	3,0,0,0,160,161,5,47,0,0,161,163,1,0,0,0,162,157,1,0,0,0,162,163,1,0,
  	0,0,163,164,1,0,0,0,164,166,3,0,0,0,165,167,3,22,11,0,166,165,1,0,0,0,
  	166,167,1,0,0,0,167,168,1,0,0,0,168,169,5,46,0,0,169,170,3,14,7,0,170,
  	171,5,49,0,0,171,25,1,0,0,0,172,173,5,50,0,0,173,174,3,38,19,0,174,175,
  	5,51,0,0,175,176,3,14,7,0,176,177,5,52,0,0,177,178,3,14,7,0,178,179,5,
  	53,0,0,179,180,3,38,19,0,180,27,1,0,0,0,181,182,7,4,0,0,182,183,3,36,
  	18,0,183,29,1,0,0,0,184,185,3,36,18,0,185,186,7,5,0,0,186,187,3,38,19,
  	0,187,31,1,0,0,0,188,189,3,36,18,0,189,190,5,15,0,0,190,191,3,36,18,0,
  	191,33,1,0,0,0,192,193,5,54,0,0,193,35,1,0,0,0,194,201,5,60,0,0,195,196,
  	5,42,0,0,196,197,3,38,19,0,197,198,5,43,0,0,198,200,1,0,0,0,199,195,1,
  	0,0,0,200,203,1,0,0,0,201,199,1,0,0,0,201,202,1,0,0,0,202,210,1,0,0,0,
  	203,201,1,0,0,0,204,205,5,55,0,0,205,208,3,0,0,0,206,207,5,56,0,0,207,
  	209,3,0,0,0,208,206,1,0,0,0,208,209,1,0,0,0,209,211,1,0,0,0,210,204,1,
  	0,0,0,210,211,1,0,0,0,211,37,1,0,0,0,212,218,3,0,0,0,213,218,3,36,18,
  	0,214,218,3,40,20,0,215,218,3,42,21,0,216,218,3,44,22,0,217,212,1,0,0,
  	0,217,213,1,0,0,0,217,214,1,0,0,0,217,215,1,0,0,0,217,216,1,0,0,0,218,
  	39,1,0,0,0,219,220,5,40,0,0,220,221,3,38,19,0,221,222,7,6,0,0,222,223,
  	3,38,19,0,223,224,5,41,0,0,224,41,1,0,0,0,225,226,7,7,0,0,226,227,3,38,
  	19,0,227,43,1,0,0,0,228,229,5,40,0,0,229,230,3,38,19,0,230,231,7,8,0,
  	0,231,232,3,0,0,0,232,233,5,41,0,0,233,45,1,0,0,0,19,57,62,70,76,86,98,
  	107,113,120,130,140,151,157,162,166,201,208,210,217
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  tsyrecparserParserStaticData = std::move(staticData);
}

}

TSyrecParser::TSyrecParser(TokenStream *input) : TSyrecParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

TSyrecParser::TSyrecParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  TSyrecParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *tsyrecparserParserStaticData->atn, tsyrecparserParserStaticData->decisionToDFA, tsyrecparserParserStaticData->sharedContextCache, options);
}

TSyrecParser::~TSyrecParser() {
  delete _interpreter;
}

const atn::ATN& TSyrecParser::getATN() const {
  return *tsyrecparserParserStaticData->atn;
}

std::string TSyrecParser::getGrammarFileName() const {
  return "TSyrecParser.g4";
}

const std::vector<std::string>& TSyrecParser::getRuleNames() const {
  return tsyrecparserParserStaticData->ruleNames;
}

const dfa::Vocabulary& TSyrecParser::getVocabulary() const {
  return tsyrecparserParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView TSyrecParser::getSerializedATN() const {
  return tsyrecparserParserStaticData->serializedATN;
}


//----------------- NumberContext ------------------------------------------------------------------

TSyrecParser::NumberContext::NumberContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t TSyrecParser::NumberContext::getRuleIndex() const {
  return TSyrecParser::RuleNumber;
}

void TSyrecParser::NumberContext::copyFrom(NumberContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- NumberFromSignalwidthContext ------------------------------------------------------------------

tree::TerminalNode* TSyrecParser::NumberFromSignalwidthContext::SIGNAL_WIDTH_PREFIX() {
  return getToken(TSyrecParser::SIGNAL_WIDTH_PREFIX, 0);
}

tree::TerminalNode* TSyrecParser::NumberFromSignalwidthContext::IDENT() {
  return getToken(TSyrecParser::IDENT, 0);
}

TSyrecParser::NumberFromSignalwidthContext::NumberFromSignalwidthContext(NumberContext *ctx) { copyFrom(ctx); }


std::any TSyrecParser::NumberFromSignalwidthContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitNumberFromSignalwidth(this);
  else
    return visitor->visitChildren(this);
}
//----------------- NumberFromLoopVariableContext ------------------------------------------------------------------

tree::TerminalNode* TSyrecParser::NumberFromLoopVariableContext::LOOP_VARIABLE_PREFIX() {
  return getToken(TSyrecParser::LOOP_VARIABLE_PREFIX, 0);
}

tree::TerminalNode* TSyrecParser::NumberFromLoopVariableContext::IDENT() {
  return getToken(TSyrecParser::IDENT, 0);
}

TSyrecParser::NumberFromLoopVariableContext::NumberFromLoopVariableContext(NumberContext *ctx) { copyFrom(ctx); }


std::any TSyrecParser::NumberFromLoopVariableContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitNumberFromLoopVariable(this);
  else
    return visitor->visitChildren(this);
}
//----------------- NumberFromConstantContext ------------------------------------------------------------------

tree::TerminalNode* TSyrecParser::NumberFromConstantContext::INT() {
  return getToken(TSyrecParser::INT, 0);
}

TSyrecParser::NumberFromConstantContext::NumberFromConstantContext(NumberContext *ctx) { copyFrom(ctx); }


std::any TSyrecParser::NumberFromConstantContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitNumberFromConstant(this);
  else
    return visitor->visitChildren(this);
}
//----------------- NumberFromExpressionContext ------------------------------------------------------------------

tree::TerminalNode* TSyrecParser::NumberFromExpressionContext::OPEN_RBRACKET() {
  return getToken(TSyrecParser::OPEN_RBRACKET, 0);
}

tree::TerminalNode* TSyrecParser::NumberFromExpressionContext::CLOSE_RBRACKET() {
  return getToken(TSyrecParser::CLOSE_RBRACKET, 0);
}

std::vector<TSyrecParser::NumberContext *> TSyrecParser::NumberFromExpressionContext::number() {
  return getRuleContexts<TSyrecParser::NumberContext>();
}

TSyrecParser::NumberContext* TSyrecParser::NumberFromExpressionContext::number(size_t i) {
  return getRuleContext<TSyrecParser::NumberContext>(i);
}

tree::TerminalNode* TSyrecParser::NumberFromExpressionContext::OP_PLUS() {
  return getToken(TSyrecParser::OP_PLUS, 0);
}

tree::TerminalNode* TSyrecParser::NumberFromExpressionContext::OP_MINUS() {
  return getToken(TSyrecParser::OP_MINUS, 0);
}

tree::TerminalNode* TSyrecParser::NumberFromExpressionContext::OP_MULTIPLY() {
  return getToken(TSyrecParser::OP_MULTIPLY, 0);
}

tree::TerminalNode* TSyrecParser::NumberFromExpressionContext::OP_DIVISION() {
  return getToken(TSyrecParser::OP_DIVISION, 0);
}

TSyrecParser::NumberFromExpressionContext::NumberFromExpressionContext(NumberContext *ctx) { copyFrom(ctx); }


std::any TSyrecParser::NumberFromExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitNumberFromExpression(this);
  else
    return visitor->visitChildren(this);
}
TSyrecParser::NumberContext* TSyrecParser::number() {
  NumberContext *_localctx = _tracker.createInstance<NumberContext>(_ctx, getState());
  enterRule(_localctx, 0, TSyrecParser::RuleNumber);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(57);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case TSyrecParser::INT: {
        _localctx = _tracker.createInstance<TSyrecParser::NumberFromConstantContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(46);
        match(TSyrecParser::INT);
        break;
      }

      case TSyrecParser::SIGNAL_WIDTH_PREFIX: {
        _localctx = _tracker.createInstance<TSyrecParser::NumberFromSignalwidthContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(47);
        match(TSyrecParser::SIGNAL_WIDTH_PREFIX);
        setState(48);
        match(TSyrecParser::IDENT);
        break;
      }

      case TSyrecParser::LOOP_VARIABLE_PREFIX: {
        _localctx = _tracker.createInstance<TSyrecParser::NumberFromLoopVariableContext>(_localctx);
        enterOuterAlt(_localctx, 3);
        setState(49);
        match(TSyrecParser::LOOP_VARIABLE_PREFIX);
        setState(50);
        match(TSyrecParser::IDENT);
        break;
      }

      case TSyrecParser::OPEN_RBRACKET: {
        _localctx = _tracker.createInstance<TSyrecParser::NumberFromExpressionContext>(_localctx);
        enterOuterAlt(_localctx, 4);
        setState(51);
        match(TSyrecParser::OPEN_RBRACKET);
        setState(52);
        antlrcpp::downCast<NumberFromExpressionContext *>(_localctx)->lhsOperand = number();
        setState(53);
        antlrcpp::downCast<NumberFromExpressionContext *>(_localctx)->op = _input->LT(1);
        _la = _input->LA(1);
        if (!((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 2944) != 0))) {
          antlrcpp::downCast<NumberFromExpressionContext *>(_localctx)->op = _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(54);
        antlrcpp::downCast<NumberFromExpressionContext *>(_localctx)->rhsOperand = number();
        setState(55);
        match(TSyrecParser::CLOSE_RBRACKET);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ProgramContext ------------------------------------------------------------------

TSyrecParser::ProgramContext::ProgramContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* TSyrecParser::ProgramContext::EOF() {
  return getToken(TSyrecParser::EOF, 0);
}

std::vector<TSyrecParser::ModuleContext *> TSyrecParser::ProgramContext::module() {
  return getRuleContexts<TSyrecParser::ModuleContext>();
}

TSyrecParser::ModuleContext* TSyrecParser::ProgramContext::module(size_t i) {
  return getRuleContext<TSyrecParser::ModuleContext>(i);
}


size_t TSyrecParser::ProgramContext::getRuleIndex() const {
  return TSyrecParser::RuleProgram;
}


std::any TSyrecParser::ProgramContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitProgram(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::ProgramContext* TSyrecParser::program() {
  ProgramContext *_localctx = _tracker.createInstance<ProgramContext>(_ctx, getState());
  enterRule(_localctx, 2, TSyrecParser::RuleProgram);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(60); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(59);
      module();
      setState(62); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while (_la == TSyrecParser::KEYWORD_MODULE);
    setState(64);
    match(TSyrecParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ModuleContext ------------------------------------------------------------------

TSyrecParser::ModuleContext::ModuleContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* TSyrecParser::ModuleContext::KEYWORD_MODULE() {
  return getToken(TSyrecParser::KEYWORD_MODULE, 0);
}

tree::TerminalNode* TSyrecParser::ModuleContext::IDENT() {
  return getToken(TSyrecParser::IDENT, 0);
}

tree::TerminalNode* TSyrecParser::ModuleContext::OPEN_RBRACKET() {
  return getToken(TSyrecParser::OPEN_RBRACKET, 0);
}

tree::TerminalNode* TSyrecParser::ModuleContext::CLOSE_RBRACKET() {
  return getToken(TSyrecParser::CLOSE_RBRACKET, 0);
}

TSyrecParser::StatementListContext* TSyrecParser::ModuleContext::statementList() {
  return getRuleContext<TSyrecParser::StatementListContext>(0);
}

TSyrecParser::ParameterListContext* TSyrecParser::ModuleContext::parameterList() {
  return getRuleContext<TSyrecParser::ParameterListContext>(0);
}

std::vector<TSyrecParser::SignalListContext *> TSyrecParser::ModuleContext::signalList() {
  return getRuleContexts<TSyrecParser::SignalListContext>();
}

TSyrecParser::SignalListContext* TSyrecParser::ModuleContext::signalList(size_t i) {
  return getRuleContext<TSyrecParser::SignalListContext>(i);
}


size_t TSyrecParser::ModuleContext::getRuleIndex() const {
  return TSyrecParser::RuleModule;
}


std::any TSyrecParser::ModuleContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitModule(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::ModuleContext* TSyrecParser::module() {
  ModuleContext *_localctx = _tracker.createInstance<ModuleContext>(_ctx, getState());
  enterRule(_localctx, 4, TSyrecParser::RuleModule);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(66);
    match(TSyrecParser::KEYWORD_MODULE);
    setState(67);
    match(TSyrecParser::IDENT);
    setState(68);
    match(TSyrecParser::OPEN_RBRACKET);
    setState(70);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 15032385536) != 0)) {
      setState(69);
      parameterList();
    }
    setState(72);
    match(TSyrecParser::CLOSE_RBRACKET);
    setState(76);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == TSyrecParser::VAR_TYPE_WIRE

    || _la == TSyrecParser::VAR_TYPE_STATE) {
      setState(73);
      signalList();
      setState(78);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(79);
    statementList();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParameterListContext ------------------------------------------------------------------

TSyrecParser::ParameterListContext::ParameterListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<TSyrecParser::ParameterContext *> TSyrecParser::ParameterListContext::parameter() {
  return getRuleContexts<TSyrecParser::ParameterContext>();
}

TSyrecParser::ParameterContext* TSyrecParser::ParameterListContext::parameter(size_t i) {
  return getRuleContext<TSyrecParser::ParameterContext>(i);
}

std::vector<tree::TerminalNode *> TSyrecParser::ParameterListContext::PARAMETER_DELIMITER() {
  return getTokens(TSyrecParser::PARAMETER_DELIMITER);
}

tree::TerminalNode* TSyrecParser::ParameterListContext::PARAMETER_DELIMITER(size_t i) {
  return getToken(TSyrecParser::PARAMETER_DELIMITER, i);
}


size_t TSyrecParser::ParameterListContext::getRuleIndex() const {
  return TSyrecParser::RuleParameterList;
}


std::any TSyrecParser::ParameterListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitParameterList(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::ParameterListContext* TSyrecParser::parameterList() {
  ParameterListContext *_localctx = _tracker.createInstance<ParameterListContext>(_ctx, getState());
  enterRule(_localctx, 6, TSyrecParser::RuleParameterList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(81);
    parameter();
    setState(86);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == TSyrecParser::PARAMETER_DELIMITER) {
      setState(82);
      match(TSyrecParser::PARAMETER_DELIMITER);
      setState(83);
      parameter();
      setState(88);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParameterContext ------------------------------------------------------------------

TSyrecParser::ParameterContext::ParameterContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

TSyrecParser::SignalDeclarationContext* TSyrecParser::ParameterContext::signalDeclaration() {
  return getRuleContext<TSyrecParser::SignalDeclarationContext>(0);
}

tree::TerminalNode* TSyrecParser::ParameterContext::VAR_TYPE_IN() {
  return getToken(TSyrecParser::VAR_TYPE_IN, 0);
}

tree::TerminalNode* TSyrecParser::ParameterContext::VAR_TYPE_OUT() {
  return getToken(TSyrecParser::VAR_TYPE_OUT, 0);
}

tree::TerminalNode* TSyrecParser::ParameterContext::VAR_TYPE_INOUT() {
  return getToken(TSyrecParser::VAR_TYPE_INOUT, 0);
}


size_t TSyrecParser::ParameterContext::getRuleIndex() const {
  return TSyrecParser::RuleParameter;
}


std::any TSyrecParser::ParameterContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitParameter(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::ParameterContext* TSyrecParser::parameter() {
  ParameterContext *_localctx = _tracker.createInstance<ParameterContext>(_ctx, getState());
  enterRule(_localctx, 8, TSyrecParser::RuleParameter);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(89);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 15032385536) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(90);
    signalDeclaration();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SignalListContext ------------------------------------------------------------------

TSyrecParser::SignalListContext::SignalListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<TSyrecParser::SignalDeclarationContext *> TSyrecParser::SignalListContext::signalDeclaration() {
  return getRuleContexts<TSyrecParser::SignalDeclarationContext>();
}

TSyrecParser::SignalDeclarationContext* TSyrecParser::SignalListContext::signalDeclaration(size_t i) {
  return getRuleContext<TSyrecParser::SignalDeclarationContext>(i);
}

tree::TerminalNode* TSyrecParser::SignalListContext::VAR_TYPE_WIRE() {
  return getToken(TSyrecParser::VAR_TYPE_WIRE, 0);
}

tree::TerminalNode* TSyrecParser::SignalListContext::VAR_TYPE_STATE() {
  return getToken(TSyrecParser::VAR_TYPE_STATE, 0);
}

std::vector<tree::TerminalNode *> TSyrecParser::SignalListContext::PARAMETER_DELIMITER() {
  return getTokens(TSyrecParser::PARAMETER_DELIMITER);
}

tree::TerminalNode* TSyrecParser::SignalListContext::PARAMETER_DELIMITER(size_t i) {
  return getToken(TSyrecParser::PARAMETER_DELIMITER, i);
}


size_t TSyrecParser::SignalListContext::getRuleIndex() const {
  return TSyrecParser::RuleSignalList;
}


std::any TSyrecParser::SignalListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitSignalList(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::SignalListContext* TSyrecParser::signalList() {
  SignalListContext *_localctx = _tracker.createInstance<SignalListContext>(_ctx, getState());
  enterRule(_localctx, 10, TSyrecParser::RuleSignalList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(92);
    _la = _input->LA(1);
    if (!(_la == TSyrecParser::VAR_TYPE_WIRE

    || _la == TSyrecParser::VAR_TYPE_STATE)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(93);
    signalDeclaration();
    setState(98);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == TSyrecParser::PARAMETER_DELIMITER) {
      setState(94);
      match(TSyrecParser::PARAMETER_DELIMITER);
      setState(95);
      signalDeclaration();
      setState(100);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SignalDeclarationContext ------------------------------------------------------------------

TSyrecParser::SignalDeclarationContext::SignalDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* TSyrecParser::SignalDeclarationContext::IDENT() {
  return getToken(TSyrecParser::IDENT, 0);
}

std::vector<tree::TerminalNode *> TSyrecParser::SignalDeclarationContext::OPEN_SBRACKET() {
  return getTokens(TSyrecParser::OPEN_SBRACKET);
}

tree::TerminalNode* TSyrecParser::SignalDeclarationContext::OPEN_SBRACKET(size_t i) {
  return getToken(TSyrecParser::OPEN_SBRACKET, i);
}

std::vector<tree::TerminalNode *> TSyrecParser::SignalDeclarationContext::CLOSE_SBRACKET() {
  return getTokens(TSyrecParser::CLOSE_SBRACKET);
}

tree::TerminalNode* TSyrecParser::SignalDeclarationContext::CLOSE_SBRACKET(size_t i) {
  return getToken(TSyrecParser::CLOSE_SBRACKET, i);
}

tree::TerminalNode* TSyrecParser::SignalDeclarationContext::OPEN_RBRACKET() {
  return getToken(TSyrecParser::OPEN_RBRACKET, 0);
}

tree::TerminalNode* TSyrecParser::SignalDeclarationContext::CLOSE_RBRACKET() {
  return getToken(TSyrecParser::CLOSE_RBRACKET, 0);
}

std::vector<tree::TerminalNode *> TSyrecParser::SignalDeclarationContext::INT() {
  return getTokens(TSyrecParser::INT);
}

tree::TerminalNode* TSyrecParser::SignalDeclarationContext::INT(size_t i) {
  return getToken(TSyrecParser::INT, i);
}


size_t TSyrecParser::SignalDeclarationContext::getRuleIndex() const {
  return TSyrecParser::RuleSignalDeclaration;
}


std::any TSyrecParser::SignalDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitSignalDeclaration(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::SignalDeclarationContext* TSyrecParser::signalDeclaration() {
  SignalDeclarationContext *_localctx = _tracker.createInstance<SignalDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 12, TSyrecParser::RuleSignalDeclaration);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(101);
    match(TSyrecParser::IDENT);
    setState(107);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == TSyrecParser::OPEN_SBRACKET) {
      setState(102);
      match(TSyrecParser::OPEN_SBRACKET);
      setState(103);
      antlrcpp::downCast<SignalDeclarationContext *>(_localctx)->intToken = match(TSyrecParser::INT);
      antlrcpp::downCast<SignalDeclarationContext *>(_localctx)->dimensionTokens.push_back(antlrcpp::downCast<SignalDeclarationContext *>(_localctx)->intToken);
      setState(104);
      match(TSyrecParser::CLOSE_SBRACKET);
      setState(109);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(113);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == TSyrecParser::OPEN_RBRACKET) {
      setState(110);
      match(TSyrecParser::OPEN_RBRACKET);
      setState(111);
      antlrcpp::downCast<SignalDeclarationContext *>(_localctx)->signalWidthToken = match(TSyrecParser::INT);
      setState(112);
      match(TSyrecParser::CLOSE_RBRACKET);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StatementListContext ------------------------------------------------------------------

TSyrecParser::StatementListContext::StatementListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<TSyrecParser::StatementContext *> TSyrecParser::StatementListContext::statement() {
  return getRuleContexts<TSyrecParser::StatementContext>();
}

TSyrecParser::StatementContext* TSyrecParser::StatementListContext::statement(size_t i) {
  return getRuleContext<TSyrecParser::StatementContext>(i);
}

std::vector<tree::TerminalNode *> TSyrecParser::StatementListContext::STATEMENT_DELIMITER() {
  return getTokens(TSyrecParser::STATEMENT_DELIMITER);
}

tree::TerminalNode* TSyrecParser::StatementListContext::STATEMENT_DELIMITER(size_t i) {
  return getToken(TSyrecParser::STATEMENT_DELIMITER, i);
}


size_t TSyrecParser::StatementListContext::getRuleIndex() const {
  return TSyrecParser::RuleStatementList;
}


std::any TSyrecParser::StatementListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitStatementList(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::StatementListContext* TSyrecParser::statementList() {
  StatementListContext *_localctx = _tracker.createInstance<StatementListContext>(_ctx, getState());
  enterRule(_localctx, 14, TSyrecParser::RuleStatementList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(115);
    antlrcpp::downCast<StatementListContext *>(_localctx)->statementContext = statement();
    antlrcpp::downCast<StatementListContext *>(_localctx)->stmts.push_back(antlrcpp::downCast<StatementListContext *>(_localctx)->statementContext);
    setState(120);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == TSyrecParser::STATEMENT_DELIMITER) {
      setState(116);
      match(TSyrecParser::STATEMENT_DELIMITER);
      setState(117);
      antlrcpp::downCast<StatementListContext *>(_localctx)->statementContext = statement();
      antlrcpp::downCast<StatementListContext *>(_localctx)->stmts.push_back(antlrcpp::downCast<StatementListContext *>(_localctx)->statementContext);
      setState(122);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StatementContext ------------------------------------------------------------------

TSyrecParser::StatementContext::StatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

TSyrecParser::CallStatementContext* TSyrecParser::StatementContext::callStatement() {
  return getRuleContext<TSyrecParser::CallStatementContext>(0);
}

TSyrecParser::ForStatementContext* TSyrecParser::StatementContext::forStatement() {
  return getRuleContext<TSyrecParser::ForStatementContext>(0);
}

TSyrecParser::IfStatementContext* TSyrecParser::StatementContext::ifStatement() {
  return getRuleContext<TSyrecParser::IfStatementContext>(0);
}

TSyrecParser::UnaryStatementContext* TSyrecParser::StatementContext::unaryStatement() {
  return getRuleContext<TSyrecParser::UnaryStatementContext>(0);
}

TSyrecParser::AssignStatementContext* TSyrecParser::StatementContext::assignStatement() {
  return getRuleContext<TSyrecParser::AssignStatementContext>(0);
}

TSyrecParser::SwapStatementContext* TSyrecParser::StatementContext::swapStatement() {
  return getRuleContext<TSyrecParser::SwapStatementContext>(0);
}

TSyrecParser::SkipStatementContext* TSyrecParser::StatementContext::skipStatement() {
  return getRuleContext<TSyrecParser::SkipStatementContext>(0);
}


size_t TSyrecParser::StatementContext::getRuleIndex() const {
  return TSyrecParser::RuleStatement;
}


std::any TSyrecParser::StatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitStatement(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::StatementContext* TSyrecParser::statement() {
  StatementContext *_localctx = _tracker.createInstance<StatementContext>(_ctx, getState());
  enterRule(_localctx, 16, TSyrecParser::RuleStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(130);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 9, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(123);
      callStatement();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(124);
      forStatement();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(125);
      ifStatement();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(126);
      unaryStatement();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(127);
      assignStatement();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(128);
      swapStatement();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(129);
      skipStatement();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CallStatementContext ------------------------------------------------------------------

TSyrecParser::CallStatementContext::CallStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* TSyrecParser::CallStatementContext::OPEN_RBRACKET() {
  return getToken(TSyrecParser::OPEN_RBRACKET, 0);
}

tree::TerminalNode* TSyrecParser::CallStatementContext::CLOSE_RBRACKET() {
  return getToken(TSyrecParser::CLOSE_RBRACKET, 0);
}

tree::TerminalNode* TSyrecParser::CallStatementContext::OP_CALL() {
  return getToken(TSyrecParser::OP_CALL, 0);
}

tree::TerminalNode* TSyrecParser::CallStatementContext::OP_UNCALL() {
  return getToken(TSyrecParser::OP_UNCALL, 0);
}

std::vector<tree::TerminalNode *> TSyrecParser::CallStatementContext::IDENT() {
  return getTokens(TSyrecParser::IDENT);
}

tree::TerminalNode* TSyrecParser::CallStatementContext::IDENT(size_t i) {
  return getToken(TSyrecParser::IDENT, i);
}

std::vector<tree::TerminalNode *> TSyrecParser::CallStatementContext::PARAMETER_DELIMITER() {
  return getTokens(TSyrecParser::PARAMETER_DELIMITER);
}

tree::TerminalNode* TSyrecParser::CallStatementContext::PARAMETER_DELIMITER(size_t i) {
  return getToken(TSyrecParser::PARAMETER_DELIMITER, i);
}


size_t TSyrecParser::CallStatementContext::getRuleIndex() const {
  return TSyrecParser::RuleCallStatement;
}


std::any TSyrecParser::CallStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitCallStatement(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::CallStatementContext* TSyrecParser::callStatement() {
  CallStatementContext *_localctx = _tracker.createInstance<CallStatementContext>(_ctx, getState());
  enterRule(_localctx, 18, TSyrecParser::RuleCallStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(132);
    _la = _input->LA(1);
    if (!(_la == TSyrecParser::OP_CALL

    || _la == TSyrecParser::OP_UNCALL)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(133);
    antlrcpp::downCast<CallStatementContext *>(_localctx)->moduleIdent = match(TSyrecParser::IDENT);
    setState(134);
    match(TSyrecParser::OPEN_RBRACKET);
    setState(135);
    antlrcpp::downCast<CallStatementContext *>(_localctx)->identToken = match(TSyrecParser::IDENT);
    antlrcpp::downCast<CallStatementContext *>(_localctx)->callerArguments.push_back(antlrcpp::downCast<CallStatementContext *>(_localctx)->identToken);
    setState(140);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == TSyrecParser::PARAMETER_DELIMITER) {
      setState(136);
      match(TSyrecParser::PARAMETER_DELIMITER);
      setState(137);
      antlrcpp::downCast<CallStatementContext *>(_localctx)->identToken = match(TSyrecParser::IDENT);
      antlrcpp::downCast<CallStatementContext *>(_localctx)->callerArguments.push_back(antlrcpp::downCast<CallStatementContext *>(_localctx)->identToken);
      setState(142);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(143);
    match(TSyrecParser::CLOSE_RBRACKET);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LoopVariableDefinitionContext ------------------------------------------------------------------

TSyrecParser::LoopVariableDefinitionContext::LoopVariableDefinitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* TSyrecParser::LoopVariableDefinitionContext::LOOP_VARIABLE_PREFIX() {
  return getToken(TSyrecParser::LOOP_VARIABLE_PREFIX, 0);
}

tree::TerminalNode* TSyrecParser::LoopVariableDefinitionContext::OP_EQUAL() {
  return getToken(TSyrecParser::OP_EQUAL, 0);
}

tree::TerminalNode* TSyrecParser::LoopVariableDefinitionContext::IDENT() {
  return getToken(TSyrecParser::IDENT, 0);
}


size_t TSyrecParser::LoopVariableDefinitionContext::getRuleIndex() const {
  return TSyrecParser::RuleLoopVariableDefinition;
}


std::any TSyrecParser::LoopVariableDefinitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitLoopVariableDefinition(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::LoopVariableDefinitionContext* TSyrecParser::loopVariableDefinition() {
  LoopVariableDefinitionContext *_localctx = _tracker.createInstance<LoopVariableDefinitionContext>(_ctx, getState());
  enterRule(_localctx, 20, TSyrecParser::RuleLoopVariableDefinition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(145);
    match(TSyrecParser::LOOP_VARIABLE_PREFIX);
    setState(146);
    antlrcpp::downCast<LoopVariableDefinitionContext *>(_localctx)->variableIdent = match(TSyrecParser::IDENT);
    setState(147);
    match(TSyrecParser::OP_EQUAL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LoopStepsizeDefinitionContext ------------------------------------------------------------------

TSyrecParser::LoopStepsizeDefinitionContext::LoopStepsizeDefinitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* TSyrecParser::LoopStepsizeDefinitionContext::KEYWORD_STEP() {
  return getToken(TSyrecParser::KEYWORD_STEP, 0);
}

TSyrecParser::NumberContext* TSyrecParser::LoopStepsizeDefinitionContext::number() {
  return getRuleContext<TSyrecParser::NumberContext>(0);
}

tree::TerminalNode* TSyrecParser::LoopStepsizeDefinitionContext::OP_MINUS() {
  return getToken(TSyrecParser::OP_MINUS, 0);
}


size_t TSyrecParser::LoopStepsizeDefinitionContext::getRuleIndex() const {
  return TSyrecParser::RuleLoopStepsizeDefinition;
}


std::any TSyrecParser::LoopStepsizeDefinitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitLoopStepsizeDefinition(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::LoopStepsizeDefinitionContext* TSyrecParser::loopStepsizeDefinition() {
  LoopStepsizeDefinitionContext *_localctx = _tracker.createInstance<LoopStepsizeDefinitionContext>(_ctx, getState());
  enterRule(_localctx, 22, TSyrecParser::RuleLoopStepsizeDefinition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(149);
    match(TSyrecParser::KEYWORD_STEP);
    setState(151);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == TSyrecParser::OP_MINUS) {
      setState(150);
      match(TSyrecParser::OP_MINUS);
    }
    setState(153);
    number();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ForStatementContext ------------------------------------------------------------------

TSyrecParser::ForStatementContext::ForStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* TSyrecParser::ForStatementContext::KEYWORD_FOR() {
  return getToken(TSyrecParser::KEYWORD_FOR, 0);
}

tree::TerminalNode* TSyrecParser::ForStatementContext::KEYWORD_DO() {
  return getToken(TSyrecParser::KEYWORD_DO, 0);
}

TSyrecParser::StatementListContext* TSyrecParser::ForStatementContext::statementList() {
  return getRuleContext<TSyrecParser::StatementListContext>(0);
}

tree::TerminalNode* TSyrecParser::ForStatementContext::KEYWORD_ROF() {
  return getToken(TSyrecParser::KEYWORD_ROF, 0);
}

std::vector<TSyrecParser::NumberContext *> TSyrecParser::ForStatementContext::number() {
  return getRuleContexts<TSyrecParser::NumberContext>();
}

TSyrecParser::NumberContext* TSyrecParser::ForStatementContext::number(size_t i) {
  return getRuleContext<TSyrecParser::NumberContext>(i);
}

tree::TerminalNode* TSyrecParser::ForStatementContext::KEYWORD_TO() {
  return getToken(TSyrecParser::KEYWORD_TO, 0);
}

TSyrecParser::LoopStepsizeDefinitionContext* TSyrecParser::ForStatementContext::loopStepsizeDefinition() {
  return getRuleContext<TSyrecParser::LoopStepsizeDefinitionContext>(0);
}

TSyrecParser::LoopVariableDefinitionContext* TSyrecParser::ForStatementContext::loopVariableDefinition() {
  return getRuleContext<TSyrecParser::LoopVariableDefinitionContext>(0);
}


size_t TSyrecParser::ForStatementContext::getRuleIndex() const {
  return TSyrecParser::RuleForStatement;
}


std::any TSyrecParser::ForStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitForStatement(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::ForStatementContext* TSyrecParser::forStatement() {
  ForStatementContext *_localctx = _tracker.createInstance<ForStatementContext>(_ctx, getState());
  enterRule(_localctx, 24, TSyrecParser::RuleForStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(155);
    match(TSyrecParser::KEYWORD_FOR);
    setState(162);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx)) {
    case 1: {
      setState(157);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 12, _ctx)) {
      case 1: {
        setState(156);
        loopVariableDefinition();
        break;
      }

      default:
        break;
      }
      setState(159);
      antlrcpp::downCast<ForStatementContext *>(_localctx)->startValue = number();
      setState(160);
      match(TSyrecParser::KEYWORD_TO);
      break;
    }

    default:
      break;
    }
    setState(164);
    antlrcpp::downCast<ForStatementContext *>(_localctx)->endValue = number();
    setState(166);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == TSyrecParser::KEYWORD_STEP) {
      setState(165);
      loopStepsizeDefinition();
    }
    setState(168);
    match(TSyrecParser::KEYWORD_DO);
    setState(169);
    statementList();
    setState(170);
    match(TSyrecParser::KEYWORD_ROF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IfStatementContext ------------------------------------------------------------------

TSyrecParser::IfStatementContext::IfStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* TSyrecParser::IfStatementContext::KEYWORD_IF() {
  return getToken(TSyrecParser::KEYWORD_IF, 0);
}

tree::TerminalNode* TSyrecParser::IfStatementContext::KEYWORD_THEN() {
  return getToken(TSyrecParser::KEYWORD_THEN, 0);
}

tree::TerminalNode* TSyrecParser::IfStatementContext::KEYWORD_ELSE() {
  return getToken(TSyrecParser::KEYWORD_ELSE, 0);
}

tree::TerminalNode* TSyrecParser::IfStatementContext::KEYWORD_FI() {
  return getToken(TSyrecParser::KEYWORD_FI, 0);
}

std::vector<TSyrecParser::ExpressionContext *> TSyrecParser::IfStatementContext::expression() {
  return getRuleContexts<TSyrecParser::ExpressionContext>();
}

TSyrecParser::ExpressionContext* TSyrecParser::IfStatementContext::expression(size_t i) {
  return getRuleContext<TSyrecParser::ExpressionContext>(i);
}

std::vector<TSyrecParser::StatementListContext *> TSyrecParser::IfStatementContext::statementList() {
  return getRuleContexts<TSyrecParser::StatementListContext>();
}

TSyrecParser::StatementListContext* TSyrecParser::IfStatementContext::statementList(size_t i) {
  return getRuleContext<TSyrecParser::StatementListContext>(i);
}


size_t TSyrecParser::IfStatementContext::getRuleIndex() const {
  return TSyrecParser::RuleIfStatement;
}


std::any TSyrecParser::IfStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitIfStatement(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::IfStatementContext* TSyrecParser::ifStatement() {
  IfStatementContext *_localctx = _tracker.createInstance<IfStatementContext>(_ctx, getState());
  enterRule(_localctx, 26, TSyrecParser::RuleIfStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(172);
    match(TSyrecParser::KEYWORD_IF);
    setState(173);
    antlrcpp::downCast<IfStatementContext *>(_localctx)->guardCondition = expression();
    setState(174);
    match(TSyrecParser::KEYWORD_THEN);
    setState(175);
    antlrcpp::downCast<IfStatementContext *>(_localctx)->trueBranchStmts = statementList();
    setState(176);
    match(TSyrecParser::KEYWORD_ELSE);
    setState(177);
    antlrcpp::downCast<IfStatementContext *>(_localctx)->falseBranchStmts = statementList();
    setState(178);
    match(TSyrecParser::KEYWORD_FI);
    setState(179);
    antlrcpp::downCast<IfStatementContext *>(_localctx)->matchingGuardExpression = expression();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnaryStatementContext ------------------------------------------------------------------

TSyrecParser::UnaryStatementContext::UnaryStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

TSyrecParser::SignalContext* TSyrecParser::UnaryStatementContext::signal() {
  return getRuleContext<TSyrecParser::SignalContext>(0);
}

tree::TerminalNode* TSyrecParser::UnaryStatementContext::OP_INVERT_ASSIGN() {
  return getToken(TSyrecParser::OP_INVERT_ASSIGN, 0);
}

tree::TerminalNode* TSyrecParser::UnaryStatementContext::OP_INCREMENT_ASSIGN() {
  return getToken(TSyrecParser::OP_INCREMENT_ASSIGN, 0);
}

tree::TerminalNode* TSyrecParser::UnaryStatementContext::OP_DECREMENT_ASSIGN() {
  return getToken(TSyrecParser::OP_DECREMENT_ASSIGN, 0);
}


size_t TSyrecParser::UnaryStatementContext::getRuleIndex() const {
  return TSyrecParser::RuleUnaryStatement;
}


std::any TSyrecParser::UnaryStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitUnaryStatement(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::UnaryStatementContext* TSyrecParser::unaryStatement() {
  UnaryStatementContext *_localctx = _tracker.createInstance<UnaryStatementContext>(_ctx, getState());
  enterRule(_localctx, 28, TSyrecParser::RuleUnaryStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(181);
    antlrcpp::downCast<UnaryStatementContext *>(_localctx)->unaryOp = _input->LT(1);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 14) != 0))) {
      antlrcpp::downCast<UnaryStatementContext *>(_localctx)->unaryOp = _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(182);
    signal();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AssignStatementContext ------------------------------------------------------------------

TSyrecParser::AssignStatementContext::AssignStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

TSyrecParser::SignalContext* TSyrecParser::AssignStatementContext::signal() {
  return getRuleContext<TSyrecParser::SignalContext>(0);
}

TSyrecParser::ExpressionContext* TSyrecParser::AssignStatementContext::expression() {
  return getRuleContext<TSyrecParser::ExpressionContext>(0);
}

tree::TerminalNode* TSyrecParser::AssignStatementContext::OP_ADD_ASSIGN() {
  return getToken(TSyrecParser::OP_ADD_ASSIGN, 0);
}

tree::TerminalNode* TSyrecParser::AssignStatementContext::OP_SUB_ASSIGN() {
  return getToken(TSyrecParser::OP_SUB_ASSIGN, 0);
}

tree::TerminalNode* TSyrecParser::AssignStatementContext::OP_XOR_ASSIGN() {
  return getToken(TSyrecParser::OP_XOR_ASSIGN, 0);
}


size_t TSyrecParser::AssignStatementContext::getRuleIndex() const {
  return TSyrecParser::RuleAssignStatement;
}


std::any TSyrecParser::AssignStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitAssignStatement(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::AssignStatementContext* TSyrecParser::assignStatement() {
  AssignStatementContext *_localctx = _tracker.createInstance<AssignStatementContext>(_ctx, getState());
  enterRule(_localctx, 30, TSyrecParser::RuleAssignStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(184);
    signal();
    setState(185);
    antlrcpp::downCast<AssignStatementContext *>(_localctx)->assignmentOp = _input->LT(1);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 112) != 0))) {
      antlrcpp::downCast<AssignStatementContext *>(_localctx)->assignmentOp = _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(186);
    expression();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SwapStatementContext ------------------------------------------------------------------

TSyrecParser::SwapStatementContext::SwapStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* TSyrecParser::SwapStatementContext::OP_SWAP() {
  return getToken(TSyrecParser::OP_SWAP, 0);
}

std::vector<TSyrecParser::SignalContext *> TSyrecParser::SwapStatementContext::signal() {
  return getRuleContexts<TSyrecParser::SignalContext>();
}

TSyrecParser::SignalContext* TSyrecParser::SwapStatementContext::signal(size_t i) {
  return getRuleContext<TSyrecParser::SignalContext>(i);
}


size_t TSyrecParser::SwapStatementContext::getRuleIndex() const {
  return TSyrecParser::RuleSwapStatement;
}


std::any TSyrecParser::SwapStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitSwapStatement(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::SwapStatementContext* TSyrecParser::swapStatement() {
  SwapStatementContext *_localctx = _tracker.createInstance<SwapStatementContext>(_ctx, getState());
  enterRule(_localctx, 32, TSyrecParser::RuleSwapStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(188);
    antlrcpp::downCast<SwapStatementContext *>(_localctx)->lhsOperand = signal();
    setState(189);
    match(TSyrecParser::OP_SWAP);
    setState(190);
    antlrcpp::downCast<SwapStatementContext *>(_localctx)->rhsOperand = signal();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SkipStatementContext ------------------------------------------------------------------

TSyrecParser::SkipStatementContext::SkipStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* TSyrecParser::SkipStatementContext::KEYWORD_SKIP() {
  return getToken(TSyrecParser::KEYWORD_SKIP, 0);
}


size_t TSyrecParser::SkipStatementContext::getRuleIndex() const {
  return TSyrecParser::RuleSkipStatement;
}


std::any TSyrecParser::SkipStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitSkipStatement(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::SkipStatementContext* TSyrecParser::skipStatement() {
  SkipStatementContext *_localctx = _tracker.createInstance<SkipStatementContext>(_ctx, getState());
  enterRule(_localctx, 34, TSyrecParser::RuleSkipStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(192);
    match(TSyrecParser::KEYWORD_SKIP);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SignalContext ------------------------------------------------------------------

TSyrecParser::SignalContext::SignalContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* TSyrecParser::SignalContext::IDENT() {
  return getToken(TSyrecParser::IDENT, 0);
}

std::vector<tree::TerminalNode *> TSyrecParser::SignalContext::OPEN_SBRACKET() {
  return getTokens(TSyrecParser::OPEN_SBRACKET);
}

tree::TerminalNode* TSyrecParser::SignalContext::OPEN_SBRACKET(size_t i) {
  return getToken(TSyrecParser::OPEN_SBRACKET, i);
}

std::vector<tree::TerminalNode *> TSyrecParser::SignalContext::CLOSE_SBRACKET() {
  return getTokens(TSyrecParser::CLOSE_SBRACKET);
}

tree::TerminalNode* TSyrecParser::SignalContext::CLOSE_SBRACKET(size_t i) {
  return getToken(TSyrecParser::CLOSE_SBRACKET, i);
}

tree::TerminalNode* TSyrecParser::SignalContext::BITRANGE_START_PREFIX() {
  return getToken(TSyrecParser::BITRANGE_START_PREFIX, 0);
}

std::vector<TSyrecParser::ExpressionContext *> TSyrecParser::SignalContext::expression() {
  return getRuleContexts<TSyrecParser::ExpressionContext>();
}

TSyrecParser::ExpressionContext* TSyrecParser::SignalContext::expression(size_t i) {
  return getRuleContext<TSyrecParser::ExpressionContext>(i);
}

std::vector<TSyrecParser::NumberContext *> TSyrecParser::SignalContext::number() {
  return getRuleContexts<TSyrecParser::NumberContext>();
}

TSyrecParser::NumberContext* TSyrecParser::SignalContext::number(size_t i) {
  return getRuleContext<TSyrecParser::NumberContext>(i);
}

tree::TerminalNode* TSyrecParser::SignalContext::BITRANGE_END_PREFIX() {
  return getToken(TSyrecParser::BITRANGE_END_PREFIX, 0);
}


size_t TSyrecParser::SignalContext::getRuleIndex() const {
  return TSyrecParser::RuleSignal;
}


std::any TSyrecParser::SignalContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitSignal(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::SignalContext* TSyrecParser::signal() {
  SignalContext *_localctx = _tracker.createInstance<SignalContext>(_ctx, getState());
  enterRule(_localctx, 36, TSyrecParser::RuleSignal);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(194);
    match(TSyrecParser::IDENT);
    setState(201);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == TSyrecParser::OPEN_SBRACKET) {
      setState(195);
      match(TSyrecParser::OPEN_SBRACKET);
      setState(196);
      antlrcpp::downCast<SignalContext *>(_localctx)->expressionContext = expression();
      antlrcpp::downCast<SignalContext *>(_localctx)->accessedDimensions.push_back(antlrcpp::downCast<SignalContext *>(_localctx)->expressionContext);
      setState(197);
      match(TSyrecParser::CLOSE_SBRACKET);
      setState(203);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(210);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == TSyrecParser::BITRANGE_START_PREFIX) {
      setState(204);
      match(TSyrecParser::BITRANGE_START_PREFIX);
      setState(205);
      antlrcpp::downCast<SignalContext *>(_localctx)->bitStart = number();
      setState(208);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == TSyrecParser::BITRANGE_END_PREFIX) {
        setState(206);
        match(TSyrecParser::BITRANGE_END_PREFIX);
        setState(207);
        antlrcpp::downCast<SignalContext *>(_localctx)->bitRangeEnd = number();
      }
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionContext ------------------------------------------------------------------

TSyrecParser::ExpressionContext::ExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t TSyrecParser::ExpressionContext::getRuleIndex() const {
  return TSyrecParser::RuleExpression;
}

void TSyrecParser::ExpressionContext::copyFrom(ExpressionContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- ExpressionFromSignalContext ------------------------------------------------------------------

TSyrecParser::SignalContext* TSyrecParser::ExpressionFromSignalContext::signal() {
  return getRuleContext<TSyrecParser::SignalContext>(0);
}

TSyrecParser::ExpressionFromSignalContext::ExpressionFromSignalContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any TSyrecParser::ExpressionFromSignalContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitExpressionFromSignal(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ExpressionFromBinaryExpressionContext ------------------------------------------------------------------

TSyrecParser::BinaryExpressionContext* TSyrecParser::ExpressionFromBinaryExpressionContext::binaryExpression() {
  return getRuleContext<TSyrecParser::BinaryExpressionContext>(0);
}

TSyrecParser::ExpressionFromBinaryExpressionContext::ExpressionFromBinaryExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any TSyrecParser::ExpressionFromBinaryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitExpressionFromBinaryExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ExpressionFromNumberContext ------------------------------------------------------------------

TSyrecParser::NumberContext* TSyrecParser::ExpressionFromNumberContext::number() {
  return getRuleContext<TSyrecParser::NumberContext>(0);
}

TSyrecParser::ExpressionFromNumberContext::ExpressionFromNumberContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any TSyrecParser::ExpressionFromNumberContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitExpressionFromNumber(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ExpressionFromUnaryExpressionContext ------------------------------------------------------------------

TSyrecParser::UnaryExpressionContext* TSyrecParser::ExpressionFromUnaryExpressionContext::unaryExpression() {
  return getRuleContext<TSyrecParser::UnaryExpressionContext>(0);
}

TSyrecParser::ExpressionFromUnaryExpressionContext::ExpressionFromUnaryExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any TSyrecParser::ExpressionFromUnaryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitExpressionFromUnaryExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ExpressionFromShiftExpressionContext ------------------------------------------------------------------

TSyrecParser::ShiftExpressionContext* TSyrecParser::ExpressionFromShiftExpressionContext::shiftExpression() {
  return getRuleContext<TSyrecParser::ShiftExpressionContext>(0);
}

TSyrecParser::ExpressionFromShiftExpressionContext::ExpressionFromShiftExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any TSyrecParser::ExpressionFromShiftExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitExpressionFromShiftExpression(this);
  else
    return visitor->visitChildren(this);
}
TSyrecParser::ExpressionContext* TSyrecParser::expression() {
  ExpressionContext *_localctx = _tracker.createInstance<ExpressionContext>(_ctx, getState());
  enterRule(_localctx, 38, TSyrecParser::RuleExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(217);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 18, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<TSyrecParser::ExpressionFromNumberContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(212);
      number();
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<TSyrecParser::ExpressionFromSignalContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(213);
      signal();
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<TSyrecParser::ExpressionFromBinaryExpressionContext>(_localctx);
      enterOuterAlt(_localctx, 3);
      setState(214);
      binaryExpression();
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<TSyrecParser::ExpressionFromUnaryExpressionContext>(_localctx);
      enterOuterAlt(_localctx, 4);
      setState(215);
      unaryExpression();
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<TSyrecParser::ExpressionFromShiftExpressionContext>(_localctx);
      enterOuterAlt(_localctx, 5);
      setState(216);
      shiftExpression();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BinaryExpressionContext ------------------------------------------------------------------

TSyrecParser::BinaryExpressionContext::BinaryExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* TSyrecParser::BinaryExpressionContext::OPEN_RBRACKET() {
  return getToken(TSyrecParser::OPEN_RBRACKET, 0);
}

tree::TerminalNode* TSyrecParser::BinaryExpressionContext::CLOSE_RBRACKET() {
  return getToken(TSyrecParser::CLOSE_RBRACKET, 0);
}

std::vector<TSyrecParser::ExpressionContext *> TSyrecParser::BinaryExpressionContext::expression() {
  return getRuleContexts<TSyrecParser::ExpressionContext>();
}

TSyrecParser::ExpressionContext* TSyrecParser::BinaryExpressionContext::expression(size_t i) {
  return getRuleContext<TSyrecParser::ExpressionContext>(i);
}

tree::TerminalNode* TSyrecParser::BinaryExpressionContext::OP_PLUS() {
  return getToken(TSyrecParser::OP_PLUS, 0);
}

tree::TerminalNode* TSyrecParser::BinaryExpressionContext::OP_MINUS() {
  return getToken(TSyrecParser::OP_MINUS, 0);
}

tree::TerminalNode* TSyrecParser::BinaryExpressionContext::OP_MULTIPLY() {
  return getToken(TSyrecParser::OP_MULTIPLY, 0);
}

tree::TerminalNode* TSyrecParser::BinaryExpressionContext::OP_DIVISION() {
  return getToken(TSyrecParser::OP_DIVISION, 0);
}

tree::TerminalNode* TSyrecParser::BinaryExpressionContext::OP_MODULO() {
  return getToken(TSyrecParser::OP_MODULO, 0);
}

tree::TerminalNode* TSyrecParser::BinaryExpressionContext::OP_UPPER_BIT_MULTIPLY() {
  return getToken(TSyrecParser::OP_UPPER_BIT_MULTIPLY, 0);
}

tree::TerminalNode* TSyrecParser::BinaryExpressionContext::OP_LOGICAL_AND() {
  return getToken(TSyrecParser::OP_LOGICAL_AND, 0);
}

tree::TerminalNode* TSyrecParser::BinaryExpressionContext::OP_LOGICAL_OR() {
  return getToken(TSyrecParser::OP_LOGICAL_OR, 0);
}

tree::TerminalNode* TSyrecParser::BinaryExpressionContext::OP_BITWISE_AND() {
  return getToken(TSyrecParser::OP_BITWISE_AND, 0);
}

tree::TerminalNode* TSyrecParser::BinaryExpressionContext::OP_BITWISE_OR() {
  return getToken(TSyrecParser::OP_BITWISE_OR, 0);
}

tree::TerminalNode* TSyrecParser::BinaryExpressionContext::OP_BITWISE_XOR() {
  return getToken(TSyrecParser::OP_BITWISE_XOR, 0);
}

tree::TerminalNode* TSyrecParser::BinaryExpressionContext::OP_LESS_THAN() {
  return getToken(TSyrecParser::OP_LESS_THAN, 0);
}

tree::TerminalNode* TSyrecParser::BinaryExpressionContext::OP_GREATER_THAN() {
  return getToken(TSyrecParser::OP_GREATER_THAN, 0);
}

tree::TerminalNode* TSyrecParser::BinaryExpressionContext::OP_EQUAL() {
  return getToken(TSyrecParser::OP_EQUAL, 0);
}

tree::TerminalNode* TSyrecParser::BinaryExpressionContext::OP_NOT_EQUAL() {
  return getToken(TSyrecParser::OP_NOT_EQUAL, 0);
}

tree::TerminalNode* TSyrecParser::BinaryExpressionContext::OP_LESS_OR_EQUAL() {
  return getToken(TSyrecParser::OP_LESS_OR_EQUAL, 0);
}

tree::TerminalNode* TSyrecParser::BinaryExpressionContext::OP_GREATER_OR_EQUAL() {
  return getToken(TSyrecParser::OP_GREATER_OR_EQUAL, 0);
}


size_t TSyrecParser::BinaryExpressionContext::getRuleIndex() const {
  return TSyrecParser::RuleBinaryExpression;
}


std::any TSyrecParser::BinaryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitBinaryExpression(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::BinaryExpressionContext* TSyrecParser::binaryExpression() {
  BinaryExpressionContext *_localctx = _tracker.createInstance<BinaryExpressionContext>(_ctx, getState());
  enterRule(_localctx, 40, TSyrecParser::RuleBinaryExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(219);
    match(TSyrecParser::OPEN_RBRACKET);
    setState(220);
    antlrcpp::downCast<BinaryExpressionContext *>(_localctx)->lhsOperand = expression();
    setState(221);
    antlrcpp::downCast<BinaryExpressionContext *>(_localctx)->binaryOperation = _input->LT(1);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 452927360) != 0))) {
      antlrcpp::downCast<BinaryExpressionContext *>(_localctx)->binaryOperation = _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(222);
    antlrcpp::downCast<BinaryExpressionContext *>(_localctx)->rhsOperand = expression();
    setState(223);
    match(TSyrecParser::CLOSE_RBRACKET);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnaryExpressionContext ------------------------------------------------------------------

TSyrecParser::UnaryExpressionContext::UnaryExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

TSyrecParser::ExpressionContext* TSyrecParser::UnaryExpressionContext::expression() {
  return getRuleContext<TSyrecParser::ExpressionContext>(0);
}

tree::TerminalNode* TSyrecParser::UnaryExpressionContext::OP_LOGICAL_NEGATION() {
  return getToken(TSyrecParser::OP_LOGICAL_NEGATION, 0);
}

tree::TerminalNode* TSyrecParser::UnaryExpressionContext::OP_BITWISE_NEGATION() {
  return getToken(TSyrecParser::OP_BITWISE_NEGATION, 0);
}


size_t TSyrecParser::UnaryExpressionContext::getRuleIndex() const {
  return TSyrecParser::RuleUnaryExpression;
}


std::any TSyrecParser::UnaryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitUnaryExpression(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::UnaryExpressionContext* TSyrecParser::unaryExpression() {
  UnaryExpressionContext *_localctx = _tracker.createInstance<UnaryExpressionContext>(_ctx, getState());
  enterRule(_localctx, 42, TSyrecParser::RuleUnaryExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(225);
    antlrcpp::downCast<UnaryExpressionContext *>(_localctx)->unaryOperation = _input->LT(1);
    _la = _input->LA(1);
    if (!(_la == TSyrecParser::OP_LOGICAL_NEGATION

    || _la == TSyrecParser::OP_BITWISE_NEGATION)) {
      antlrcpp::downCast<UnaryExpressionContext *>(_localctx)->unaryOperation = _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(226);
    expression();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ShiftExpressionContext ------------------------------------------------------------------

TSyrecParser::ShiftExpressionContext::ShiftExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* TSyrecParser::ShiftExpressionContext::OPEN_RBRACKET() {
  return getToken(TSyrecParser::OPEN_RBRACKET, 0);
}

TSyrecParser::ExpressionContext* TSyrecParser::ShiftExpressionContext::expression() {
  return getRuleContext<TSyrecParser::ExpressionContext>(0);
}

TSyrecParser::NumberContext* TSyrecParser::ShiftExpressionContext::number() {
  return getRuleContext<TSyrecParser::NumberContext>(0);
}

tree::TerminalNode* TSyrecParser::ShiftExpressionContext::CLOSE_RBRACKET() {
  return getToken(TSyrecParser::CLOSE_RBRACKET, 0);
}

tree::TerminalNode* TSyrecParser::ShiftExpressionContext::OP_RIGHT_SHIFT() {
  return getToken(TSyrecParser::OP_RIGHT_SHIFT, 0);
}

tree::TerminalNode* TSyrecParser::ShiftExpressionContext::OP_LEFT_SHIFT() {
  return getToken(TSyrecParser::OP_LEFT_SHIFT, 0);
}


size_t TSyrecParser::ShiftExpressionContext::getRuleIndex() const {
  return TSyrecParser::RuleShiftExpression;
}


std::any TSyrecParser::ShiftExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<TSyrecParserVisitor*>(visitor))
    return parserVisitor->visitShiftExpression(this);
  else
    return visitor->visitChildren(this);
}

TSyrecParser::ShiftExpressionContext* TSyrecParser::shiftExpression() {
  ShiftExpressionContext *_localctx = _tracker.createInstance<ShiftExpressionContext>(_ctx, getState());
  enterRule(_localctx, 44, TSyrecParser::RuleShiftExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(228);
    match(TSyrecParser::OPEN_RBRACKET);
    setState(229);
    expression();
    setState(230);
    antlrcpp::downCast<ShiftExpressionContext *>(_localctx)->shiftOperation = _input->LT(1);
    _la = _input->LA(1);
    if (!(_la == TSyrecParser::OP_LEFT_SHIFT

    || _la == TSyrecParser::OP_RIGHT_SHIFT)) {
      antlrcpp::downCast<ShiftExpressionContext *>(_localctx)->shiftOperation = _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(231);
    number();
    setState(232);
    match(TSyrecParser::CLOSE_RBRACKET);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

void TSyrecParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  tsyrecparserParserInitialize();
#else
  ::antlr4::internal::call_once(tsyrecparserParserOnceFlag, tsyrecparserParserInitialize);
#endif
}
