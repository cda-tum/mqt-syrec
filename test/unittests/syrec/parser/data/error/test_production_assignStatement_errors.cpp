#include "test_syrec_parser_errors_base.hpp"

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

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionSetByAssignmentLhsNotResetByUnknownBitwidthOfFutureOperandAndCausesErrorOnOperandBitwidthMissmatch) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 69), 1, 2);
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 1 do a.0 += (b.0:$i + a.1:2) rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionSetByAssignmentLhsNotResetByConstantExpressionAndCausesErrorOnOperandBitwidthMissmatch) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 69), 1, 2);
    performTestExecution("module main(inout a(4)) for $i = 0 to 3 step 1 do a.0 += (($i + 2) + a.1:2) rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionSetByAssignmentLhsSetDespiteIndexForAccessedValueOfDimensionHavingUnknownValueAndCausesErrorOnOperandBitwidthMissmatch) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 69), 1, 2);
    performTestExecution("module main(inout a[4](4)) for $i = 0 to 3 step 1 do a[$i].0 += (2 + a[0].1:2) rof");
}

TEST_F(SyrecParserErrorTestsFixture, VariableAccessWithUnknownAccessedBitrangeWidthOnAssignmentLhsDoesNotBlockFutureOperandBitwidthRestrictions) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 86), 2, 1);
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 3 step 1 do a.0:$i += ((b.0:1 << 2) + (b.1 + a.1:2)) rof");
}

TEST_F(SyrecParserErrorTestsFixture, VariableAccessWithUnknownAccessedBitOnAssignmentLhsDoesNotBlockFutureOperandBitwidthRestrictions) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 69), 1, 2);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 90), 1, 2);
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 3 step 1 do a.$i += ((b.0:1 << 2) + (b.1 + a.1:2)) rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionSetOnAssignmentLhsForUnknownAccessedBit) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 63), 1, 4);
    performTestExecution("module main(inout a[1](4), in b(4)) for $i = 0 to 1 do a.$i += b rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionFromDimensionAccessOnLhsOfAssignmentDoesNotResetRestrictionSetByEnclosingVariableAccessOfLhsAccessingBit) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 72), 1, 2);
    performTestExecution("module main(inout a[2](4), in b(2)) for $i = 0 to 1 do a[(b + $i)].0 += b rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionFromDimensionAccessOnLhsOfAssignmentDoesNotResetRestrictionSetByEnclosingVariableAccessOfLhsAccessingBitrange) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 74), 2, 4);
    performTestExecution("module main(inout a[2](4), in b(4)) for $i = 0 to 1 do a[(b + $i)].1:2 += b rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionFromDimensionAccessOnLhsOfAssignmentDoesNotResetRestrictionSetByEnclosingVariableAccessOfLhsAccessingFullbitwidth) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 70), 4, 1);
    performTestExecution("module main(inout a[2](4), in b(2)) for $i = 0 to 1 do a[(b + $i)] += b.0 rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionFromDimensionAccessOnLhsOfAssignmentDoesNotPropagateIfEnclosingVariableAccessOfLhsDoesNotSetRestriction) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 88), 4, 2);
    performTestExecution("module main(inout a[2](4), in b(2)) for $i = 0 to 1 do a[(b + 2)].0:$i += ((a[0] + 2) + b) rof");
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroDetectedDueToTruncationOfConstantValuesUsingModuloOperationInRhsOfAssignment) {
    const auto customParserConfig = syrec::ReadProgramSettings(32, utils::IntegerConstantTruncationOperation::Modulo);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 42));
    performTestExecution("module main(inout a(4), in b(2)) a.0:1 += ((b + 6) / 3)", customParserConfig);
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroDetectedDueToTruncationOfConstantValuesUsingBitwiseAndOperationInRhsOfAssignment) {
    const auto customParserConfig = syrec::ReadProgramSettings(32, utils::IntegerConstantTruncationOperation::BitwiseAnd);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 42));
    performTestExecution("module main(inout a(4), in b(2)) a.0:1 += ((b + 6) / 4)", customParserConfig);
}
