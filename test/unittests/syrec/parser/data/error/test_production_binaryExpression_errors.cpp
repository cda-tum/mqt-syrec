#include "test_syrec_parser_errors_base.hpp"

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

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInModuloOperationWithConstantValueDivisiorCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 41));
    performTestExecution("module main(out a(4), out b[2](4)) a += (2 % 0)");
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInModuloOperationWithConstantExpressionDivisiorCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 41));
    performTestExecution("module main(out a(4), out b[2](4)) a += (2 % (#b - 4))");
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInUpperBitMultiplicationOperationWithConstantValueDivisiorCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 41));
    performTestExecution("module main(out a(4), out b[2](4)) a += (2 *> 0)");
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInUpperBitMultiplicationOperationWithConstantExpressionDivisiorCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 41));
    performTestExecution("module main(out a(4), out b[2](4)) a += (2 *> (#b - 4))");
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroDetectedDueToTruncationOfConstantValuesUsingModuloOperationInLhsOfBinaryExpression) {
    const auto customParserConfig = syrec::ReadProgramSettings(32, utils::IntegerConstantTruncationOperation::Modulo);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 55));
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 104));
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 1 do if ((b.$i:1 / 3) + a.0) then ++= a.0:1 else skip fi ((b.$i:1 / 3) + a.0) rof", customParserConfig);
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroDetectedDueToTruncationOfConstantValuesUsingModuloOperationInRhsOfBinaryExpression) {
    const auto customParserConfig = syrec::ReadProgramSettings(32, utils::IntegerConstantTruncationOperation::Modulo);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 55));
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 104));
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 1 do if (a.0 + (b.$i:1 / 3)) then ++= a.0:1 else skip fi (a.0 + (b.$i:1 / 3)) rof", customParserConfig);
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroDetectedDueToTruncationOfConstantValuesUsingBitwiseAndOperationInLhsOfBinaryExpression) {
    const auto customParserConfig = syrec::ReadProgramSettings(32, utils::IntegerConstantTruncationOperation::BitwiseAnd);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 55));
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 104));
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 1 do if ((b.$i:1 / 4) + a.0) then ++= a.0:1 else skip fi ((b.$i:1 / 4) + a.0) rof", customParserConfig);
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroDetectedDueToTruncationOfConstantValuesUsingBitwiseAndOperationInRhsOfBinaryExpression) {
    const auto customParserConfig = syrec::ReadProgramSettings(32, utils::IntegerConstantTruncationOperation::BitwiseAnd);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 55));
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 104));
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 1 do if (a.0 + (b.$i:1 / 4)) then ++= a.0:1 else skip fi (a.0 + (b.$i:1 / 4)) rof", customParserConfig);
}

TEST_F(SyrecParserErrorTestsFixture, SemanticErrorInLhsOperandOfBinaryExpressionReportedEvenIfExpressionCouldBeSimplified) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 33), "b");
    performTestExecution("module main(inout a(2)) a.0 += ((b.1 + 1) * (#a - 2))");
}

TEST_F(SyrecParserErrorTestsFixture, SemanticErrorInRhsOperandOfBinaryExpressionReportedEvenIfExpressionCouldBeSimplified) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 44), "b");
    performTestExecution("module main(inout a(2)) a.0 += ((#a - 2) * (b.1 + 1))");
}
