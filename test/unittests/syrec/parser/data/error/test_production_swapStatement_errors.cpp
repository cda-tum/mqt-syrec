#include "test_syrec_parser_errors_base.hpp"

// TODO: Add tests for overlapping operands of swap operation (tests which were already defined for the assignment statement and can be copied).
// The question is whether the whole set of tests need to be repeated for the swap statement?

//// TODO: Combinations for bitwidth missmatches between full signal bitwidth, bit and bitrange access combinations for operands of binary expression and assignment statement.

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

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingBitAccessWithUnknownValueForIndexAndRhsBeingBitrangeAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 62), 1, 3);
    performTestExecution("module main(inout a(4), out b(4)) for $i = 0 to 3 do a.$i <=> b.3:1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingBitAccessWithUnknownValueForIndexAndRhsBeingFullBitwidthAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 62), 1, 4);
    performTestExecution("module main(inout a(4), out b(4)) for $i = 0 to 3 do a.$i <=> b rof");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingBitrangeAccessAndRhsBeingBitAccessWithUnknownValueForIndexAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 63), 3, 1);
    performTestExecution("module main(inout a(4), out b(4)) for $i = 0 to 3 do a.3:1 <=> b.$i rof");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingFullBitwidthAccessAndRhsBeingBitAccessWithUnknownValueForIndexAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 59), 4, 1);
    performTestExecution("module main(inout a(4), out b(4)) for $i = 0 to 3 do a <=> b.$i rof");
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
