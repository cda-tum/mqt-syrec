#include "test_syrec_parser_errors_base.hpp"

#include <gtest/gtest.h>

#include "core/syrec/program.hpp"
#include "core/syrec/parser/utils/custom_error_messages.hpp"
#include "core/syrec/parser/utils/parser_messages_container.hpp"
#include "core/syrec/parser/utils/syrec_operation_utils.hpp"

using namespace syrec_parser_error_tests;

TEST_F(SyrecParserErrorTestsFixture, UsageOfVariableOfTypeInOnLhsOfAssignStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::AssignmentToReadonlyVariable>(Message::Position(1, 31), "a");
    performTestExecution("module main(in a(4), out b(4)) a += b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfVariableOfTypeStateOnLhsOfAssignStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::AssignmentToReadonlyVariable>(Message::Position(1, 33), "a");
    performTestExecution("module main(out b(4)) state a(4) a += b");
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
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 59), 1, 2);
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 1 do a.0 += (b.0:$i + a.1:2) rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionSetByAssignmentLhsNotResetByConstantExpressionAndCausesErrorOnOperandBitwidthMissmatch) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 57), 1, 2);
    performTestExecution("module main(inout a(4)) for $i = 0 to 3 step 1 do a.0 += (($i + 2) + a.1:2) rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionSetByAssignmentLhsSetDespiteIndexForAccessedValueOfDimensionHavingUnknownValueAndCausesErrorOnOperandBitwidthMissmatch) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 64), 1, 2);
    performTestExecution("module main(inout a[4](4)) for $i = 0 to 3 step 1 do a[$i].0 += (2 + a[0].1:2) rof");
}

TEST_F(SyrecParserErrorTestsFixture, VariableAccessWithUnknownAccessedBitrangeWidthOnAssignmentLhsDoesNotBlockFutureOperandBitwidthRestrictions) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 85), 2, 1);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 92), 1, 2);
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 3 step 1 do a.0:$i += ((b.0:1 << 2) + (b.1 + a.1:2)) rof");
}

TEST_F(SyrecParserErrorTestsFixture, VariableAccessWithUnknownAccessedBitOnAssignmentLhsDoesNotBlockFutureOperandBitwidthRestrictions) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 67), 1, 2);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 83), 2, 1);
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


TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToFullBitwidthWithRhsExprDefiningOnlyVariableAccessWithAccessedBitwidthSmallerThanLhs) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 38), 4, 2);
    performTestExecution("module main(inout a(4), in b(4)) a += b.0:1");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToFullBitwidthWithRhsExprDefiningOnlyVariableAccessWithAccessedBitwidthLargerThanLhs) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 38), 2, 3);
    performTestExecution("module main(inout a(2), in b(4)) a += b.3:1");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToFullBitwidthWithRhsExprDefiningOnlyVariableAccessWithAccessOnBitUsingConstantIndex) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 38), 4, 1);
    performTestExecution("module main(inout a(4), in b(2)) a += b.1");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToFullBitwidthWithRhsExprDefiningOnlyVariableAccessWithAccessOnUknownBit) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 57), 4, 1);
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 1 do a += b.$i rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToFullBitwidthWithRhsExprDefiningExpressionWhosAccessedBitrangeIsLargerThanLhs) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 53), 2, 4);
    performTestExecution("module main(inout a(2), in b(6)) a += ((2 + b.0:1) + (b.0:3 + b.1:4))");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToFullBitwidthWithRhsExprDefiningExpressionUsingArithmeticOperationWhosAccessedBitrangeIsSmallerThanLhs) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 58), 4, 2);
    performTestExecution("module main(inout a[2](4), in b(2)) a[0] += ((2 + a[1]) + (b.0:1 + b.1:0))");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToFullBitwidthWithRhsExprDefiningExpressionUsingRelationalOperationWhosAccessedBitrangeIsSmallerThanLhs) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 38), 4, 1);
    performTestExecution("module main(inout a(4), in b(2)) a += (b > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToFullBitwidthWithRhsExprDefiningExpressionUsingLogicalOperationWhosAccessedBitrangeIsSmallerThanLhs) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 44), 4, 1);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 58), 1, 2);
    performTestExecution("module main(inout a[2](4), in b(2)) a[0] += ((a[1] > 2) + (2 - b))");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToBitUsingConstantIndexWithRhsExprDefiningOnlyVariableAccessWithAccessedBitwidthLargerThanLhs) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 40), 1, 2);
    performTestExecution("module main(inout a(4), in b(2)) a.1 += b");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToBitUsingConstantIndexWithRhsExprDefiningExpressionWhosAccessedBitrangeIsLargerThanLhs) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 46), 1, 2);
    performTestExecution("module main(inout a[2](4), in b(2)) a[0].1 += ((a[1].0:1 + b) + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToUknownBitWithRhsExprDefiningOnlyVariableAccessWithAccessedBitwidthLargerThanLhs) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 60), 1, 3);
    performTestExecution("module main(inout a(2), in b(4)) for $i = 0 to 1 do a.$i += b.0:2 rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToUknownBitWithRhsExprDefiningExpressionWhosAccessedBitrangeIsLargerThanLhs) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 66), 1, 3);
    performTestExecution("module main(inout a[2](4), in b(3)) for $i = 0 to 3 do a[1].$i += ((a[0].3:1 + 2) + b) rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToBitrangeUsingConstantIndicesWithRhsExprDefiningOnlyVariableAccessWithAccessedBitwidthSmallerThanLhs) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 42), 3, 2);
    performTestExecution("module main(inout a(4), in b(6)) a.1:3 += b.1:0");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToBitrangeUsingConstantIndicesRhsExprDefiningOnlyVariableAccessWithAccessedBitwidthLargerThanLhs) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 42), 2, 3);
    performTestExecution("module main(inout a(4), in b(6)) a.1:0 += b.1:3");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToBitrangeUsingConstantIndicesRhsExprDefiningOnlyVariableAccessWithAccessOnBitUsingConstantIndex) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 42), 3, 1);
    performTestExecution("module main(inout a(4), in b(2)) a.1:3 += b.1");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToBitrangeUsingConstantIndicesRhsExprDefiningOnlyVariableAccessWithAccessOnUknownBit) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 61), 2, 1);
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 2 do a.1:2 += b.$i rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToBitrangeUsingConstantIndicesRhsExprDefiningExpressionWhosAccessedBitrangeIsLargerThanLhs) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 48), 2, 4);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 62), 4, 2);
    performTestExecution("module main(inout a[2](4), in b(6)) a[0].1:2 += ((a[1] + 2) + b.2:1)");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToBitrangeUsingConstantIndicesRhsExprDefiningExpressionUsingArithmeticOperationWhosAccessedBitrangeIsSmallerThanLhs) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 43), 2, 1);
    performTestExecution("module main(inout a(4)) wire b(4) a.0:1 += (b.1 + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToBitrangeUsingConstantIndicesRhsExprDefiningExpressionUsingRelationalOperationWhosAccessedBitrangeIsSmallerThanLhs) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 43), 2, 1);
    performTestExecution("module main(inout a(4)) wire b(4) a.0:1 += (b < 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionBetweenAssignmentToBitrangeUsingConstantIndicesRhsExprDefiningExpressionUsingLogicalOperationWhosAccessedBitrangeIsSmallerThanLhs) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 43), 2, 1);
    performTestExecution("module main(inout a(4)) wire b(4) a.0:1 += ((b > 2) || (b != 3))");
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


TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToFullSignalBitwidthOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingFullbitwidth) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 43), generateVariableAccessOverlappingIndicesDataContainer({0}, 0).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) a[0] += b[a[0]]");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToFullSignalBitwidthOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitWithConstantIndexValue) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 43), generateVariableAccessOverlappingIndicesDataContainer({0}, 1).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) a[0] += b[a[0].1]");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToFullSignalBitwidthUsingImplicitDimensionAccessOfLhsOfAssignmentInDimensionAccessOfVariableAccesOfRhsUsingImplicitDimensionAccess) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 40), generateVariableAccessOverlappingIndicesDataContainer({0}, 0).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) a += b[a]");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToFullSignalBitwidthUsingImplicitDimensionAccessOfLhsOfAssignmentInDimensionAccessOfVariableAccesOfRhsUsingExplicitDimensionAccess) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 40), generateVariableAccessOverlappingIndicesDataContainer({0}, 0).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) a += b[a[0]]");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToFullSignalBitwidthUsingExplicitDimensionAccessOfLhsOfAssignmentInDimensionAccessOfVariableAccesOfRhsUsingImplicitDimensionAccess) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 43), generateVariableAccessOverlappingIndicesDataContainer({0}, 0).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) a[0] += b[a]");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToFullSignalBitwidthOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeUsingConstantIndices) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 43), generateVariableAccessOverlappingIndicesDataContainer({0}, 1).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) a[0] += b[a[0].1:2]");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToFullSignalBitwidthOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantValueStartIndex) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 62), generateVariableAccessOverlappingIndicesDataContainer({0}, 1).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) for $i = 3 to 0 do a[0] += b[a[0].1:($i + 1)] rof");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToFullSignalBitwidthOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantValueEndIndex) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 62), generateVariableAccessOverlappingIndicesDataContainer({0}, 1).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) for $i = 3 to 0 do a[0] += b[a[0].($i + 1):1] rof");
}


TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitUsingConstantIndexValueOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingFullbitwidth) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 45), generateVariableAccessOverlappingIndicesDataContainer({0}, 2).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(1)) a[0].2 += b[a[0]]");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitUsingConstantIndexValueOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitWithConstantIndexValueAndBitIndicesMatching) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 45), generateVariableAccessOverlappingIndicesDataContainer({0}, 2).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(1)) a[0].2 += b[a[0].2]");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitUsingConstantIndexValueOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantIndicesWithBitEnclosedByBitrange) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 45), generateVariableAccessOverlappingIndicesDataContainer({0}, 2).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(1)) a[0].2 += b[a[0].0:3]");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitUsingConstantIndexValueOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantIndicesWithBitIndexEqualToBitrangeStartIndex) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 45), generateVariableAccessOverlappingIndicesDataContainer({0}, 2).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(1)) a[0].2 += b[a[0].2:3]");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitUsingConstantIndexValueOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantIndicesWithBitIndexEqualToBitrangeEndIndex) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 45), generateVariableAccessOverlappingIndicesDataContainer({0}, 3).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(1)) a[0].3 += b[a[0].2:3]");
}


TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantIndicesOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingFullbitwidth) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 47), generateVariableAccessOverlappingIndicesDataContainer({0}, 1).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(3)) a[0].1:3 += b[a]");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantIndicesOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitWithConstantIndexEnclosedByBitrange) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 47), generateVariableAccessOverlappingIndicesDataContainer({0}, 2).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(3)) a[0].1:3 += b[a.2]");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantIndicesOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitWithConstantIndexEqualToBitrangeStartIndex) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 47), generateVariableAccessOverlappingIndicesDataContainer({0}, 1).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(3)) a[0].1:3 += b[a.1]");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantIndicesOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitWithConstantIndexEqualToBitrangeEndIndex) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 47), generateVariableAccessOverlappingIndicesDataContainer({0}, 3).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(3)) a[0].1:3 += b[a.3]");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantIndicesOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantIndicesAndLhsBitrangeEnclosedByRhsOne) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 47), generateVariableAccessOverlappingIndicesDataContainer({0}, 2).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(6), in b(2)) a[0].2:3 += b[a.1:4]");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantIndicesOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantIndicesAndRhsBitrangeEnclosedByLhsOne) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 47), generateVariableAccessOverlappingIndicesDataContainer({0}, 2).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(6), in b(4)) a[0].1:4 += b[a.2:3]");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantIndicesOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantIndicesAndLhsBitrangeOverlapping) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 47), generateVariableAccessOverlappingIndicesDataContainer({0}, 1).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(6), in b(4)) a[0].1:4 += b[a.0:3]");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantIndicesOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantIndicesAndLhsBitrangesBeingEqual) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 47), generateVariableAccessOverlappingIndicesDataContainer({0}, 1).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(6), in b(4)) a[0].1:4 += b[a.4:1]");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantIndicesOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantStartIndexEnclosedByBitrange) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 66), generateVariableAccessOverlappingIndicesDataContainer({0}, 2).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(6), in b(4)) for $i = 0 to 3 do a[0].1:4 += b[a[0].2:($i + 1)] rof");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantIndicesOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantStartIndexEqualToBitrangeBounds) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 66), generateVariableAccessOverlappingIndicesDataContainer({0}, 4).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(6), in b(4)) for $i = 0 to 3 do a[0].4:1 += b[a[0].4:($i + 1)] rof");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantIndicesOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantEndIndexEnclosedByBitrange) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 66), generateVariableAccessOverlappingIndicesDataContainer({0}, 2).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(6), in b(4)) for $i = 0 to 3 do a[0].1:4 += b[a[0].($i + 1):2] rof");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantIndicesOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantEndIndexEqualToBitrangeBounds) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 66), generateVariableAccessOverlappingIndicesDataContainer({0}, 4).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(6), in b(4)) for $i = 0 to 3 do a[0].1:4 += b[a[0].($i + 1):4] rof");
}


TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantValueForStartIndexOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingFullbitwidth) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 67), generateVariableAccessOverlappingIndicesDataContainer({0}, 2).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) for $i = 3 to 0 do a[0].2:$i += b[a[0]] rof");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantValueForStartIndexOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitWithConstantIndicesMatching) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 67), generateVariableAccessOverlappingIndicesDataContainer({0}, 2).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) for $i = 3 to 0 do a[0].2:$i += b[a[0].2] rof");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantValueForStartIndexOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantIndicesAndLhsBitEnclosedByRhsOne) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 67), generateVariableAccessOverlappingIndicesDataContainer({0}, 2).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) for $i = 3 to 0 do a[0].2:$i += b[a[0].0:3] rof");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantValueForStartIndexOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantIndicesAndLhsBitEqualToStartIndex) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 67), generateVariableAccessOverlappingIndicesDataContainer({0}, 3).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) for $i = 3 to 0 do a[0].3:$i += b[a[0].0:3] rof");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantValueForStartIndexOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantStartIndexEqualToLhsIndex) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 67), generateVariableAccessOverlappingIndicesDataContainer({0}, 3).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) for $i = 3 to 0 do a[0].3:$i += b[a[0].3:(1 - $i)] rof");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantValueForStartIndexOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantEndIndexEqualToLhsIndex) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 67), generateVariableAccessOverlappingIndicesDataContainer({0}, 3).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) for $i = 3 to 0 do a[0].3:$i += b[a[0].(1 - $i):3] rof");
}


TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantValueForEndIndexOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingFullbitwidth) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 67), generateVariableAccessOverlappingIndicesDataContainer({0}, 2).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) for $i = 3 to 0 do a[0].$i:2 += b[a[0]] rof");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantValueForEndIndexOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitWithConstantIndicesMatching) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 67), generateVariableAccessOverlappingIndicesDataContainer({0}, 2).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) for $i = 3 to 0 do a[0].$i:2 += b[a[0].2] rof");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantValueForEndIndexOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantIndicesAndLhsBitEnclosedByRhsOne) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 67), generateVariableAccessOverlappingIndicesDataContainer({0}, 2).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) for $i = 3 to 0 do a[0].$i:2 += b[a[0].0:3] rof");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantValueForEndIndexOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantIndicesAndLhsBitEqualToStartIndex) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 67), generateVariableAccessOverlappingIndicesDataContainer({0}, 3).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) for $i = 3 to 0 do a[0].$i:3 += b[a[0].0:3] rof");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantValueForEndIndexOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantStartIndexEqualToLhsIndex) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 67), generateVariableAccessOverlappingIndicesDataContainer({0}, 3).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) for $i = 3 to 0 do a[0].$i:3 += b[a[0].3:(1 - $i)] rof");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingAccessOnAssignedToBitrangeUsingConstantValueForEndIndexOfLhsOfAssignmentInDimensionAccessOfVaribleAccessOfRhsAccessingBitrangeWithConstantEndIndexEqualToLhsIndex) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 67), generateVariableAccessOverlappingIndicesDataContainer({0}, 3).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a(4), in b(4)) for $i = 3 to 0 do a[0].$i:3 += b[a[0].(1 - $i):3] rof");
}


