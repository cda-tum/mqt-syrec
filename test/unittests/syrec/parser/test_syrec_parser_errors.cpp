#include "utils/test_syrec_parser_errors_base.hpp"
// TODO: Should we tests for non-integer numbers used in any signal declaration
// TODO: Check whether swaps or assignments (both unary and binary ones) between N-d signals are possible
// TODO: Should semantic errors in not taken branches of if-statement be reported
// TODO: Recursive module calls should be possible (it its the reponsibility of the user to prevent infinite loops)
// TODO: The user can use a variable multiple times as a caller argument for a module call (synthesis of the statement should then detected an overlap between the operands of any statement that would prevent the inversion of the latter)
// TODO: Should the user be able to perform recursive module calls even for the main module (defined either implicitly or explicitly, the latter is currently disallowed)
// TOOD: Is the expression defined for an if statement expected to have a bitwidth of one?
// TODO: Allow user to define integer constant truncation either as XOR or OR
// TODO: Some syrec synthesis test use the EXPECT_XX macros instead of the ASSERT_XX macros with the former silently failing and causes erros in latter code that should not execute 

// Tests for production module
TEST_F(SyrecParserErrorTestsFixture, OmittingModuleKeywordCausesError) {
    recordSyntaxError(Message::Position(1, 0), "mismatched input 'main' expecting 'module'");
    performTestExecution("main() skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidModuleKeywordUsageCausesError) {
    recordSyntaxError(Message::Position(1, 0), "mismatched input 'modul' expecting 'module'");
    performTestExecution("modul main() skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidSymbolInModuleIdentifierCausesError) {
    recordSyntaxError(Message::Position(1, 0), "mismatched input 'mod' expecting 'module'");
    performTestExecution("mod-ule main() skip");
}

TEST_F(SyrecParserErrorTestsFixture, EmptyModuleIdentifierCausesError) {
    recordSyntaxError(Message::Position(1, 7), "missing IDENT at '('");
    performTestExecution("module () skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingModuleParameterListOpeningBracketCausesError) {
    recordSyntaxError(Message::Position(1, 11), "missing '(' at ')'");
    performTestExecution("module main) skip");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidModuleParameterListOpeningBracketCausesError) {
    recordSyntaxError(Message::Position(1, 11), "mismatched input '[' expecting '('");
    performTestExecution("module main[) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingModuleParameterListClosingBracketCausesError) {
    recordSyntaxError(Message::Position(1, 13), "mismatched input 'skip' expecting {'in', 'out', 'inout', ')'}");
    performTestExecution("module main( skip");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidModuleParameterListClosingBracketCausesError) {
    recordSyntaxError(Message::Position(1, 12), "mismatched input ']' expecting {'in', 'out', 'inout', ')'}");
    performTestExecution("module main(] skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingModuleParameterDelimiterCausesError) {
    recordSyntaxError(Message::Position(1, 24), "mismatched input 'out' expecting ')'");
    performTestExecution("module main(in a[2](16) out b(16)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableDeclarationAfterModuleParameterDelimiterCausesError) {
    recordSyntaxError(Message::Position(1, 25), "mismatched input ')' expecting {'in', 'out', 'inout'}");
    performTestExecution("module main(in a[2](16), ) skip");
}

TEST_F(SyrecParserErrorTestsFixture, EmptyModuleBodyCausesError) {
    recordSyntaxError(Message::Position(1, 24), "mismatched input '<EOF>' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main(in a[2](16))");
}

// TODO: According to the specification, an overload of the top level module not named 'main' is possible
// TODO: The specification also does not disallow the definition of an overload of the module named 'main' but specifies that the user can define a top-level module with the special identifier 'main'
// see section 2.1
TEST_F(SyrecParserErrorTestsFixture, OverloadOfModuleNamedMainCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::DuplicateMainModuleDefinition>(Message::Position(1, 37));
    performTestExecution("module main(in a[2](16)) skip module main(out b[1](16)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateDeclarationOfModuleUsingSameInParameterTypeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::DuplicateModuleDeclaration>(Message::Position(1, 63), "add");
    performTestExecution("module add(in a(4), in b(4), out res(4)) res ^= (a + b) module add(in lOp(4), in rOp(4), out res(4)) res += lOp; res += rOp");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateDeclarationOfModuleUsingSameOutParameterTypeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::DuplicateModuleDeclaration>(Message::Position(1, 58), "double");
    performTestExecution("module double(in a(4), out res(4)) res ^= (a << 1) module double(in lOp(4), out res(4)) res += (lOp * 2)");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateDeclarationOfModuleUsingSameInoutParameterTypeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::DuplicateModuleDeclaration>(Message::Position(1, 46), "add");
    performTestExecution("module add(inout a(4), in b(4)) a += b module add(inout lOp(4), in rOp(4)) lOp ^= rOp");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateDeclarationOfModuleWithFirstModuleUsingOutParameterAndSecondModuleUsingInoutParameterTypeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::DuplicateModuleDeclaration>(Message::Position(1, 46), "add");
    performTestExecution("module add(inout a(4), in b(4)) a += b module add(out lOp(4), in rOp(4)) lOp ^= rOp");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateDeclarationOfModuleWithFirstModuleUsingInoutParameterAndSecondModuleUsingOutParameterTypeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::DuplicateModuleDeclaration>(Message::Position(1, 44), "add");
    performTestExecution("module add(out a(4), in b(4)) a += b module add(inout lOp(4), in rOp(4)) lOp ^= rOp");
}

// Tests for production parameter
TEST_F(SyrecParserErrorTestsFixture, OmittingVariableTypeCausesError) {
    recordSyntaxError(Message::Position(1, 12), "mismatched input 'a' expecting {'in', 'out', 'inout', ')'}");
    performTestExecution("module main(a[2](16)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidVariableTypeCausesError) {
    recordSyntaxError(Message::Position(1, 12), "mismatched input 'int' expecting {'in', 'out', 'inout', ')'}");
    performTestExecution("module main(int a(16)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableIdentifierInModuleParameterDeclarationCausesError) {
    recordSyntaxError(Message::Position(1, 15), "missing IDENT at '('");
    performTestExecution("module main(in (16)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateVariableIdentifierSharingSameVariableTypeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::DuplicateVariableDeclaration>(Message::Position(1, 25), "a");
    performTestExecution("module main(in a(16), in a(8)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateVariableIdentifierUsingDifferentVariableTypeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::DuplicateVariableDeclaration>(Message::Position(1, 26), "a");
    performTestExecution("module main(in a(16), out a(16)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidSymbolInModuleParameterIdentifierCausesError) {
    recordSyntaxError(Message::Position(1, 16), "mismatched input '-' expecting ')'");
    performTestExecution("module main(in a-t(16)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableValuesForDimensionOpeningBracketCausesError) {
    recordSyntaxError(Message::Position(1, 18), "mismatched input ']' expecting ')'");
    performTestExecution("module main(in a16](16)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidVariableValuesForDimensionOpeningBracketCausesError) {
    recordSyntaxError(Message::Position(1, 16), "token recognition error at: '{'");
    recordSyntaxError(Message::Position(1, 17), "mismatched input '16' expecting ')'");
    performTestExecution("module main(in a{16](2)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableValuesForDimensionClosingBracketCausesError) {
    recordSyntaxError(Message::Position(1, 18), "missing ']' at ')'");
    performTestExecution("module main(in a[2) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidVariableValuesForDimensionClosingBracketCausesError) {
    recordSyntaxError(Message::Position(1, 18), "token recognition error at: '}'");
    recordSyntaxError(Message::Position(1, 19), "missing ']' at ')'");
    performTestExecution("module main(in a[2}) skip");
}

TEST_F(SyrecParserErrorTestsFixture, NoneNumericValueForNumberOfValuesForDimensionCausesError) {
    recordSyntaxError(Message::Position(1, 17), "mismatched input 'test' expecting INT");
    recordSyntaxError(Message::Position(1, 25), "extraneous input ')' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main(in a[test](2)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingValueForNumberOfValuesForDimensionCausesError) {
    recordSyntaxError(Message::Position(1, 17), "missing INT at ']'");
    performTestExecution("module main(in a[]) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketForModuleParameterBitwidthDeclarationCausesError) {
    recordSyntaxError(Message::Position(1, 19), "extraneous input ')' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main(in a16)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketForModuleParameterBitwidthDeclarationCausesError) {
    recordSyntaxError(Message::Position(1, 16), "token recognition error at: '{'");
    recordSyntaxError(Message::Position(1, 17), "extraneous input '16' expecting ')'");
    recordSyntaxError(Message::Position(1, 20), "extraneous input ')' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main(in a{16)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketForModuleParameterBitwidthDeclarationCausesError) {
    recordSyntaxError(Message::Position(1, 21), "missing ')' at 'skip'");
    performTestExecution("module main(in a(16) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketForModuleParameterBitwidthDeclarationCausesError) {
    recordSyntaxError(Message::Position(1, 19), "token recognition error at: '}'");
    recordSyntaxError(Message::Position(1, 22), "missing ')' at 'skip'");
    performTestExecution("module main(in a(16}) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingModuleParameterBitwidthWithBracketsDefinedCausesError) {
    recordSyntaxError(Message::Position(1, 17), "missing INT at ')'");
    performTestExecution("module main(in a()) skip");
}

TEST_F(SyrecParserErrorTestsFixture, NoneNumericModuleParameterBitwidthCausesError) {
    recordSyntaxError(Message::Position(1, 19), "mismatched input '-' expecting ')'");
    recordSyntaxError(Message::Position(1, 22), "extraneous input ')' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main(in a(2 -3)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, DeclaredParameterBitwidthLongerThanMaximumSupportedValueCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::DeclaredVariableBitwidthTooLarge>(Message::Position(1, 17), 33, 32);
    buildAndRecordExpectedSemanticError<SemanticError::DeclaredVariableBitwidthTooLarge>(Message::Position(1, 40), 45, 32);
    performTestExecution("module main(in a(33), inout b(2), out c(45)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, ParameterBitwidthTakenFromUserConfigLongerThanMaximumSupportedValueCausesError) {
    constexpr unsigned int defaultSignalBitwidth = 33;
    const auto             userProvidedParserConfiguration = syrec::ReadProgramSettings(defaultSignalBitwidth);

    buildAndRecordExpectedSemanticError<SemanticError::DeclaredVariableBitwidthTooLarge>(Message::Position(1, 15), defaultSignalBitwidth, 32);
    buildAndRecordExpectedSemanticError<SemanticError::DeclaredVariableBitwidthTooLarge>(Message::Position(1, 34), defaultSignalBitwidth, 32);
    performTestExecution("module main(in a, inout b(2), out c) skip", userProvidedParserConfiguration);
}

// Tests for signal-list
TEST_F(SyrecParserErrorTestsFixture, OmittingLocalModuleParameterTypeInDeclarationCausesError) {
    recordSyntaxError(Message::Position(1, 15), "no viable alternative at input 'a('");
    performTestExecution("module main() a(16) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidLocalModuleParameterTypeInDeclarationCausesError) {
    recordSyntaxError(Message::Position(1, 18), "no viable alternative at input 'int a'");
    performTestExecution("module main() int a(16) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingLocalVariableDelimiterInDeclarationsSharingSameVariableType) {
    recordSyntaxError(Message::Position(1, 26), "no viable alternative at input 'b('");
    performTestExecution("module main() wire a(16) b(16) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableIdentifierInLocalModuleParameterDeclarationCausesError) {
    recordSyntaxError(Message::Position(1, 19), "missing IDENT at '('");
    performTestExecution("module main() wire (16) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableIdentifierInLocalModuleParameterDeclarationSharingSameVariableTypeCausesError) {
    recordSyntaxError(Message::Position(1, 26), "missing IDENT at '['");
    performTestExecution("module main() wire a(16), [2](4) skip");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateVariableIdentiferSharingSameVariableTypeInLocalModuleParameterDeclarationCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::DuplicateVariableDeclaration>(Message::Position(1, 32), "a");
    performTestExecution("module main() wire a(16), b(8), a(4) skip");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateVariableIdentiferSharingSameVariableTypeInSeparateDeclarationInLocalModuleParameterDeclarationCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::DuplicateVariableDeclaration>(Message::Position(1, 36), "a");
    performTestExecution("module main() wire a(16), b(8) wire a(4) skip");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateVariableIdentiferUsingDifferentVariableTypeInLocalModuleParameterDeclarationCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::DuplicateVariableDeclaration>(Message::Position(1, 37), "a");
    performTestExecution("module main() wire a(16) state b(8), a(4) skip");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateVariableIdentiferBetweenModuleParameterAndLocalVariableInSeparateLocalVariableDeclaration) {
    buildAndRecordExpectedSemanticError<SemanticError::DuplicateVariableDeclaration>(Message::Position(1, 38), "a");
    performTestExecution("module main(in a(16)) wire b(8) state a(4) skip");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateVariableIdentiferBetweenModuleParameterAndLocalVariableDeclarationWithLatterDefiningMultipleVariablesOfSameType) {
    buildAndRecordExpectedSemanticError<SemanticError::DuplicateVariableDeclaration>(Message::Position(1, 44), "a");
    performTestExecution("module main(in a(16)) wire c(4) state b(8), a(4) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidSymbolInLocalModuleVariableIdentifierDefinedInSeparateDeclarationCausesError) {
    recordSyntaxError(Message::Position(1, 20), "extraneous input '-' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main() wire a-2(16) state b(4) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidSymbolInLocalModuleVariableIdentifierDefinedInDeclarationSharingVariableTypesCausesError) {
    recordSyntaxError(Message::Position(1, 27), "extraneous input '#' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main() wire a(16), b#2 skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableValuesForDimensionOpeningBracketInLocalModuleParameterDeclarationCausesError) {
    recordSyntaxError(Message::Position(1, 21), "extraneous input ']' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main() wire a2](4) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidVariableValuesForDimensionOpeningBracketInLocalModuleParameterDeclarationCausesError) {
    recordSyntaxError(Message::Position(1, 20), "token recognition error at: '{'");
    recordSyntaxError(Message::Position(1, 21), "extraneous input '2' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main() wire a{2] skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableValuesForDimensionClosingBracketInLocalModuleParameterDeclarationCausesError) {
    recordSyntaxError(Message::Position(1, 23), "missing ']' at 'skip'");
    performTestExecution("module main() wire a[2 skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidVariableValuesForDimensionClosingBracketInLocalModuleParameterDeclarationCausesError) {
    recordSyntaxError(Message::Position(1, 22), "mismatched input ')' expecting ']'");
    performTestExecution("module main() wire a[2) skip");
}

TEST_F(SyrecParserErrorTestsFixture, NoneNumericValueForNumberOfValuesForDimensionInLocalModuleParameterDeclarationCausesError) {
    recordSyntaxError(Message::Position(1, 31), "mismatched input '#' expecting INT");
    performTestExecution("module main() wire b(4) wire a[#b) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingValueForNumberOfValuesForDimensionInLocalModuleVariableDeclarationCausesError) {
    recordSyntaxError(Message::Position(1, 21), "missing INT at ']'");
    performTestExecution("module main() wire a[] skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketOfBitwidthInLocalModuleVariableDeclarationCausesError) {
    recordSyntaxError(Message::Position(1, 22), "extraneous input ')' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main() wire a16) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketOfBitwidthInLocalModuleVariableDeclarationCausesError) {
    recordSyntaxError(Message::Position(1, 22), "token recognition error at: '}'");
    performTestExecution("module main() wire a16} skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketOfBitwidthInLocalModuleVariableDeclarationCausesError) {
    recordSyntaxError(Message::Position(1, 24), "missing ')' at 'skip'");
    performTestExecution("module main() wire a(16 skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketOfBitwidthInModuleVariableDeclarationCausesError) {
    recordSyntaxError(Message::Position(1, 23), "token recognition error at: '}'");
    recordSyntaxError(Message::Position(1, 25), "missing ')' at 'skip'");
    performTestExecution("module main() wire a(16} skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingLocalModuleVariableBitwidthValueWhenBracketsWereDefinedCausesError) {
    recordSyntaxError(Message::Position(1, 21), "missing INT at ')'");
    performTestExecution("module main() wire a() skip");
}

TEST_F(SyrecParserErrorTestsFixture, NoneNumericLocalModuleVariableBitwidthCausesError) {
    recordSyntaxError(Message::Position(1, 21), "mismatched input 'test' expecting INT");
    performTestExecution("module main() wire a(test) skip");
}

TEST_F(SyrecParserErrorTestsFixture, DeclaredLocalVariableBitwidthLongerThanMaximumSupportedValueCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::DeclaredVariableBitwidthTooLarge>(Message::Position(1, 41), 45, 32);
    buildAndRecordExpectedSemanticError<SemanticError::DeclaredVariableBitwidthTooLarge>(Message::Position(1, 52), 33, 32);
    performTestExecution("module main(inout b(2), out c(4)) wire x(45) wire t(33) skip");
}

TEST_F(SyrecParserErrorTestsFixture, DeclaredLocalVariableBitwidthInSignalListLongerThanMaximumSupportedValueCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::DeclaredVariableBitwidthTooLarge>(Message::Position(1, 50), 45, 32);
    buildAndRecordExpectedSemanticError<SemanticError::DeclaredVariableBitwidthTooLarge>(Message::Position(1, 67), 33, 32);
    performTestExecution("module main(inout b(2), out c(4)) wire test(2), x(45), y(4) wire t(33), z skip");
}

TEST_F(SyrecParserErrorTestsFixture, LocalVariableBitwidthTakenFromUserConfigurationLongerThanMaximumSupportedValueCausesError) {
    constexpr unsigned int defaultSignalBitwidth           = 33;
    const auto             userProvidedParserConfiguration = syrec::ReadProgramSettings(defaultSignalBitwidth);

    buildAndRecordExpectedSemanticError<SemanticError::DeclaredVariableBitwidthTooLarge>(Message::Position(1, 39), defaultSignalBitwidth, 32);
    buildAndRecordExpectedSemanticError<SemanticError::DeclaredVariableBitwidthTooLarge>(Message::Position(1, 46), defaultSignalBitwidth, 32);
    performTestExecution("module main(inout b(2), out c(4)) wire x wire t skip", userProvidedParserConfiguration);
}

TEST_F(SyrecParserErrorTestsFixture, LocalVariableBitwidthTakenFromUserConfigurationInSignalListLongerThanMaximumSupportedValueCausesError) {
    constexpr unsigned int defaultSignalBitwidth           = 33;
    const auto             userProvidedParserConfiguration = syrec::ReadProgramSettings(defaultSignalBitwidth);

    buildAndRecordExpectedSemanticError<SemanticError::DeclaredVariableBitwidthTooLarge>(Message::Position(1, 48), defaultSignalBitwidth, 32);
    buildAndRecordExpectedSemanticError<SemanticError::DeclaredVariableBitwidthTooLarge>(Message::Position(1, 61), defaultSignalBitwidth, 32);
    performTestExecution("module main(inout b(2), out c(4)) wire test(2), x, y(4) wire t, z(1) skip", userProvidedParserConfiguration);
}

// Tests for production call-statement
// TODO: All call statements should be repeated with the uncall keyword
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

// TODO: Modifiable parameter overlap tests (i.e. module x(inout a(4), out b(4)) a <=> b ... module main() wire t(4) call x(t, t)
TEST_F(SyrecParserErrorTestsFixture, OmittingCallStatementParameterClosingBracket) {
    recordSyntaxError(Message::Position(1, 108), "extraneous input '<EOF>' expecting {',', ')'}");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call add(a, b, c");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidCallStatementParameterClosingBracket) {
    recordSyntaxError(Message::Position(1, 108), "extraneous input ']' expecting {',', ')'}");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call add(a, b, c]");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingUncallStatementParameterClosingBracket) {
    recordSyntaxError(Message::Position(1, 110), "extraneous input '<EOF>' expecting {',', ')'}");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall add(a, b, c");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidUncallStatementParameterClosingBracket) {
    recordSyntaxError(Message::Position(1, 110), "extraneous input ']' expecting {',', ')'}");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall add(a, b, c]");
}

// TODO: Detection of one-level infinite recursion for call statements that are not defined in branches of if-statement (or in if-statements branches whos guard condition evaluates to a constant value)
// TODO: Printing user caller signature in semantic error message
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

// Tests for production for-statement
TEST_F(SyrecParserErrorTestsFixture, OmittingForStatementKeywordCausesError) {
    recordSyntaxError(Message::Position(1, 31), "extraneous input '$' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    recordSyntaxError(Message::Position(1, 34), "no viable alternative at input 'i ='");
    performTestExecution("module main(in a(4), out b(4)) $i = 0 to 3 do b.$i ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidForStatementKeywordCausesError) {
    recordSyntaxError(Message::Position(1, 31), "mismatched input 'do' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main(in a(4), out b(4)) do $i = 0 to 3 do b.$i ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingLoopVariableIdentPrefixCausesError) {
    recordSyntaxError(Message::Position(1, 35), "mismatched input 'i' expecting {'$', '#', '(', INT}");
    performTestExecution("module main(in a(4), out b(4)) for i = 0 to 3 do b.$i ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingLoopVariableInitialValueInitializationEqualSignCausesError) {
    recordSyntaxError(Message::Position(1, 38), "no viable alternative at input '$i 0'");
    performTestExecution("module main(in a(4), out b(4)) for $i 0 to 3 do b.$i ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericLoopVariableInitialValueInitializationCausesError) {
    recordSyntaxError(Message::Position(1, 40), "mismatched input 'b' expecting {'$', '#', '(', INT}");
    performTestExecution("module main(in a(4), out b(4)) for $i = b to 3 do b.$i ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableInInitialValueInitializationUsingSingleOperandCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ValueOfLoopVariableNotUsableInItsInitialValueDeclaration>(Message::Position(1, 40), "$i");
    performTestExecution("module main(in a(4), out b(4)) for $i = $i to 3 do b.$i ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableInInitialValueInitializationUsingExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ValueOfLoopVariableNotUsableInItsInitialValueDeclaration>(Message::Position(1, 78), "$i");
    performTestExecution("module main(in a(4), out b(4)) for $j = 0 to 2 step 1 do for $i = ((2 - $j) * $i) to 3 do b.$i ^= 1 rof rof");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingToKeywordInLoopVariableDefinitionCausesError) {
    recordSyntaxError(Message::Position(1, 42), "missing 'to' at '3'");
    performTestExecution("module main(in a(4), out b(4)) for $i = 0 3 do b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingToKeywordInForLoopIterationNumbersCausesError) {
    recordSyntaxError(Message::Position(1, 37), "no viable alternative at input '0 3'");
    performTestExecution("module main(in a(4), out b(4)) for 0 3 do b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericLoopIterationNumberStartValueInLoopVariableInitializationCausesError) {
    recordSyntaxError(Message::Position(1, 38), "mismatched input 'b' expecting {'$', '#', '(', INT}");
    performTestExecution("module main(in a(4), out b(4)) for $i=b to 3 do b.$i ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericLoopIterationNumberStartValueCausesError) {
    recordSyntaxError(Message::Position(1, 35), "mismatched input 'b' expecting {'$', '#', '(', INT}");
    performTestExecution("module main(in a(4), out b(4)) for b to 3 do b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericLoopIterationNumberEndValueCausesError) {
    recordSyntaxError(Message::Position(1, 40), "mismatched input 'b' expecting {'$', '#', '(', INT}");
    performTestExecution("module main(in a(4), out b(4)) for 2 to b do b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericLoopVariableEndValueCausesError) {
    recordSyntaxError(Message::Position(1, 45), "mismatched input 'b' expecting {'$', '#', '(', INT}");
    performTestExecution("module main(in a(4), out b(4)) for $i = 0 to b do b.$i ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidForLoopStepSizeKeywordCausesError) {
    recordSyntaxError(Message::Position(1, 37), "no viable alternative at input '3 incr'");
    performTestExecution("module main(in a(4), out b(4)) for 3 incr 1 do b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericLoopVariableStepsizeValueCausesError) {
    recordSyntaxError(Message::Position(1, 42), "mismatched input 'b' expecting {'-', '$', '#', '(', INT}");
    performTestExecution("module main(in a(4), out b(4)) for 3 step b do b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsingNonMinusSymbolAfterStepKeywordCausesError) {
    recordSyntaxError(Message::Position(1, 44), "extraneous input '-' expecting 'do'");
    performTestExecution("module main(in a(4), out b(4)) for 3 step 1 - do b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsingMultipleMinusSymbolsAfterStepkeywordCausesError) {
    recordSyntaxError(Message::Position(1, 43), "extraneous input '-' expecting {'$', '#', '(', INT}");
    performTestExecution("module main(in a(4), out b(4)) for 3 step --1 do b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingDoKeywordAfterLoopHeaderDefinitionCausesError) {
    recordSyntaxError(Message::Position(1, 44), "missing 'do' at 'b'");
    performTestExecution("module main(in a(4), out b(4)) for 3 step 1 b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidDoKeywordAfterLoopHeaderDefinitionCausesError) {
    recordSyntaxError(Message::Position(1, 45), "mismatched input 'loop' expecting {'do', 'step'}");
    performTestExecution("module main(in a(4), out b(4)) for $i=0 to 3 loop b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, EmptyLoopBodyCausesError) {
    recordSyntaxError(Message::Position(1, 48), "mismatched input 'rof' expecting {'++=', '--=', '~=', 'call', 'uncall', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main(in a(4), out b(4)) for $i=0 to 3 do rof");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateDefinitionOfLoopVariableInNestedLoopCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::DuplicateVariableDeclaration>(Message::Position(1, 53), "$i");
    performTestExecution("module main(in a(4), out b(4)) for $i=0 to 3 do for $i=1 to 3 do skip rof rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNonNumericExpressionInLoopVariableInitializationCausesError) {
    recordSyntaxError(Message::Position(1, 41), "mismatched input 'b' expecting {'$', '#', '(', INT}");
    performTestExecution("module main(in a(4), out b(4)) for $i = (b - 2) to 3 step 1 do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNonNumericExpressionInLoopVariableEndValueDefinitionCausesError) {
    recordSyntaxError(Message::Position(1, 46), "mismatched input 'b' expecting {'$', '#', '(', INT}");
    performTestExecution("module main(in a(4), out b(4)) for $i = 0 to (b - 2) step 1 do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNonNumericExpressionInLoopVariableStepsizeDefinitionCausesError) {
    recordSyntaxError(Message::Position(1, 43), "mismatched input 'b' expecting {'$', '#', '(', INT}");
    performTestExecution("module main(in a(4), out b(4)) for 3 step (b - 2) do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNonNumericExpressionInForLoopIterationNumberStartValueDefinitionCausesError) {
    recordSyntaxError(Message::Position(1, 36), "no viable alternative at input '(b'");
    performTestExecution("module main(in a(4), out b(4)) for (b - 2) step 1 do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNonNumericExpressionInForLoopIterationNumberEndValueDefinitionCausesError) {
    recordSyntaxError(Message::Position(1, 41), "mismatched input 'b' expecting {'$', '#', '(', INT}");
    performTestExecution("module main(in a(4), out b(4)) for 0 to (b - 2) step 1 do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNonNumericExpressionInForLoopIterationNumberStepsizeDefinitionCausesError) {
    recordSyntaxError(Message::Position(1, 48), "mismatched input 'b' expecting {'$', '#', '(', INT}");
    performTestExecution("module main(in a(4), out b(4)) for 0 to 3 step (b - 2) do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingRofKeywordAfterLoopBodyCausesError) {
    recordSyntaxError(Message::Position(1, 52), "missing 'rof' at '<EOF>'");
    performTestExecution("module main(in a(4), out b(4)) for 3 step 1 do  ++=b");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidRofKeywordAfterLoopBodyCausesError) {
    recordSyntaxError(Message::Position(1, 51), "no viable alternative at input 'done;'");
    recordSyntaxError(Message::Position(1, 57), "missing 'rof' at '<EOF>'");
    performTestExecution("module main(in a(4), out b(4)) for 3 step 1 do done; ++=b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfStepsizeWithConstantValueOfZeroWithConstantValuedStartAndEndCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::InfiniteLoopDetected>(Message::Position(1, 14), 1, 3, 0);
    performTestExecution("module main() for 1 to 3 step 0 do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfStepsizeWithEvaluatedValueOfZeroWithConstantValuedStartAndEndCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::InfiniteLoopDetected>(Message::Position(1, 21), 2, 0, 0);
    performTestExecution("module main(in b(2)) for 2 to 0 step ((2 - 1) * 0) do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfStepsizeWithConstantValueOfZeroWithImplicitStartAndConstantValueOfEndCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::InfiniteLoopDetected>(Message::Position(1, 14), 0, 2, 0);
    performTestExecution("module main() for 2 step 0 do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfStepsizeWithEvaluatedValueOfZeroWithImplicitStartAndEvaluatedValueOfEndCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::InfiniteLoopDetected>(Message::Position(1, 21), 0, 3, 0);
    performTestExecution("module main(in b(2)) for (#b + 1) step (#b * 0) do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfStepsizeWithConstantValueOfZeroWithEvaluatedConstantValueForStartAndConstantValueOfEndCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::InfiniteLoopDetected>(Message::Position(1, 21), 0, 2, 0);
    performTestExecution("module main(in b(2)) for (#b - 2) to 2 step 0 do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfStepsizeWithConstantValueOfZeroWithEvaluatedConstantValuesForStartAndEndValueCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::InfiniteLoopDetected>(Message::Position(1, 21), 0, 1, 0);
    performTestExecution("module main(in b(2)) for (#b - 2) to (2 - 1) step 0 do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfStepsizeWithConstantValueOfZeroWithConstantValueForStartAndEvaluatedConstantValueOfEndCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::InfiniteLoopDetected>(Message::Position(1, 21), 1, 3, 0);
    performTestExecution("module main(in b(2)) for 1 to (#b + 1) step 0 do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfStepsizeWithEvaluatedConstantValueOfZeroWithEvaluatedConstantValueForStartAndConstantValueOfEndCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::InfiniteLoopDetected>(Message::Position(1, 21), 0, 2, 0);
    performTestExecution("module main(in b(2)) for (#b - 2) to 2 step (2 - #b) do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfStepsizeWithEvaluatedConstantValueOfZeroWithEvaluatedConstantValuesForStartAndEndValueCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::InfiniteLoopDetected>(Message::Position(1, 21), 0, 1, 0);
    performTestExecution("module main(in b(2)) for (#b - 2) to (2 - 1) step ((#b - 1) - 1) do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfStepsizeWithEvaluatedConstantValueOfZeroWithConstantValueForStartAndEvaluatedConstantValueOfEndCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::InfiniteLoopDetected>(Message::Position(1, 21), 1, 3, 0);
    performTestExecution("module main(in b(2)) for 1 to (#b + 1) step ((#b + 10) * 0) do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfStepsizeWithConstantValueOfZeroWithTruncatedNegativeEvaluatedConstantValueForStartAndConstantValueOfEndCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::InfiniteLoopDetected>(Message::Position(1, 21), UINT_MAX, 1, 0);
    performTestExecution("module main(in b(2)) for $i = (#b - 3) to 1 step 0 do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfStepsizeWithConstantValueOfZeroWithConstantValueOfStartAndTruncatedNegativeEvaluatedConstantValueOfEndCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::InfiniteLoopDetected>(Message::Position(1, 21), 0, UINT_MAX, 0);
    performTestExecution("module main(in b(2)) for $i = 0 to (#b - 3) step 0 do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfStepsizeWithTruncatedNegativeEvaluatedConstantValueForStartAndEndValueCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::InfiniteLoopDetected>(Message::Position(1, 21), UINT_MAX - 1, UINT_MAX, 0);
    performTestExecution("module main(in b(2)) for $i = (#b - 4) to (#b - 3) step 0 do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfStepsizeWithConstantValueOfZeroWithTruncatedOverflowingEvaluatedConstantValueForStartAndConstantValueOfEndCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::InfiniteLoopDetected>(Message::Position(1, 21), 1, 2, 0);
    performTestExecution("module main(in b(2)) for $i = ((#b - 3) + 2) to 2 step 0 do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfStepsizeWithConstantValueOfZeroWithImplicitConstantValueForStartAndTruncatedOverflowingEvaluatedConstantValueForEndCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::InfiniteLoopDetected>(Message::Position(1, 21), 0, 1, 0);
    performTestExecution("module main(in b(2)) for ((#b - 3) + 2) step 0 do skip rof");
}

// TODO: Negative value for stepsize determined for number expression caught correctly
// TODO: Negative stepsize values should only be explicitily be definable if the value is defined as an integer constant
// because the current IR representation cannot store the minus sign for constant expressions (or we extend the constant expression in that case with an
// additional constant expression of the form (X * (0 - 1)) since we cannot explicitly use the value -1 (due to values being stored as unsigned integers)

// Tests for production if-statement
TEST_F(SyrecParserErrorTestsFixture, OmittingIfKeywordCausesError) {
    recordSyntaxError(Message::Position(1, 22), "extraneous input '(' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    recordSyntaxError(Message::Position(1, 25), "no viable alternative at input 'a >'");
    performTestExecution("module main(out a(4)) (a > 2) then skip else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidIfKeywordCausesError) {
    recordSyntaxError(Message::Position(1, 24), "extraneous input '-' expecting {'!', '~', '$', '#', '(', IDENT, INT}");
    recordSyntaxError(Message::Position(1, 30), "mismatched input '(' expecting 'then'");
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 25), "cond");
    performTestExecution("module main(out a(4)) if-cond (a > 2) then skip else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketOfIfStatementInNonConstantValueGuardExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 27), "mismatched input '>' expecting 'then'");
    performTestExecution("module main(out a(4)) if a > 2) then skip else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketOfIfStatementInNonConstantValueGuardExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 25), "token recognition error at: '{'");
    recordSyntaxError(Message::Position(1, 28), "mismatched input '>' expecting 'then'");
    performTestExecution("module main(out a(4)) if {a > 2) then skip else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketOfIfStatementInNonConstantValueGuardExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 32), "missing ')' at 'then'");
    performTestExecution("module main(out a(4)) if (a > 2 then skip else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketOfIfStatementInNonConstantValueGuardExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 31), "token recognition error at: '}'");
    recordSyntaxError(Message::Position(1, 33), "missing ')' at 'then'");
    performTestExecution("module main(out a(4)) if (a > 2} then skip else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingThenKeywordAfterGuardConditionCausesError) {
    recordSyntaxError(Message::Position(1, 33), "missing 'then' at 'skip'");
    performTestExecution("module main(out a(4)) if (a > 2) skip else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidThenKeywordAfterGuardConditionCausesError) {
    recordSyntaxError(Message::Position(1, 33), "mismatched input 'do' expecting 'then'");
    performTestExecution("module main(out a(4)) if (a > 2) do skip else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingElseKeywordAfterGuardConditionCausesError) {
    recordSyntaxError(Message::Position(1, 43), "missing 'else' at 'skip'");
    performTestExecution("module main(out a(4)) if (a > 2) then skip skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidElseKeywordAfterGuardConditionCausesError) {
    recordSyntaxError(Message::Position(1, 43), "missing 'else' at 'elif'");
    performTestExecution("module main(out a(4)) if (a > 2) then skip elif skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingFiKeywordAfterGuardConditionCausesError) {
    recordSyntaxError(Message::Position(1, 52), "mismatched input '<EOF>' expecting 'fi'");
    performTestExecution("module main(out a(4)) if (a > 2) then skip else skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidFiKeywordAfterGuardConditionCausesError) {
    recordSyntaxError(Message::Position(1, 53), "missing 'fi' at 'done'");
    recordSyntaxError(Message::Position(1, 58), "extraneous input '(' expecting {<EOF>, 'module'}");
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 53), "done");
    performTestExecution("module main(out a(4)) if (a > 2) then skip else skip done (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketOfIfStatementInNonConstantValueClosingGuardExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 58), "extraneous input '>' expecting {<EOF>, 'module'}");
    performTestExecution("module main(out a(4)) if (a > 2) then skip else skip fi a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketOfIfStatementInNonConstantValueClosingGuardExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 56), "token recognition error at: '{'");
    recordSyntaxError(Message::Position(1, 59), "extraneous input '>' expecting {<EOF>, 'module'}");
    performTestExecution("module main(out a(4)) if (a > 2) then skip else skip fi {a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketOfIfStatementInNonConstantValueClosingGuardExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 62), "missing ')' at '<EOF>'");
    performTestExecution("module main(out a(4)) if (a > 2) then skip else skip fi (a > 2");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketOfIfStatementInNonConstantValueClosingGuardExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 62), "mismatched input ']' expecting ')'");
    performTestExecution("module main(out a(4)) if (a > 2) then skip else skip fi (a > 2]");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchBetweenGuardAndClosingGuardConditionBasedOnBitwidthCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 58));
    performTestExecution("module main(out a(4)) if (a.0 > 2) then skip else skip fi (a.1:2 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchBetweenGuardAndClosingGuardConditionBasedOnAccessedValueOfDimensionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 62));
    performTestExecution("module main(out a[2](4)) if (a[0] > 2) then skip else skip fi (a[1] > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchBetweenGuardAndClosingGuardConditionBasedOnNumberOfAccessedDimensionsCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 65));
    performTestExecution("module main(out a[2][3](4)) if (a[0] > 2) then skip else skip fi (a[1][0] > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchBetweenGuardAndClosingGuardConditionBasedOnOperandOrderCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 62));
    performTestExecution("module main(out a[2](4)) if (a[0] > 2) then skip else skip fi (2 < a[1])");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchBetweenGuardAndClosingGuardConditionBasedOnTypeOfExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 62));
    performTestExecution("module main(out a[2](4)) if (a[0] > 2) then skip else skip fi ((a[0] << 2) > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchBetweenGuardAndClosingGuardConditionBasedOnConstantValuesCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 70));
    performTestExecution("module main(out a[2](4), out b(2)) if (#a > 2) then skip else skip fi (#b > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchBetweenGuardAndClosingGuardConditionUsingLoopVariablesCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 96));
    performTestExecution("module main(out a[2](4), out b(2)) for $i = 0 to 2 step 1 do if ($i > 2) then skip else skip fi ($i << 2) rof");
}

// TODO: Should numeric expressions that evaluate to the same value but are defined using a different structure be considered as usable in the guard/closing-guard condition of the if statement?
// TODO: Should omitting of the accessed value of a 1-D signal in the guard condition while the closing guard condition explicitly defined the accessed value of the dimension be causing an error?

// TODO: Guard expression missmatch semantic error
TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInGuardExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::TooFewDimensionsAccessed>(Message::Position(1, 32), 1, 2);
    buildAndRecordExpectedSemanticError<SemanticError::TooManyDimensionsAccessed>(Message::Position(1, 66), 3, 2);
    performTestExecution("module main(out a[2][1](4)) if (a[0] > 2) then skip else skip fi (a[0][0][1] > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableWithNotCompletelySpecifiedDimensionAccessInGuardExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::TooFewDimensionsAccessed>(Message::Position(1, 32), 1, 2);
    buildAndRecordExpectedSemanticError<SemanticError::TooFewDimensionsAccessed>(Message::Position(1, 66), 1, 2);
    performTestExecution("module main(out a[2][3](4)) if (a[1] > 2) then skip else skip fi (a[1] > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, EmptyTrueBranchInIfStatementCausesError) {
    recordSyntaxError(Message::Position(1, 38), "extraneous input 'else' expecting {'++=', '--=', '~=', 'call', 'uncall', 'for', 'if', 'skip', IDENT}");
    recordSyntaxError(Message::Position(1, 48), "mismatched input 'fi' expecting 'else'");
    performTestExecution("module main(out a(4)) if (a > 2) then else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, EmptyFalseBranchInIfStatementCausesError) {
    recordSyntaxError(Message::Position(1, 48), "mismatched input 'fi' expecting {'++=', '--=', '~=', 'call', 'uncall', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main(out a(4)) if (a > 2) then skip else fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, SemanticErrorInNotTakenTrueBranchWithConstantValueGuardConditionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 51), "c");
    performTestExecution("module main(in a(4)) wire b(4) if (2 < 1) then --= c else ++= b fi (2 < 1)");
}

TEST_F(SyrecParserErrorTestsFixture, SemanticErrorInNotTakenFalseBranchWithConstantValueGuardConditionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 62), "c");
    performTestExecution("module main(in a(4)) wire b(4) if (2 > 1) then ++= b else --= c fi (2 > 1)");
}

TEST_F(SyrecParserErrorTestsFixture, SemanticErrorInGuardConditionSubexpressionThatCouldBeSkippedDueToShortCircuitEvaluationCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 47), "c");
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 92), "c");
    performTestExecution("module main(in a(4)) wire b(4) if ((2 > 1) || (c > a)) then ++= b else skip fi ((2 > 1) || (c > a))");
}

// TODO: Guard expression missmatch semantic error
TEST_F(SyrecParserErrorTestsFixture, SemanticErrorInClosingGuardConditionReportedWhenGuardConditionWasWithoutError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 63), "b");
    performTestExecution("module main(out a[2](4)) if (a[0] > 2) then skip else skip fi (b[0] > 2)");
}

// Tests for production unary-statement
TEST_F(SyrecParserErrorTestsFixture, UsageOfReadonlyVariableInUnaryStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::AssignmentToReadonlyVariable>(Message::Position(1, 25), "b");
    performTestExecution("module main(in b(4)) ++= b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableInUnaryStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 18), "b");
    performTestExecution("module main() ++= b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUnknownUnaryAssignmentOperationInUnaryStatementCausesError) {
    recordSyntaxError(Message::Position(1, 22), "mismatched input '*' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main(out b(4)) *= b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfExpressionInUnaryStatementCausesError) {
    recordSyntaxError(Message::Position(1, 28), "extraneous input '(' expecting IDENT");
    recordSyntaxError(Message::Position(1, 31), "extraneous input '-' expecting {<EOF>, 'module'}");
    performTestExecution("module main(inout b(4)) ++= (b - 2)");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableInUnaryAssignmentCausesError) {
    recordSyntaxError(Message::Position(1, 51), "extraneous input '$' expecting IDENT");
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 52), "i");
    performTestExecution("module main(in b(4)) for $i = 0 to 3 step 1 do ++= $i rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInEvaluatedVariableAccessInUnaryAssignmentCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::OmittingDimensionAccessOnlyPossibleFor1DSignalWithSingleValue>(Message::Position(1, 31));
    performTestExecution("module main(inout b[2](4)) ++= b");
}

// TODO: Overlapping index in accessed values per dimension not possible
// TODO: Should accessed variable bitwidths between statement operands match (i.e. a.0:2 += (2 + b.0))

// Tests for production assign-statement
TEST_F(SyrecParserErrorTestsFixture, UsageOfReadonlyVariableOnLhsOfAssignStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::AssignmentToReadonlyVariable>(Message::Position(1, 31), "a");
    performTestExecution("module main(in a(4), out b(4)) a += b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableOnLhsOfAssignStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 22), "a");
    performTestExecution("module main(out b(4)) a += b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableOnRhsOfAssignStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 39), "c");
    performTestExecution("module main(inout a(4), out b(4)) a += c");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUnknownAssignOperationCausesError) {
    recordSyntaxError(Message::Position(1, 24), "no viable alternative at input 'a :'");
    performTestExecution("module main(out b(4)) a := b");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingEqualSignFromAssignOperationCausesError) {
    recordSyntaxError(Message::Position(1, 24), "no viable alternative at input 'a +'");
    performTestExecution("module main(out b(4)) a + b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableAsLhsOperandOfAssignmentCausesError) {
    recordSyntaxError(Message::Position(1, 48), "extraneous input '$' expecting {'++=', '--=', '~=', 'call', 'uncall', 'for', 'if', 'skip', IDENT}");
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 49), "i");
    performTestExecution("module main(out b(4)) for $i = 0 to 3 step 1 do $i += b rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInEvaluatedVariableAccessInLhsOperandOfAssignmentCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::OmittingDimensionAccessOnlyPossibleFor1DSignalWithSingleValue>(Message::Position(1, 25));
    performTestExecution("module main(out b[2](4)) b += 2");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInBitwidthsOfOperandsOfAssignmentCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 41), 1, 2);
    performTestExecution("module main(inout a(4), out b(4)) a.1 += b.2:3");
}

// Tests for production swap-statement
TEST_F(SyrecParserErrorTestsFixture, UsageOfReadonlyVariableOnLhsOfSwapStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::AssignmentToReadonlyVariable>(Message::Position(1, 31), "a");
    performTestExecution("module main(in a(4), out b(4)) a <=> b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableOnLhsOfSwapStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 31), "c");
    performTestExecution("module main(in a(4), out b(4)) c <=> b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfReadonlyVariableOnRhsOfSwapStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::AssignmentToReadonlyVariable>(Message::Position(1, 37), "a");
    performTestExecution("module main(in a(4), out b(4)) b <=> a");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableOnRhsOfSwapStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 40), "c");
    performTestExecution("module main(inout a(4), out b(4)) a <=> c");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfExpressionOnLhsOfSwapStatementCausesError) {
    recordSyntaxError(Message::Position(1, 34), "extraneous input '(' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    recordSyntaxError(Message::Position(1, 37), "no viable alternative at input 'a -'");
    performTestExecution("module main(inout a(4), out b(4)) (a - 2) <=> b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfExpressionOnRhsOfSwapStatementCausesError) {
    recordSyntaxError(Message::Position(1, 40), "extraneous input '(' expecting IDENT");
    recordSyntaxError(Message::Position(1, 43), "extraneous input '-' expecting {<EOF>, 'module'}");
    performTestExecution("module main(inout a(4), out b(4)) a <=> (b - 2)");
}

// TODO: Combinations for bitwidth missmatches between full signal bitwidth, bit and bitrange access combinations for operands of binary expression and assignment statement.
TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsDefiningNoBitAccessAndRhsDefiningBitAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 40), 4, 1);
    performTestExecution("module main(inout a(4), out b(4)) a <=> b.1");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsDefiningNoBitAccessAndRhsDefiningBitrangeAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 40), 4, 2);
    performTestExecution("module main(inout a(4), out b(4)) a <=> b.0:1");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsDefiningNoBitAccessAndRhsDefiningBitrangeWithStartLargerThanEndAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 40), 4, 2);
    performTestExecution("module main(inout a(4), out b(4)) a <=> b.1:0");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingBitAccessAndRhsDefiningNoBitrangeAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 42), 1, 4);
    performTestExecution("module main(inout a(4), out b(4)) a.1 <=> b");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingBitAccessAndRhsDefiningBitrangeAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 42), 1, 2);
    performTestExecution("module main(inout a(4), out b(4)) a.1 <=> b.2:3");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingBitAccessAndRhsDefiningBitrangeWithStartLargerThanEndAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 42), 1, 2);
    performTestExecution("module main(inout a(4), out b(4)) a.1 <=> b.3:2");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingBitrangeAccessAndRhsDefiningNoBitrangeAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 44), 2, 4);
    performTestExecution("module main(inout a(4), out b(4)) a.1:2 <=> b");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingBitrangeAccessWithStartLargerThanEndAndRhsDefiningNoBitrangeAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 44), 2, 4);
    performTestExecution("module main(inout a(4), out b(4)) a.2:1 <=> b");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingBitrangeAccessAndRhsDefiningBitAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 44), 2, 1);
    performTestExecution("module main(inout a(4), out b(4)) a.2:3 <=> b.1");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingBitrangeAccessWithStartLargerThanEndAndRhsDefiningBitAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 44), 2, 1);
    performTestExecution("module main(inout a(4), out b(4)) a.3:2 <=> b.1");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingBitrangeAccessAndRhsDefiningBitrangeAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 44), 2, 3);
    performTestExecution("module main(inout a(4), out b(4)) a.0:1 <=> b.1:3");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingBitrangeAccessAndRhsDefiningBitrangeWithStartLargerThanEndAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 44), 2, 3);
    performTestExecution("module main(inout a(4), out b(4)) a.0:1 <=> b.3:1");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingBitrangeAccessWithStartLargerThanEndAndRhsDefiningBitrangeWithStartLargerThanEndAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 44), 2, 3);
    performTestExecution("module main(inout a(4), out b(4)) a.1:0 <=> b.3:1");
}

TEST_F(SyrecParserErrorTestsFixture, Non1DSignalInEvaluatedVaribleAccessOfLhsSwapOperandCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::OmittingDimensionAccessOnlyPossibleFor1DSignalWithSingleValue>(Message::Position(1, 37));
    performTestExecution("module main(inout a[2](4), out b(4)) a <=> b");
}

TEST_F(SyrecParserErrorTestsFixture, Non1DSignalInEvaluatedVaribleAccessOfRhsSwapOperandCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::OmittingDimensionAccessOnlyPossibleFor1DSignalWithSingleValue>(Message::Position(1, 43));
    performTestExecution("module main(inout a(4), out b[2](4)) a <=> b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfConstantOperandAsLhsSwapOperandCausesError) {
    recordSyntaxError(Message::Position(1, 22), "mismatched input '2' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main(out b(4)) 2 <=> b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfConstantOperandAsRhsSwapOperandCausesError) {
    recordSyntaxError(Message::Position(1, 28), "mismatched input '2' expecting IDENT");
    performTestExecution("module main(out b(4)) b <=> 2");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidSwapOperationCausesError) {
    recordSyntaxError(Message::Position(1, 36), "no viable alternative at input 'a ='");
    performTestExecution("module main(inout a(4), out b(4)) a => b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableAsLhsOperandOfSwapOperationCausesError) {
    recordSyntaxError(Message::Position(1, 41), "extraneous input '$' expecting {'++=', '--=', '~=', 'call', 'uncall', 'for', 'if', 'skip', IDENT}");
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 42), "i");
    performTestExecution("module main(out b(4)) for $i = 0 to 3 do $i <=> b rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableAsRhsOperandOfSwapOperationCausesError) {
    recordSyntaxError(Message::Position(1, 47), "extraneous input '$' expecting IDENT");
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 48), "i");
    performTestExecution("module main(out b(4)) for $i = 0 to 3 do b <=> $i rof");
}

// TODO: Add tests for overlapping operands of swap operation (tests which were already defined for the assignment statement and can be copied).
// The question is whether the whole set of tests need to be repeated for the swap statement?

// Tests for production binary-expression
// TODO: Tests for truncation of values larger than the expected bitwidth
// TODO: If short circuit evaluation of a binary expression can be performed, should semantic errors be reported? 
TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableInBinaryExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 28), "a");
    performTestExecution("module main(out b(4)) b += (a - 2)");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUnknownBinaryOperationInBinaryExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 39), "no viable alternative at input '(a <=>'");
    performTestExecution("module main(out b(4), in a(4)) b += (a <=> 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketOfBinaryExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 38), "extraneous input '+' expecting {<EOF>, 'module'}");
    performTestExecution("module main(out b(4), in a(4)) b += a + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketOfBinaryExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 36), "extraneous input '[' expecting {'!', '~', '$', '#', '(', IDENT, INT}");
    recordSyntaxError(Message::Position(1, 39), "extraneous input '+' expecting {<EOF>, 'module'}");
    performTestExecution("module main(out b(4), in a(4)) b += [a + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketOfBinaryExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 42), "missing ')' at '<EOF>'");
    performTestExecution("module main(out b(4), in a(4)) b += (a + 2");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketOfBinaryExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 42), "mismatched input ']' expecting ')'");
    performTestExecution("module main(out b(4), in a(4)) b += (a + 2]");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInEvaluatedVariableAccessInLhsOperandOfBinaryExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::OmittingDimensionAccessOnlyPossibleFor1DSignalWithSingleValue>(Message::Position(1, 41));
    performTestExecution("module main(out a(4), out b[2](4)) a += (b + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInEvaluatedVariableAccessInRhsOperandOfBinaryExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::OmittingDimensionAccessOnlyPossibleFor1DSignalWithSingleValue>(Message::Position(1, 45));
    performTestExecution("module main(out a(4), out b[2](4)) a += (2 - b)");
}

// TODO: Tests for nested expressions

// Tests for production unary-expression
// TODO: Add tests when IR supports unary expressions

// Tests for production shift-expression
// TODO: Tests for truncation of values larger than the expected bitwidth
TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableInLhsOperandOfShiftExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 42), "c");
    performTestExecution("module main(out a(4), out b[2](4)) a += ((c << 2) + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableInRhsOperandOfShiftExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 51), "c");
    performTestExecution("module main(out a(4), out b[2](4)) a += ((b[0] << #c) + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidShiftOperationCausesError) {
    recordSyntaxError(Message::Position(1, 47), "no viable alternative at input '((b[0] <=>'");
    performTestExecution("module main(out a(4), out b[2](4)) a += ((b[0] <=> 2) + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingLhsOperandOfShiftExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 42), "no viable alternative at input '((<<'");
    performTestExecution("module main(out a(4), out b[2](4)) a += ((<< #b) + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingRhsOperandOfShiftExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 56), "mismatched input ')' expecting {'$', '#', '(', INT}");
    performTestExecution("module main(out a(4), out b[2](4)) a += (b[1] + (b[0] >>))");
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericExpressionInRhsOperandOfShiftExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 51), "no viable alternative at input '((b[0] << (b'");
    performTestExecution("module main(out a(4), out b[2](4)) a += ((b[0] << (b[1] - 2) + 2))");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableUsedAsLhsOperandOfShiftOperationCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::OmittingDimensionAccessOnlyPossibleFor1DSignalWithSingleValue>(Message::Position(1, 42));
    performTestExecution("module main(out a(4), out b[2](4)) a += ((b >> #a) + 2)");
}

// Tests for production number
TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableInNumericExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 43), "c");
    performTestExecution("module main(out a(4), out b[2](4)) ++= a[(#c - 2)]");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidOperationInNumericExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 45), "mismatched input '<<' expecting {'+', '-', '*', '/'}");
    performTestExecution("module main(out a(8), out b[2](2)) ++= a.(#b << 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketOfNumericExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 31), "mismatched input '-' expecting ']'");
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedValueForDimensionOutOfRange>(Message::Position(1, 28), 4, 0, 1);
    performTestExecution("module main(out a(4)) ++= a[#a - 4)]");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidOpeningBracketInNumericExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 31), "token recognition error at: '{'");
    recordSyntaxError(Message::Position(1, 35), "mismatched input '-' expecting ']'");
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedValueForDimensionOutOfRange>(Message::Position(1, 32), 4, 0, 4);
    performTestExecution("module main(out a[4](4)) ++= a[{#a - 2)]");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketOfNumericExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 48), "missing ')' at '<EOF>'");
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 41), 6, 4);
    performTestExecution("module main(out a(4), out b[2](4)) ++= a.(#a + 2");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidClosingBracketInNumericExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 48), "mismatched input ']' expecting ')'");
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 41), 6, 4);
    performTestExecution("module main(out a(4), out b[2](4)) ++= a.(#a + 2]");
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInNumericExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 46));
    performTestExecution("module main(out a(4), out b[2](4)) ++= a.(2 / 0)");
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInEvaluatedNumericExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 46));
    performTestExecution("module main(out a(4), out b[2](4)) ++= a.(2 / (#a - 4))");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredLoopVariableInNumericExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 70), "$j");
    performTestExecution("module main(out a(4), out b[2](4)) for $i = 0 to 3 step 1 do ++= a.(($j + 2) / 2) rof");
}

TEST_F(SyrecParserErrorTestsFixture, AccessingBitwidthOfUnknownVariableCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 31), "b");
    performTestExecution("module main(out a[1](4)) a += #b");
}

TEST_F(SyrecParserErrorTestsFixture, AccessingBitwidthOfLoopVariableCausesError) {
    recordSyntaxError(Message::Position(1, 50), "mismatched input '(' expecting IDENT");
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 52), "i");
    performTestExecution("module main(out a[1](4)) for $i = 0 to 3 do a += #($i) rof");
}

TEST_F(SyrecParserErrorTestsFixture, AccessingBitwidthOfConstantCausesError) {
    recordSyntaxError(Message::Position(1, 31), "mismatched input '5' expecting IDENT");
    performTestExecution("module main(out a[1](4)) a += #5");
}

// Tests for production signal
// TODO: Tests for indices out of range and division by zero errors in dynamic expressions (i.e. loop variables evaluated at compile time)
// TODO: Tests for truncation of values larger than the expected bitwidth
// TODO: Tests for out of range indices for variable with no explicit dimension and bitwidth declaration
TEST_F(SyrecParserErrorTestsFixture, OmittingVariableIdentifierInVariableAccessCausesError) {
    recordSyntaxError(Message::Position(1, 26), "missing IDENT at '.'");
    performTestExecution("module main(out a(4)) ++= .0");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketForAccessOnDimensionOfVariableCausesError) {
    recordSyntaxError(Message::Position(1, 31), "extraneous input ']' expecting {<EOF>, 'module'}");
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 29), "a1");
    performTestExecution("module main(out a[2](4)) ++= a1].0");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketForAccessOnDimensionOfVariableCausesError) {
    recordSyntaxError(Message::Position(1, 30), "extraneous input '(' expecting {<EOF>, 'module'}");
    buildAndRecordExpectedSemanticError<SemanticError::OmittingDimensionAccessOnlyPossibleFor1DSignalWithSingleValue>(Message::Position(1, 29));
    performTestExecution("module main(out a[2](4)) ++= a(1].0");
}

TEST_F(SyrecParserErrorTestsFixture, IndexForAccessedValueOfDimensionOutOfRangeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedValueForDimensionOutOfRange>(Message::Position(1, 31), 2, 0, 2);
    performTestExecution("module main(out a[2](4)) ++= a[2].0");
}

TEST_F(SyrecParserErrorTestsFixture, IndexForAccessedValueOfDimensionInNonConstantExpressionOutOfRangeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedValueForDimensionOutOfRange>(Message::Position(1, 31), 4, 0, 2);
    performTestExecution("module main(out a[2](4)) ++= a[#a].0");
}

TEST_F(SyrecParserErrorTestsFixture, NumberOfAccessedDimensionsOfVariableOutOfRangeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::TooManyDimensionsAccessed>(Message::Position(1, 29), 2, 1);
    performTestExecution("module main(out a[2](4)) ++= a[0][1].0");
}

TEST_F(SyrecParserErrorTestsFixture, NumberOfAccessedDimensionsOfVariableImplicitlyDeclaredAs1DVariableWithSingleValueOutOfRangeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::TooManyDimensionsAccessed>(Message::Position(1, 26), 2, 1);
    performTestExecution("module main(out a(4)) ++= a[0][1].0");
}

TEST_F(SyrecParserErrorTestsFixture, IndexForAccessedValueDimensionsOfVariableImplicitlyDeclaredAs1DVariableWithSingleValueOutOfRangeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedValueForDimensionOutOfRange>(Message::Position(1, 28), 2, 0, 1);
    performTestExecution("module main(out a(4)) ++= a[2].0");
}


TEST_F(SyrecParserErrorTestsFixture, None1DSizeOfVariableAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::OmittingDimensionAccessOnlyPossibleFor1DSignalWithSingleValue>(Message::Position(1, 29));
    performTestExecution("module main(out a[2](4)) ++= a.0");
}

TEST_F(SyrecParserErrorTestsFixture, None1DAccessOnExpressionForValueOfDimensionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::OmittingDimensionAccessOnlyPossibleFor1DSignalWithSingleValue>(Message::Position(1, 44));
    performTestExecution("module main(out a[2](4), out b[2](2)) ++= a[b].0");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketForAccessOnDimensionOfVariableCausesError) {
    recordSyntaxError(Message::Position(1, 32), "missing ']' at '.'");
    performTestExecution("module main(out a[2](4)) ++= a[0.0");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketForAccessOnDimensionOfVariableCausesError) {
    recordSyntaxError(Message::Position(1, 32), "token recognition error at: '}'");
    recordSyntaxError(Message::Position(1, 33), "missing ']' at '.'");
    performTestExecution("module main(out a[2](4)) ++= a[0}.0");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingBitrangeStartSymbolCausesError) {
    recordSyntaxError(Message::Position(1, 33), "extraneous input '0' expecting {<EOF>, 'module'}");
    performTestExecution("module main(out a[2](4)) ++= a[0]0");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidBitrangeStartSymbolCausesError) {
    recordSyntaxError(Message::Position(1, 33), "extraneous input ':' expecting {<EOF>, 'module'}");
    performTestExecution("module main(out a[2](4)) ++= a[0]:0");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidBitrangEndSymbolCausesError) {
    recordSyntaxError(Message::Position(1, 35), "extraneous input '.' expecting {<EOF>, 'module'}");
    performTestExecution("module main(out a[2](4)) ++= a[0].0.0");
}

// TODO: Division by zero error are tested in tests for production 'number', should we explicitly tests the same behaviour for every usage of the production in other productions?
TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInDynamicBitrangeStartValueExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 33));
    performTestExecution("module main(out a(4)) ++= a.(2 / (#a - 4))");
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInDynamicBitrangeEndValueExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 35));
    performTestExecution("module main(out a(4)) ++= a.0:(2 / (#a - 4))");
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInDynamicExpressionForAccessValueOfDimensionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 36));
    performTestExecution("module main(out a[2](4)) ++= a[(2 / (#a - 4))]");
}

TEST_F(SyrecParserErrorTestsFixture, BitrangeStartValueIsConstantAndOutOfRangeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 28), 5, 4);
    performTestExecution("module main(out a(4)) ++= a.5");
}

TEST_F(SyrecParserErrorTestsFixture, BitrangeEndValueIsConstantAndOutOfRangeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 30), 5, 4);
    performTestExecution("module main(out a(4)) ++= a.0:5");
}

TEST_F(SyrecParserErrorTestsFixture, BitrangeStartAndEndValueIsConstantAndOutOfRangeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 28), 7, 4);
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 30), 5, 4);
    performTestExecution("module main(out a(4)) ++= a.7:5");
}

TEST_F(SyrecParserErrorTestsFixture, BitrangeStartValueIsDynamicExpressionAndOutOfRangeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 28), 5, 4);
    performTestExecution("module main(out a(4)) ++= a.(#a + 1):3");
}

TEST_F(SyrecParserErrorTestsFixture, BitrangeEndValueIsDynamicExpressionAndOutOfRangeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 30), 5, 4);
    performTestExecution("module main(out a(4)) ++= a.0:(#a + 1)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingExplicitDimensionAccessOn1DSignalWithMultipleValuesCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::OmittingDimensionAccessOnlyPossibleFor1DSignalWithSingleValue>(Message::Position(1, 29));
    performTestExecution("module main(out a[2](4)) ++= a");
}

TEST_F(SyrecParserErrorTestsFixture, AccessingTooFewDimensionsOfNDSignalInVariableAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::TooFewDimensionsAccessed>(Message::Position(1, 32), 1, 2);
    performTestExecution("module main(out a[2][3](4)) ++= a[0]");
}

TEST_F(SyrecParserErrorTestsFixture, AccessingTooManyDimensionsOfNDSignalInVariableAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::TooManyDimensionsAccessed>(Message::Position(1, 32), 3, 2);
    performTestExecution("module main(out a[2][3](4)) ++= a[0][1][2]");
}

TEST_F(SyrecParserErrorTestsFixture, AccessingOutOfRangeBitOfVariableWithBitwidthTakenFromUserConfiguration) {
    constexpr unsigned int defaultSignalBitwidth           = 3;
    const auto             userProvidedParserConfiguration = syrec::ReadProgramSettings(defaultSignalBitwidth);

    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 25), 5, defaultSignalBitwidth);
    performTestExecution("module main(out a) ++= a.5", userProvidedParserConfiguration);
}

TEST_F(SyrecParserErrorTestsFixture, AccessingOutOfRangeStartBitInBitrangeWithKnownBoundsOfVariableWithBitwidthTakenFromUserConfiguration) {
    constexpr unsigned int defaultSignalBitwidth           = 3;
    const auto             userProvidedParserConfiguration = syrec::ReadProgramSettings(defaultSignalBitwidth);

    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 25), 5, defaultSignalBitwidth);
    performTestExecution("module main(out a) ++= a.5:2", userProvidedParserConfiguration);
}

TEST_F(SyrecParserErrorTestsFixture, AccessingOutOfRangeEndBitInBitrangeWithKnownBoundsOfVariableWithBitwidthTakenFromUserConfiguration) {
    constexpr unsigned int defaultSignalBitwidth           = 3;
    const auto             userProvidedParserConfiguration = syrec::ReadProgramSettings(defaultSignalBitwidth);

    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 27), 5, defaultSignalBitwidth);
    performTestExecution("module main(out a) ++= a.2:5", userProvidedParserConfiguration);
}

TEST_F(SyrecParserErrorTestsFixture, AccessingOutOfRangeStartAndEndBitInBitrangeWithKnownBoundsOfVariableWithBitwidthTakenFromUserConfiguration) {
    constexpr unsigned int defaultSignalBitwidth = 3;
    const auto             userProvidedParserConfiguration = syrec::ReadProgramSettings(defaultSignalBitwidth);

    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 25), 7, defaultSignalBitwidth);
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 27), 5, defaultSignalBitwidth);
    performTestExecution("module main(out a) ++= a.7:5", userProvidedParserConfiguration);
}

TEST_F(SyrecParserErrorTestsFixture, AccessingOutOfRangeStartBitInBitrangeWithUnknownEndBoundOfVariableWithBitwidthTakenFromUserConfiguration) {
    constexpr unsigned int defaultSignalBitwidth           = 3;
    const auto             userProvidedParserConfiguration = syrec::ReadProgramSettings(defaultSignalBitwidth);

    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 44), 5, defaultSignalBitwidth);
    performTestExecution("module main(out a) for $i = 0 to 3 do ++= a.5:$i rof", userProvidedParserConfiguration);
}

TEST_F(SyrecParserErrorTestsFixture, AccessingOutOfRangeEndBitInBitrangeWithUnknownStartBoundOfVariableWithBitwidthTakenFromUserConfiguration) {
    constexpr unsigned int defaultSignalBitwidth           = 3;
    const auto             userProvidedParserConfiguration = syrec::ReadProgramSettings(defaultSignalBitwidth);

    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 47), 5, defaultSignalBitwidth);
    performTestExecution("module main(out a) for $i = 0 to 3 do ++= a.$i:5 rof", userProvidedParserConfiguration);
}

TEST_F(SyrecParserErrorTestsFixture, AccessingOutOfRangeStartBitInBitrangeWithUnknownEndBoundOfVariableWithExplicitlyDeclaredBitwidth) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 47), 5, 3);
    performTestExecution("module main(out a(3)) for $i = 0 to 3 do ++= a.5:$i rof");
}

