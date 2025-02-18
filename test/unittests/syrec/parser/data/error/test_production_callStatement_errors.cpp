#include "test_syrec_parser_errors_base.hpp"

TEST_F(SyrecParserErrorTestsFixture, OmittingCallKeywordCausesError) {
    recordSyntaxError(Message::Position(1, 95), "no viable alternative at input 'add('");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) add(a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidCallKeywordCausesError) {
    recordSyntaxError(Message::Position(1, 104), "no viable alternative at input 'performCall add'");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) performCall add(a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingModuleIdentiferInCallStatementCausesError) {
    recordSyntaxError(Message::Position(1, 45), "extraneous input '(' expecting IDENT");
    recordSyntaxError(Message::Position(1, 47), "mismatched input ',' expecting '('");
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingIdentifier>(Message::Position(1, 46), "a");
    performTestExecution("module main(in a(4), in b(4), out c(4)) call (a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleIdentifierNotMatchingAnyDeclaredModuleIdentifierInCallStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingIdentifier>(Message::Position(1, 45), "add");
    performTestExecution("module main(in a(4), in b(4), out c(4)) call add(a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingCallStatementParameterOpeningBracket) {
    recordSyntaxError(Message::Position(1, 101), "mismatched input ',' expecting '('");
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingIdentifier>(Message::Position(1, 97), "adda");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call adda, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidCallStatementParameterOpeningBracket) {
    recordSyntaxError(Message::Position(1, 100), "mismatched input '[' expecting '('");
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 97));
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call add[a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingCallStatementParameterIdentifierCausesError) {
    recordSyntaxError(Message::Position(1, 101), "extraneous input ',' expecting IDENT");
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 97));
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call add(, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, CallStatementParameterIdentifierNotMatchingAnyDeclaredVariableCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 101), "d");
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 97));
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call add(d, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingCallStatementParameterDelimiterCausesError) {
    recordSyntaxError(Message::Position(1, 103), "extraneous input 'b' expecting {',', ')'}");
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 97));
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call add(a b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableAsCallStatementParameterCausesError) {
    recordSyntaxError(Message::Position(1, 73), "extraneous input '$' expecting IDENT");
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 74), "i");
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 68));
    performTestExecution("module incr(inout a(4)) ++= a module main() for $i = 0 to 3 do call incr($i) rof");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingCallStatementParameterClosingBracket) {
    recordSyntaxError(Message::Position(1, 108), "extraneous input '<EOF>' expecting {',', ')'}");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call add(a, b, c");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidCallStatementParameterClosingBracket) {
    recordSyntaxError(Message::Position(1, 108), "extraneous input ']' expecting {',', ')'}");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call add(a, b, c]");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleCallOverloadResolutionFailedDueToVariableBitwidthMissmatchCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 151));
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module add(in a(8), in b(8), out c(8)) c ^= (a + b) module main(in a(12), in b(8), out c(10)) call add(a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleCallOverloadResolutionFailedDueToVariableTypeMissmatchCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 102));
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in res(4)) wire tmp1(4), tmp2(4) call add(tmp1, tmp2, res)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleCallOverloadResolutionFailedDueToNumberOfValuesForDimensionBeingSmallerThanExpectedForParameterCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 109));
    performTestExecution("module add(in a[2](4), in b(4), out c(4)) c ^= (a[0] + b) module main(out res(4)) wire tmp1(4), tmp2(4) call add(tmp1, tmp2, res)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleCallOverloadResolutionFailedDueToNumberOfValuesForDimensionBeingLargerThanExpectedForParameterCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 106));
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(out res[2](4)) wire tmp1(4), tmp2(4) call add(tmp1, tmp2, res)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleCallOverloadResolutionFailedDueToNumberOfDimensionsOfUserVariableSmallerThanExpectedForParameterCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 115));
    performTestExecution("module add(in a(4), in b[2][3](4), out c(4)) c ^= (a + b[0][1]) module main(out res(4)) wire tmp1(4), tmp2(4) call add(tmp1, tmp2, res)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleCallOverloadResolutionFailedDueToNumberOfDimensionsOfUserVariableLargerThanExpectedForParameterCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 167));
    performTestExecution("module add(in a(4), in b[2](4), out c(4)) c ^= (a + b[0]) module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(out res[2](4)) wire tmp1(4), tmp2[2](4) call add(tmp1, tmp2, res)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleCallOverloadResolutionFailedDueToTooManyUserDefinedParametersCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 134));
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module add(in a(4), out c(4)) c ^= a module main(in a(4), in b(4), out c(4)) call add(a, b, c, c)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleCallOverloadResolutionFailedDueToTooFewUserDefinedParametersCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 134));
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module add(in a(4), out c(4)) c ^= a module main(in a(4), in b(4), out c(4)) call add(a, b)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleCallOfImplicitlyDefinedMainModuleCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::CannotCallMainModule>(Message::Position(1, 53));
    performTestExecution("module increment(out c(4)) wire one(4) ++= one; call add(c, one, c) module add(in a(4), in b(4), out c(4)) c ^= (a + b)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleCallOverloadResolutionResolvesToOfImplicitlyDefinedMainModuleCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::CannotCallMainModule>(Message::Position(1, 53));
    performTestExecution("module increment(out c(4)) wire one(4) ++= one; call add(c, one) module add(in a(4), in b(4), out c(4)) c ^= (a + b) module add(inout a(4), in b(4)) a += b");
}