TEST_F(SyrecParserErrorTestsFixture, accessOnValueOfDimensionsOfAssignedToNDimensionalSignalOnRhsOfAssignmentWithAccessOnSameValuesOfDimensions) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 58), generateVariableAccessOverlappingIndicesDataContainer({0, 2, 1}, 0).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a[2][4][3](6), in b(6)) a[0][2][1] += b[a[0][2][1]]");
}

TEST_F(SyrecParserErrorTestsFixture, accessOnValueOfDimensionsOfAssignedToNDimensionalSignalOnRhsOfAssignmentUsingBitaccessWithConstantValueAndLhsAccessingFullbitwidthWithAcessOnSameValuesOfDimensions) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 58), generateVariableAccessOverlappingIndicesDataContainer({0, 2, 1}, 1).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a[2][4][3](6), in b(6)) a[0][2][1] += b[a[0][2][1].1]");
}

TEST_F(SyrecParserErrorTestsFixture, accessOnValueOfDimensionsOfAssignedToNDimensionalSignalOnRhsOfAssignmentUsingBitrangeAccessWithConstantIndicesAndLhsAccessingFullbitwidthWithAcessOnSameValuesOfDimensions) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 58), generateVariableAccessOverlappingIndicesDataContainer({0, 2, 1}, 1).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a[2][4][3](6), in b(6)) a[0][2][1] += b[a[0][2][1].1:2]");
}

TEST_F(SyrecParserErrorTestsFixture, accessOnValueOfDimensionsOfAssignedToNDimensionalSignalOnRhsOfAssignmentUsingBitrangeAccessWithStartIndexHavingConstantValueAndLhsAccessingFullbitwidthWithAcessOnSameValuesOfDimensions) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 84), generateVariableAccessOverlappingIndicesDataContainer({0, 2, 1}, 1).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a[2][4][3](6), in b(6)) for $i = 0 to 3 step 2 do a[0][2][1] += b[a[0][2][1].1:$i] rof");
}

TEST_F(SyrecParserErrorTestsFixture, accessOnValueOfDimensionsOfAssignedToNDimensionalSignalOnRhsOfAssignmentUsingBitAccessWithConstantValueAndLhsAccessingBitUsingConstantValueAndAccessedBitsOverlappingWithAcessOnSameValuesOfDimensions) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 60), generateVariableAccessOverlappingIndicesDataContainer({0, 2, 1}, 0).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a[2][4][3](6), in b(1)) a[0][2][1].0 += b[a[0][2][1].0]");
}

TEST_F(SyrecParserErrorTestsFixture, accessOnValueOfDimensionsOfAssignedToNDimensionalSignalOnRhsOfAssignmentUsingBitrangeAccessWithConstantValuesAndLhsAccessingBitUsingConstantValueAndAccessedBitEnclosedInBitrangeWithAcessOnSameValuesOfDimensions) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 60), generateVariableAccessOverlappingIndicesDataContainer({0, 2, 1}, 1).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a[2][4][3](6), in b(1)) a[0][2][1].1 += b[a[0][2][1].2:0]");
}

TEST_F(SyrecParserErrorTestsFixture, accessOnValueOfDimensionsOfAssignedToNDimensionalSignalOnRhsOfAssignmentUsingBitAccessWithConstantValuesAndLhsAccessingBitrangeUsingConstantIndicesWithAccessedBitEnclosedByBitrangeWithAcessOnSameValuesOfDimensions) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 62), generateVariableAccessOverlappingIndicesDataContainer({0, 2, 1}, 1).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a[2][4][3](6), in b(3)) a[0][2][1].2:0 += b[a[0][2][1].1]");
}

TEST_F(SyrecParserErrorTestsFixture, accessOnValueOfDimensionsOfAssignedToNDimensionalSignalOnRhsOfAssignmentUsingFullbitwidthAndLhsAccessingBitrangeUsingConstantIndicesWithAcessOnSameValuesOfDimensions) {
    buildAndRecordExpectedSemanticError<SemanticError::SynthesisOfExpressionPotentiallyNotPossibleDueToAccessOnRestrictedVariableParts>(
            Message::Position(1, 62), generateVariableAccessOverlappingIndicesDataContainer({0, 2, 1}, 2).stringifyOverlappingIndicesInformation());
    performTestExecution("module main(inout a[2][4][3](6), in b(3)) a[0][2][1].2:0 += b[a[0][2][1]]");
}