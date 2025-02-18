#include "test_syrec_parser_errors_base.hpp"

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

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroDetectedDueToTruncationOfConstantValuesUsingModuloOperationInLhsOfShiftExpression) {
    const auto customParserConfig = syrec::ReadProgramSettings(32, utils::IntegerConstantTruncationOperation::Modulo);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 55));
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 113));
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 1 do if (((b.$i:1 / 7) + a.0:2) << 2) then ++= a.0:1 else skip fi (((b.$i:1 / 7) + a.0:2) << 2) rof", customParserConfig);
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroDetectedDueToTruncationOfConstantValuesUsingBitwiseAndOperationInLhsOfShiftExpression) {
    const auto customParserConfig = syrec::ReadProgramSettings(32, utils::IntegerConstantTruncationOperation::BitwiseAnd);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 55));
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 113));
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 1 do if (((b.$i:1 / 8) + a.0:2) << 2) then ++= a.0:1 else skip fi (((b.$i:1 / 8) + a.0:2) << 2) rof", customParserConfig);
}

TEST_F(SyrecParserErrorTestsFixture, SemanticErrorInToBeShiftedExpressionReportedEvenIfShiftAmountWouldAllowSimplificationOfExpression) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 32), "b");
    performTestExecution("module main(inout a(2)) a.0 += (b.1 << 2)");
}
