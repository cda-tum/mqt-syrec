#include "test_syrec_parser_errors_base.hpp"

TEST_F(SyrecParserErrorTestsFixture, OmittingUncallKeywordCausesError) {
    recordSyntaxError(Message::Position(1, 95), "no viable alternative at input 'add('");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) add(a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidUncallKeywordCausesError) {
    recordSyntaxError(Message::Position(1, 104), "no viable alternative at input 'performCall add'");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) performCall add(a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingModuleIdentiferInUncallStatementCausesError) {
    recordSyntaxError(Message::Position(1, 47), "extraneous input '(' expecting IDENT");
    recordSyntaxError(Message::Position(1, 49), "mismatched input ',' expecting '('");
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingIdentifier>(Message::Position(1, 48), "a");
    performTestExecution("module main(in a(4), in b(4), out c(4)) uncall (a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleIdentifierNotMatchingAnyDeclaredModuleIdentifierInUncallStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingIdentifier>(Message::Position(1, 47), "add");
    performTestExecution("module main(in a(4), in b(4), out c(4)) uncall add(a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingUncallStatementParameterOpeningBracket) {
    recordSyntaxError(Message::Position(1, 103), "mismatched input ',' expecting '('");
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingIdentifier>(Message::Position(1, 99), "adda");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall adda, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidUncallStatementParameterOpeningBracket) {
    recordSyntaxError(Message::Position(1, 102), "mismatched input '[' expecting '('");
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 99));
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall add[a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingUncallStatementParameterIdentifierCausesError) {
    recordSyntaxError(Message::Position(1, 103), "extraneous input ',' expecting IDENT");
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 99));
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall add(, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, UncallStatementParameterIdentifierNotMatchingAnyDeclaredVariableCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 103), "d");
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 99));
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall add(d, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingUncallStatementParameterDelimiterCausesError) {
    recordSyntaxError(Message::Position(1, 105), "extraneous input 'b' expecting {',', ')'}");
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 99));
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall add(a b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingUncallStatementParameterClosingBracket) {
    recordSyntaxError(Message::Position(1, 110), "extraneous input '<EOF>' expecting {',', ')'}");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall add(a, b, c");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidUncallStatementParameterClosingBracket) {
    recordSyntaxError(Message::Position(1, 110), "extraneous input ']' expecting {',', ')'}");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall add(a, b, c]");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleUncallOverloadResolutionFailedDueToVariableBitwidthMissmatchCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 153));
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module add(in a(8), in b(8), out c(8)) c ^= (a + b) module main(in a(12), in b(8), out c(10)) uncall add(a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleUncallOverloadResolutionFailedDueToVariableTypeMissmatchCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 104));
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in res(4)) wire tmp1(4), tmp2(4) uncall add(tmp1, tmp2, res)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleUncallOverloadResolutionFailedDueToNumberOfValuesForDimensionBeingSmallerThanExpectedForParameterCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 111));
    performTestExecution("module add(in a[2](4), in b(4), out c(4)) c ^= (a[0] + b) module main(out res(4)) wire tmp1(4), tmp2(4) uncall add(tmp1, tmp2, res)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleUncallOverloadResolutionFailedDueToNumberOfValuesForDimensionBeingLargerThanExpectedForParameterCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 108));
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(out res[2](4)) wire tmp1(4), tmp2(4) uncall add(tmp1, tmp2, res)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleUncallOverloadResolutionFailedDueToNumberOfDimensionsOfUserVariableSmallerThanExpectedForParameterCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 117));
    performTestExecution("module add(in a(4), in b[2][3](4), out c(4)) c ^= (a + b[0][1]) module main(out res(4)) wire tmp1(4), tmp2(4) uncall add(tmp1, tmp2, res)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleUncallOverloadResolutionFailedDueToNumberOfDimensionsOfUserVariableLargerThanExpectedForParameterCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 169));
    performTestExecution("module add(in a(4), in b[2](4), out c(4)) c ^= (a + b[0]) module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(out res[2](4)) wire tmp1(4), tmp2[2](4) uncall add(tmp1, tmp2, res)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleUncallOverloadResolutionFailedDueToTooManyUserDefinedParametersCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 136));
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module add(in a(4), out c(4)) c ^= a module main(in a(4), in b(4), out c(4)) uncall add(a, b, c, c)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleUncallOverloadResolutionFailedDueToTooFewUserDefinedParametersCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingCallSignature>(Message::Position(1, 136));
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module add(in a(4), out c(4)) c ^= a module main(in a(4), in b(4), out c(4)) uncall add(a, b)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleUncallOfImplicitlyDefinedMainModuleCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::CannotCallMainModule>(Message::Position(1, 55));
    performTestExecution("module increment(out c(4)) wire one(4) ++= one; uncall add(c, one, c) module add(in a(4), in b(4), out c(4)) c ^= (a + b)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleUncallOverloadResolutionResolvesToOfImplicitlyDefinedMainModuleCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::CannotCallMainModule>(Message::Position(1, 55));
    performTestExecution("module increment(out c(4)) wire one(4) ++= one; uncall add(c, one) module add(in a(4), in b(4), out c(4)) c ^= (a + b) module add(inout a(4), in b(4)) a += b");
}