TEST_F(SyrecParserErrorTestsFixture, AccessingOutOfRangeEndBitInBitrangeWithUnknownStartBoundOfVariableWithExplicitlyDeclaredBitwidth) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 50), 5, 3);
    performTestExecution("module main(out a(3)) for $i = 0 to 3 do ++= a.$i:5 rof");
}

TEST_F(SyrecParserErrorTestsFixture, AccessingOutOfRangeBitWithEvaluationOfValueLeadingToTruncationOfValueCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 28), UINT_MAX, 3);
    performTestExecution("module main(out a(3)) ++= a.(#a - 4)");
}

TEST_F(SyrecParserErrorTestsFixture, AccessingOutOfRangeBitWithEvaluationOfValueLeadingToOverflowOfValueCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 28), 4, 3);
    performTestExecution("module main(out a(3)) ++= a.((#a - 4) + 5)");
}

TEST_F(SyrecParserErrorTestsFixture, AccessingOutOfRangeBitrangeWithEvaluationOfStartValueLeadingToTruncationOfValueCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 28), UINT_MAX, 3);
    performTestExecution("module main(out a(3)) ++= a.(#a - 4):2");
}

TEST_F(SyrecParserErrorTestsFixture, AccessingOutOfRangeBitrangeWithEvaluationOfStartValueLeadingToOverflowOfValueCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 28), 4, 3);
    performTestExecution("module main(out a(3)) ++= a.((#a - 4) + 5):2");
}

