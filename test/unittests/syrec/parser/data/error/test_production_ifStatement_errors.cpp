#include "test_syrec_parser_errors_base.hpp"

#include <gtest/gtest.h>

#include "core/syrec/parser/utils/custom_error_messages.hpp"
#include "core/syrec/parser/utils/parser_messages_container.hpp"

using namespace syrec_parser_error_tests;

TEST_F(SyrecParserErrorTestsFixture, OmittingIfKeywordCausesError) {
    recordSyntaxError(Message::Position(1, 22), "extraneous input '(' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    recordSyntaxError(Message::Position(1, 27), "no viable alternative at input 'a.1 >'");
    performTestExecution("module main(out a(4)) (a.1 > 2) then skip else skip fi (a.1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidIfKeywordCausesError) {
    recordSyntaxError(Message::Position(1, 24), "extraneous input '-' expecting {'!', '~', '$', '#', '(', IDENT, INT}");
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 25), "cond");
    recordSyntaxError(Message::Position(1, 30), "mismatched input '(' expecting 'then'");
    performTestExecution("module main(out a(4)) if-cond (a.1 > 2) then skip else skip fi (a.1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketOfIfStatementInNonConstantValueGuardExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 29), "mismatched input '>' expecting 'then'");
    performTestExecution("module main(out a(4)) if a.1 > 2) then skip else skip fi (a.1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketOfIfStatementInNonConstantValueGuardExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 25), "token recognition error at: '{'");
    recordSyntaxError(Message::Position(1, 30), "mismatched input '>' expecting 'then'");
    performTestExecution("module main(out a(4)) if {a.1 > 2) then skip else skip fi (a.1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketOfIfStatementInNonConstantValueGuardExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 34), "missing ')' at 'then'");
    performTestExecution("module main(out a(4)) if (a.1 > 2 then skip else skip fi (a.1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketOfIfStatementInNonConstantValueGuardExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 33), "token recognition error at: '}'");
    recordSyntaxError(Message::Position(1, 35), "missing ')' at 'then'");
    performTestExecution("module main(out a(4)) if (a.1 > 2} then skip else skip fi (a.1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingThenKeywordAfterGuardConditionCausesError) {
    recordSyntaxError(Message::Position(1, 35), "missing 'then' at 'skip'");
    performTestExecution("module main(out a(4)) if (a.1 > 2) skip else skip fi (a.1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidThenKeywordAfterGuardConditionCausesError) {
    recordSyntaxError(Message::Position(1, 35), "mismatched input 'do' expecting 'then'");
    performTestExecution("module main(out a(4)) if (a.1 > 2) do skip else skip fi (a.1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingElseKeywordAfterGuardConditionCausesError) {
    recordSyntaxError(Message::Position(1, 45), "missing 'else' at 'skip'");
    performTestExecution("module main(out a(4)) if (a.1 > 2) then skip skip fi (a.1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidElseKeywordAfterGuardConditionCausesError) {
    recordSyntaxError(Message::Position(1, 45), "missing 'else' at 'elif'");
    performTestExecution("module main(out a(4)) if (a.1 > 2) then skip elif skip fi (a.1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingFiKeywordAfterGuardConditionCausesError) {
    recordSyntaxError(Message::Position(1, 54), "mismatched input '<EOF>' expecting 'fi'");
    performTestExecution("module main(out a(4)) if (a.1 > 2) then skip else skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidFiKeywordAfterGuardConditionCausesError) {
    recordSyntaxError(Message::Position(1, 55), "missing 'fi' at 'done'");
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 55), "done");
    recordSyntaxError(Message::Position(1, 60), "extraneous input '(' expecting {<EOF>, 'module'}");
    performTestExecution("module main(out a(4)) if (a.1 > 2) then skip else skip done (a.1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketOfIfStatementInNonConstantValueClosingGuardExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 25));
    recordSyntaxError(Message::Position(1, 62), "extraneous input '>' expecting {<EOF>, 'module'}");
    performTestExecution("module main(out a(4)) if (a.1 > 2) then skip else skip fi a.1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketOfIfStatementInNonConstantValueClosingGuardExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 25));
    recordSyntaxError(Message::Position(1, 58), "token recognition error at: '{'");
    recordSyntaxError(Message::Position(1, 63), "extraneous input '>' expecting {<EOF>, 'module'}");
    performTestExecution("module main(out a(4)) if (a.1 > 2) then skip else skip fi {a.1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketOfIfStatementInNonConstantValueClosingGuardExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 66), "missing ')' at '<EOF>'");
    performTestExecution("module main(out a(4)) if (a.1 > 2) then skip else skip fi (a.1 > 2");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketOfIfStatementInNonConstantValueClosingGuardExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 66), "mismatched input ']' expecting ')'");
    performTestExecution("module main(out a(4)) if (a.1 > 2) then skip else skip fi (a.1 > 2]");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchInConstantValueOfAccessedValueOfDimension) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 30));
    performTestExecution("module main(inout a[4](1)) if (a[0] > 2) then ++= a[0] else --= a[0] fi (a[1] > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchInConstantExpressionValueOfAccessedValueOfDimension) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 30));
    performTestExecution("module main(inout a[4](4)) if (a[(#a - 2)].1 > 2) then ++= a[0] else --= a[0] fi (a[(#a - 1)].1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDueToOperandsMissmatchInConstantExpressionValueOfAccessedValueOfDimension) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 30));
    performTestExecution("module main(inout a[4](4)) if (a[(#a - 2)].1 > 2) then ++= a[0] else --= a[0] fi (a[(#a - 1)].1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDueToOperationMissmatchInConstantExpressionValueOfAccessedValueOfDimension) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 30));
    performTestExecution("module main(inout a[4](1)) if (a[(#a / 2)] > 2) then ++= a[0] else --= a[0] fi (a[(#a - 1)] > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDueToDifferentLoopVariablesUsedInAccessedValueOfDimension) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 82));
    performTestExecution("module main(inout a[4](1)) for $i = 0 to 3 step 1 do for $j = 0 to 2 step 1 do if a[$i] then ++= a[0] else --= a[0] fi a[$j] rof rof");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchInShiftExpressionOfAccessedValueOfDimension) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 39));
    performTestExecution("module main(inout a[4](1), in b(2)) if a[(b << 2)] then ++= a[0] else --= a[0] fi a[(b >> 3)]");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDueToOperationMissmatchInShiftExpressionOfAccessedValueOfDimension) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 51));
    performTestExecution("module main(inout a[4](1), in b(2), inout c(2)) if a[(b >> 2)] then ++= a[0] else --= a[0] fi a[(c >> 2)]");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchInBinaryExpressionOfAccessedValueOfDimension) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 51));
    performTestExecution("module main(inout a[1](4), in b(4), in c[3](1)) if c[(a > b)] then ++= a[0] else --= a[0] fi c[(b > a)]");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDueToOperationMissmatchInBinaryExpressionOfAccessedValueOfDimension) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 51));
    performTestExecution("module main(inout a[1](4), in b(4), in c[3](1)) if c[(a > b)] then ++= a[0] else --= a[0] fi c[(a < b)]");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchWithNoExplicitBitrangeAccessInGuardAndBitAccessInClosingGuardCondition) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 30));
    performTestExecution("module main(inout a[1](4)) if (a.1 > 2) then ++= a[0] else --= a[0] fi (a.0 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchWithNoExplicitBitrangeAccessInGuardAndBitrangeAccessInClosingGuardCondition) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 30));
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 72), 1, 3);
    performTestExecution("module main(inout a[1](4)) if (a.1 > 2) then ++= a[0] else --= a[0] fi (a.0:2 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDueToDifferentBitBeingAccessedUsingConstantIndex) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 27));
    performTestExecution("module main(in b[1](2)) if b[0].1 then skip else skip fi b[0].0");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDueToDifferentBitBeingAccessedUsingConstantExpression) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 53));
    performTestExecution("module main(in b[1](3)) for $i = 0 to 2 step 1 do if b[0].($i + 1) then skip else skip fi b[0].($i - 1) rof");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDueToDifferentBitrangeBeingAccessedUsingConstantValueForBitrangeStartAndConstantExpressionForEndWithBitrangeStartMissmatching) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 53));
    performTestExecution("module main(in b[1](5)) for $i = 0 to 4 step 1 do if b[0].0:($i + 1) then skip else skip fi b[0].1:($i + 1) rof");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDueToDifferentBitrangeBeingAccessedUsingConstantValueForBitrangeStartAndConstantExpressionForEndWithBitrangeEndMissmatching) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 53));
    performTestExecution("module main(in b[1](5)) for $i = 0 to 4 step 1 do if b[0].0:($i + 1) then skip else skip fi b[0].0:($i + 2) rof");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDueToDifferentBitrangeBeingAccessedUsingConstantExpressionForBitrangeStartAndConstantValueForEndWithBitrangeStartMissmatching) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 53));
    performTestExecution("module main(in b[1](5)) for $i = 0 to 4 step 1 do if b[0].($i + 1):4 then skip else skip fi b[0].($i + 2):4 rof");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDueToDifferentBitrangeBeingAccessedUsingConstantExpressionForBitrangeStartAndConstantValueForEndWithBitrangeEndMissmatching) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 53));
    performTestExecution("module main(in b[1](5)) for $i = 0 to 4 step 1 do if b[0].($i + 1):3 then skip else skip fi b[0].($i + 1):4 rof");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDueToDifferentBitrangeBeingAccessedUsingConstantExpressionForBitrangeStartAndConstantExpressionForEndWithBitrangeStartMissmatch) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 53));
    performTestExecution("module main(in b[1](5)) for $i = 0 to 4 step 1 do if b[0].($i + 1):(#b - 3) then skip else skip fi b[0].($i + 2):(#b - 3) rof");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDueToDifferentBitrangeBeingAccessedUsingConstantExpressionForBitrangeStartAndConstantExpressionForEndWithBitrangeEndMissmatch) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 53));
    performTestExecution("module main(in b[1](5)) for $i = 0 to 4 step 1 do if b[0].($i + 1):(#b - 3) then skip else skip fi b[0].($i + 2):(#b - 1) rof");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchInOperandsOfBinaryExpression) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 39));
    performTestExecution("module main(in a[2](4), in b[1](4)) if (a[0].1 > 2) then skip else skip fi (b[0].1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDueToSwappedOperandsOfBinaryExpression) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 27));
    performTestExecution("module main(in a[2](4)) if (a[0].1 > 2) then skip else skip fi (2 > a[0].1)");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchInOperationOfBinaryExpression) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 27));
    performTestExecution("module main(in a[2](4)) if (a[0].1 > a[1].1) then skip else skip fi (a[0].1 < a[1].1)");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchInOperandsOfShiftExpression) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 39));
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 40), 1, 4);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 75), 1, 4);
    performTestExecution("module main(in a[2](4), in b[1](4)) if (a[0] >> 2) then skip else skip fi (b[0] >> 2)");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchInOperationsOfShiftExpression) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 27));
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 28), 1, 4);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 63), 1, 4);
    performTestExecution("module main(in a[2](4)) if (a[0] >> 2) then skip else skip fi (a[0] << 2)");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchInOperandsOfNumericExpression) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 43));
    performTestExecution("module main() for $i = 0 to 3 step 1 do if ($i + 2) then skip else skip fi (2 + $i) rof");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchInOperationsOfNumericExpression) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 43));
    performTestExecution("module main() for $i = 0 to 3 step 1 do if ($i + 2) then skip else skip fi (1 - ($i + $i)) rof");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchInGuardConditionBeingSimplifiedCorrectlyDetected) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 30));
    performTestExecution("module main(inout a[2](4)) if ((a[0].1 + 2) * 0) then ++= a[0] else --= a[0] fi (a[1].1 + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchInClosingGuardConditionBeingSimplifiedCorrectlyDetected) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 30));
    performTestExecution("module main(inout a[2](4)) if (a[0].1 + 2) then ++= a[0] else --= a[0] fi ((a[1].1 + 2) * 0)");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDueToDifferentExpressionTypesBeingUsed) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 30));
    performTestExecution("module main(inout a[2](4)) if (a[0].1 + 2) then ++= a[0] else --= a[0] fi (a[1].0 << 2)");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDueToImplicitAndExplicitDimensionAccessOn1DVariable) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 30));
    performTestExecution("module main(inout a[1](4)) if (a.0 + 2) then ++= a[0] else --= a[0] fi (a[0].0 << 2)");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDueToExplicitAndImplicitDimensionAccessOn1DVariable) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 30));
    performTestExecution("module main(inout a[1](4)) if (a[0].0 + 2) then ++= a[0] else --= a[0] fi (a.0 << 2)");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDetectedEvenWhenExpressionsWhereSimplified) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 55));
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 2 do if (((($i + b.1) - a.0:$i) * (#b - 2)) << 3) then ++= a else skip fi (((($i + b.1) - a.1:$i) * (#b - 2)) << 3) rof");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDetectedEvenWhenExpressionsWhereSimplifiedByOneConstantOperandBeingOptimizedAway) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 55));
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 2 do if (((($i + b.1) - a.0:$i) * (#b - 1)) << 3) then ++= a else skip fi (((($i + b.1) - a.1:$i) * (#b - 1)) << 3) rof");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDetectedEvenWhenExpressionsInDimensionAccessOfVariableAccessWereSimplified) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 65));
    performTestExecution("module main(inout a[2](4), in b(2)) for $i = 0 to (#a - 1) do if a[((#b - 2) * (a[0].0:$i + b.1))].1 then ++= a[1] else skip fi a[((#b - 2) * (a[0].1:$i + b.1))].1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDetectedEvenWhenExpressionsInDimensionAccessOfVariableAccessWereSimplifiedByOneConstantOperandBeingOptimizedAway) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 65));
    performTestExecution("module main(inout a[2](4), in b(2)) for $i = 0 to (#a - 1) do if a[((#b - 1) * (a[0].0:$i + b.1))].1 then ++= a[0] else skip fi a[((#b - 2) * (a[0].1:$i + b.1))].1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDetectedEvenWhenExpressionsInAccessedBitrangeStartOfVariableAccessWereSimplified) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 62));
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 2 step 1 do if a.((#b - 2) * (($i + 1) * 2)) then ++= a else skip fi a.((#b - 2) * ($i * 2)) rof");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDetectedEvenWhenExpressionsInAccessedBitrangeEndOfVariableAccessWereSimplified) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 62));
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 2 step 1 do if a.$i:((#b - 2) * (($i + 1) * 2)) then ++= a else skip fi a.$i:((#b - 2) * ($i * 2)) rof");
}

TEST_F(SyrecParserErrorTestsFixture, IfStatementGuardConditionsMissmatchDetectedEvenWhenExpressionsInAccessedBitrangeStartAndEndOfVariableAccessWereSimplified) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 62));
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 2 step 1 do if a.(#a * (#b - 2)):((#b - 2) * (($i + 1) * 2)) then ++= a else skip fi a.(#b * (#b - 2)):((#b - 2) * ($i * 2)) rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInGuardExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 31));
    buildAndRecordExpectedSemanticError<SemanticError::TooFewDimensionsAccessed>(Message::Position(1, 32), 1, 2);
    buildAndRecordExpectedSemanticError<SemanticError::TooManyDimensionsAccessed>(Message::Position(1, 68), 3, 2);
    performTestExecution("module main(out a[2][1](4)) if (a[0].1 > 2) then skip else skip fi (a[0][0][1].1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableWithNotCompletelySpecifiedDimensionAccessInGuardExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::TooFewDimensionsAccessed>(Message::Position(1, 32), 1, 2);
    buildAndRecordExpectedSemanticError<SemanticError::TooFewDimensionsAccessed>(Message::Position(1, 68), 1, 2);
    performTestExecution("module main(out a[2][3](4)) if (a[1].1 > 2) then skip else skip fi (a[1].1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, EmptyTrueBranchInIfStatementCausesError) {
    recordSyntaxError(Message::Position(1, 40), "extraneous input 'else' expecting {'++=', '--=', '~=', 'call', 'uncall', 'for', 'if', 'skip', IDENT}");
    recordSyntaxError(Message::Position(1, 50), "mismatched input 'fi' expecting 'else'");
    performTestExecution("module main(out a(4)) if (a.1 > 2) then else skip fi (a.1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, EmptyFalseBranchInIfStatementCausesError) {
    recordSyntaxError(Message::Position(1, 50), "mismatched input 'fi' expecting {'++=', '--=', '~=', 'call', 'uncall', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main(out a(4)) if (a.1 > 2) then skip else fi (a.1 > 2)");
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
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 94), "c");
    performTestExecution("module main(in a(4)) wire b(4) if ((2 > 1) || (c > a.1)) then ++= b else skip fi ((2 > 1) || (c > a.1))");
}

TEST_F(SyrecParserErrorTestsFixture, SemanticErrorInClosingGuardConditionReportedWhenGuardConditionWasWithoutError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 65), "b");
    performTestExecution("module main(out a[2](4)) if (a[0].1 > 2) then skip else skip fi (b[0].1 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, SemanticErrorInStatementOfSkipableTrueBranchCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::AssignmentToReadonlyVariable>(Message::Position(1, 54), "b");
    performTestExecution("module main(inout a(4), in b(2)) if (#a < 2) then ++= b else ++= a fi (#a < 2)");
}

TEST_F(SyrecParserErrorTestsFixture, SemanticErrorInStatementOfSkipableFalseBranchCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::AssignmentToReadonlyVariable>(Message::Position(1, 65), "b");
    performTestExecution("module main(inout a(4), in b(2)) if (#a > 2) then ++= a else ++= b fi (#a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionSetInExpressionNotResetByUnknownBitwidthOfFutureOperandAndCausesErrorOnOperandBitwidthMissmatch) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 79), 1, 2);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 127), 1, 2);
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 3 step 1 do if (a.0 > (b.0:$i + a.1:2)) then skip else skip fi (a.0 > (b.0:$i + a.1:2)) rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionSetInExpressionNotResetByConstantExpressionAndCausesErrorOnOperandBitwidthMissmatch) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 72), 1, 2);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 122), 1, 2);
    performTestExecution("module main(inout a(4)) for $i = 0 to 3 step 1 do if (a.0 > (($i + 2) + a.1:2)) then skip else skip fi (a.0 > (($i + 2) + a.1:2)) rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionSetInExpressionDespiteIndexForAccessedValueOfDimensionHavingUnknownValueAndCausesErrorOnOperandBitwidthMissmatch) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 72), 1, 2);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 122), 1, 2);
    performTestExecution("module main(inout a[4](4)) for $i = 0 to 3 step 1 do if (a[$i].0 > (2 + a[0].1:2)) then skip else skip fi (a[$i].0 > (2 + a[0].1:2)) rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionSetInShiftExpressionOnlyResetAfterFullEnclosingExpressionWasProcessedAndCausesErrorOnOperandBitwidthMissmatch) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 57), 1, 2);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 109), 1, 2);
    performTestExecution("module main(inout a(4), in b(2)) if ((a.0 << 1) > (b.0 + a.1:2)) then skip else skip fi ((a.0 << 1) > (b.0 + a.1:2))");
}

TEST_F(SyrecParserErrorTestsFixture, VariableAccessWithUnknownAccessedBitwidthDoNotBlockFutureOperandBitwidthRestrictions) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 87), 1, 2);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 143), 1, 2);
    performTestExecution("module main(inout a(4), in b(2)) for $i = 0 to 3 step 1 do if ((a.0:$i - b.0) + (b.1 + a.1:2)) then skip else skip fi ((a.0:$i - b.0) + (b.1 + a.1:2)) rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionForValueOfDimensionOnlySetUntilExpressionIsParsedAndCausesErrorOnOperandBitwidthMissmatch) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 40), 1, 4);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 59), 1, 2);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 102), 1, 4);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 121), 1, 2);
    performTestExecution("module main(inout a[4](4), in b(4)) if (b < a[((b.0 + 2) - b.1:0)].0) then ++= a[0] else --= a[0] fi (b < a[((b.0 + 2) - b.1:0)].0)");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionForValueOfDimensionOnlySetUntilExpressionIsParsedAndResetForNextDimensionAndCausesErrorOnOperandBitwidthMissmatch) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 93), 1, 2);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 173), 1, 2);
    performTestExecution("module main(inout a[4][2](4), in b(4)) for $i = 0 to 1 do if (b.1 < a[(b + $i)][((b.0 + 2) - b.1:0)].0) then ++= a[0][1] else --= a[0][1] fi (b.1 < a[(b + $i)][((b.0 + 2) - b.1:0)].0) rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionSetInExpressionForUnknownAccessedBitDefinedWithLoopVariable) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 66), 1, 4);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 105), 1, 4);
    performTestExecution("module main(inout a[1](4), in b(4)) for $i = 0 to 1 do if (a.$i + b) then ++= a[0] else --= a fi (a.$i + b) rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionOfBinaryExpressionIsPropagatedToShiftExpr) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 90), 1, 2);
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 149), 1, 2);
    performTestExecution("module main(inout a(4), in b(2), in c[3](4)) for $i = 0 to 3 do if c[(((b.1 + 2) << $i) + a[0].0:1)].0 then skip else skip fi c[(((b.1 + 2) << $i) + a[0].0:1)].0 rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionOfBinaryExpressionIsPropagatedToBinaryExpr) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 92), 2, 1);
    performTestExecution("module main(inout a(4), in b(2), in c(2)) for $i = 0 to 3 step 2 do ++= a[(((b + 2) / $i) + c[0].0)] rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionOfShiftExpressionIsPropagatedToBinaryExpr) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 86), 2, 1);
    performTestExecution("module main(inout a(4), in b(2), in c(2)) for $i = 0 to 3 do ++= a[(((b << 2) / $i) + c[0].0)] rof");
}

TEST_F(SyrecParserErrorTestsFixture, OperandBitwidthRestrictionOfShiftExpressionIsPropagatedToShiftExpr) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 87), 2, 1);
    performTestExecution("module main(inout a(4), in b(2), in c(2)) for $i = 0 to 3 do ++= a[(((b << 2) / $i) + (c[0].0 >> 2))] rof");
}