TEST_F(SyrecParserErrorTestsFixture, AccessingOutOfRangeBitrangeWithEvaluationOfEndValueLeadingToTruncationOfValueCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 30), UINT_MAX, 3);
    performTestExecution("module main(out a(3)) ++= a.1:(#a - 4)");
}

TEST_F(SyrecParserErrorTestsFixture, AccessingOutOfRangeBitrangeWithEvaluationOfEndValueLeadingToOverflowOfValueCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 30), 4, 3);
    performTestExecution("module main(out a(3)) ++= a.1:((#a - 4) + 5)");
}

TEST_F(SyrecParserErrorTestsFixture, AccessingOutOfRangeValueOfDimensionWithEvaluationOfEndValueLeadingToTruncationOfValueCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedValueForDimensionOutOfRange>(Message::Position(1, 28), UINT_MAX, 0, 1);
    performTestExecution("module main(out a(3)) ++= a[(#a - 4)]");
}

TEST_F(SyrecParserErrorTestsFixture, AccessingOutOfRangeValueOfDimensionWithEvaluationOfEndValueLeadingToOverflowOfValueCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedValueForDimensionOutOfRange>(Message::Position(1, 28), 4, 0, 1);
    performTestExecution("module main(out a(3)) ++= a[((#a - 4) + 5)]");
}