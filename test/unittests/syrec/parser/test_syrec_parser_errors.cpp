#include "core/syrec/program.hpp"
#include "core/syrec/parser/utils/custom_error_mesages.hpp"
#include "core/syrec/parser/utils/parser_messages_container.hpp"
#include "core/utils/base_syrec_ir_entity_stringifier.hpp"
#include "fmt/compile.h"

#include "gmock/gmock-matchers.h"
#include <gtest/gtest.h>

using namespace syrecParser;

class SyrecParserErrorTestsFixture: public testing::Test {
public:
    using MessagesContainer = std::vector<Message>;

    template<SemanticError semanticError, typename... T>
    void buildAndRecordExpectedSemanticError(Message::Position messagePosition, T&&... args) {
        static_assert(!getFormatForSemanticErrorMessage<semanticError>().empty());
        static_assert(!getIdentifierForSemanticError<semanticError>().empty());

        expectedErrorMessages.emplace_back(Message(
            Message::Type::Error,
            std::string(getIdentifierForSemanticError<semanticError>()),
            messagePosition, 
            fmt::format(FMT_STRING(getFormatForSemanticErrorMessage<semanticError>()), std::forward<T>(args)...)));
    }

    void performTestExecution(const std::string& stringifiedSyrecProgramToProcess) const {
        ASSERT_FALSE(expectedErrorMessages.empty());
        syrec::Program program;

        std::string aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram;
        ASSERT_NO_FATAL_FAILURE(aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram = program.read(stringifiedSyrecProgramToProcess));
        ASSERT_NO_FATAL_FAILURE(assertExpectedErrorsAreDetectedDuringProcessingOfSyrecProgram(aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram, expectedErrorMessages));
    }

protected:
    MessagesContainer expectedErrorMessages;

    static void assertStringificationOfParsedSyrecProgramIsSuccessful(const syrec::Program& syrecProgramToStringifiy, std::ostream& containerForStringifiedProgram) {
        // TODO: Troubleshooting as to why the stringification of the SyReC program failed is currently not possible but should only happen if either the IR representation of
        // the IR representation or of an internal error in the stringifier. Can we handle the former cases better?
        utils::BaseSyrecIrEntityStringifier syrecProgramStringifier(std::nullopt);
        bool                                wasStringificationSuccessful;
        ASSERT_NO_FATAL_FAILURE(wasStringificationSuccessful = syrecProgramStringifier.stringify(containerForStringifiedProgram, syrecProgramToStringifiy)) << "Error during stringification of SyReC program";
        ASSERT_TRUE(wasStringificationSuccessful) << "Failed to stringify SyReC program";
    }

    static void assertExpectedErrorsAreDetectedDuringProcessingOfSyrecProgram(const std::string_view& aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram, const MessagesContainer& expectedErrorsDetectedDuringProcessingOfSyrecProgram) {
        std::vector<std::string_view> errorsDetectedDuringProcessingOfSyrecProgram;
        // In the best case scenario, no further resizing of the container is necessary (i.e. the number of actually found errors is equal to the number of expected ones).
        errorsDetectedDuringProcessingOfSyrecProgram.reserve(expectedErrorsDetectedDuringProcessingOfSyrecProgram.size());

        std::size_t lastFoundPositionOfNewlineDelimiter = 0;
        std::size_t currNewLineDelimiterPosition        = findNextNewlineDelimiterInString(aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram, lastFoundPositionOfNewlineDelimiter);
        while (currNewLineDelimiterPosition != std::string::npos) {
            const std::size_t lengthOfErrorMessage   = (currNewLineDelimiterPosition - lastFoundPositionOfNewlineDelimiter) + 1;
            const auto        actualCurrErrorMessage = aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram.substr(lastFoundPositionOfNewlineDelimiter, lengthOfErrorMessage);
            errorsDetectedDuringProcessingOfSyrecProgram.emplace_back(actualCurrErrorMessage);

            // On Windows system we assume that the newline is encoded as the '\r\n' character sequence while on all other system it should be equal to the '\n' character
            lastFoundPositionOfNewlineDelimiter = currNewLineDelimiterPosition + 1;

            #if _WIN32
                ++lastFoundPositionOfNewlineDelimiter;
            #endif

            currNewLineDelimiterPosition = findNextNewlineDelimiterInString(aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram, lastFoundPositionOfNewlineDelimiter);
        }
        ASSERT_NO_FATAL_FAILURE(assertExpectedAndActualErrorsMatch(expectedErrorsDetectedDuringProcessingOfSyrecProgram, errorsDetectedDuringProcessingOfSyrecProgram));
    }

    static void assertExpectedAndActualErrorsMatch(const MessagesContainer& expectedErrors, const std::vector<std::string_view>& actualErrorsInUnifiedFormat) {
        // TODO: Find better solution ot print errors
        ASSERT_EQ(expectedErrors.size(), actualErrorsInUnifiedFormat.size()) << "Expected " << expectedErrors.size() << " errors but only " << actualErrorsInUnifiedFormat.size() << " were found";
        for (size_t errorIdx = 0; errorIdx < expectedErrors.size(); ++errorIdx) {
            ASSERT_EQ(expectedErrors.at(errorIdx).stringify(), actualErrorsInUnifiedFormat.at(errorIdx)) << "Expected error: " << expectedErrors.at(errorIdx).stringify() << "| Actual Error: " << actualErrorsInUnifiedFormat.at(errorIdx);
        }
    }

private:
    [[nodiscard]] static std::size_t findNextNewlineDelimiterInString(const std::string_view& stringToSearchThrough, std::size_t searchStartPosition) {
        #if _WIN32
            return stringToSearchThrough.find_first_of("\r\n", searchStartPosition);
        #else
            return stringToSearchThrough.find_first_of('\n', searchStartPosition);
        #endif
    }
};

// TODO: Should we tests for non-integer numbers used in any signal declaration

// Tests for production module
TEST_F(SyrecParserErrorTestsFixture, OmittingModuleKeywordCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidModuleKeywordUsageCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidSymbolInModuleIdentifierCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, EmptyModuleIdentifierCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingModuleParameterListOpeningBracketCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidModuleParameterListOpeningBracketCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingModuleParameterListClosingBracketCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidModuleParameterListClosingBracketCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingModuleParameterDelimiterCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableDeclarationAfterModuleParameterDelimiterCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, EmptyModuleBodyCausesError) {
    GTEST_SKIP();
}

// TODO: According to the specification, an overload of the top level module not named 'main' is possible
// TODO: The specification also does not disallow the definition of an overload of the module named 'main' but specifies that the user can define a top-level module with the special identifier 'main'
// see section 2.1
TEST_F(SyrecParserErrorTestsFixture, OverloadOfModuleNamedMainCausesError) {
    GTEST_SKIP();
}

// Tests for production parameter
TEST_F(SyrecParserErrorTestsFixture, OmittingVariableTypeCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidVariableTypeCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableIdentifierInModuleParameterDeclarationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateVariableIdentiferSharingSameVariableTypeCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidSymbolInModuleParameterIdentifierCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateVariableIdentiferUsingDifferentVariableTypeCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableValuesForDimensionOpeningBracketCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidVariableValuesForDimensionOpeningBracketCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableValuesForDimensionClosingBracketCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidVariableValuesForDimensionClosingBracketCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, NoneNumericValueForNumberOfValuesForDimensionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingValueForNumberOfValuesForDimensionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketForModuleParameterDeclarationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketForModuleParameterDeclarationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketForModuleParameterDeclarationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketForModuleParameterDeclarationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingModuleParameterBitwidthCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, NoneNumericModuleParameterBitwidthCausesError) {
    GTEST_SKIP();
}

// Tests for signal-list
TEST_F(SyrecParserErrorTestsFixture, OmittingLocalModuleParameterTypeInDeclarationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidLocalModuleParameterTypeInDeclarationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingLocalVariableDelimiterInDeclarationsNotSharingSameVariableType) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingLocalVariableDelimiterInDeclarationsSharingSameVariableType) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableIdentifierInLocalModuleParameterDeclarationCausesError) {
    GTEST_SKIP();
}

// TODO: Test in declaration sharing variable type and single variable type declaration
TEST_F(SyrecParserErrorTestsFixture, DuplicateVariableIdentiferSharingSameVariableTypeInLocalModuleParameterDeclarationCausesError) {
    GTEST_SKIP();
}

// TODO: Test in declaration sharing variable type and single variable type declaration
TEST_F(SyrecParserErrorTestsFixture, DuplicateVariableIdentiferUsingDifferentVariableTypeInLocalModuleParameterDeclarationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateVariableIdentiferBetweenModuleParameterAndLocalVariable) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidSymbolInLocalModuleVariableIdentifierCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableValuesForDimensionOpeningBracketInLocalModuleParameterDeclarationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidVariableValuesForDimensionOpeningBracketInLocalModuleParameterDeclarationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableValuesForDimensionClosingBracketInLocalModuleParameterDeclarationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidVariableValuesForDimensionClosingBracketInLocalModuleParameterDeclarationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, NoneNumericValueForNumberOfValuesForDimensionInLocalModuleParameterDeclarationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingValueForNumberOfValuesForDimensionInLocalModuleVariableDeclarationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketInLocalModuleVariableDeclarationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketInLocalModuleVariableDeclarationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketInLocalModuleVariableDeclarationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketInModuleVariableDeclarationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingLocalModuleVariableBitwidthCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, NoneNumericLocalModuleVariableBitwidthCausesError) {
    GTEST_SKIP();
}

// Tests for production call-statement
// TODO: All call statements should be repeated with the uncall keyword
TEST_F(SyrecParserErrorTestsFixture, OmittingCallKeywordCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidCallKeywordCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingModuleIdentiferInCallStatementCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, ModuleIdentifierNotMatchingAnyDeclaredModuleIdentifierInCallStatementCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingCallStatementParameterOpeningBracket) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidCallStatementParameterOpeningBracket) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingCallStatementParameterIdentifierCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, CallStatementParameterIdentifierNotMatchingAnyDeclaredVariableCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingCallStatementParameterDelimiterCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingUncallKeywordCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidUncallKeywordCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingModuleIdentiferInUncallStatementCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, ModuleIdentifierNotMatchingAnyDeclaredModuleIdentifierInUncallStatementCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingUncallStatementParameterOpeningBracket) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidUncallStatementParameterOpeningBracket) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingUncallStatementParameterIdentifierCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UncallStatementParameterIdentifierNotMatchingAnyDeclaredVariableCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingUncallStatementParameterDelimiterCausesError) {
    GTEST_SKIP();
}

// TODO: Overload tests

TEST_F(SyrecParserErrorTestsFixture, OmittingCallStatementParameterClosingBracket) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidCallStatementParameterClosingBracket) {
    GTEST_SKIP();
}

// Tests for production for-statement
TEST_F(SyrecParserErrorTestsFixture, OmittingForStatementKeywordCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidForStatementKeywordCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingLoopVariableIdentPrefixCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingLoopVariableInitialValueInitializationEqualSignCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericLoopVariableInitialValueInitializationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableInInitialValueInitializationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingToKeywordInLoopVariableDefinitionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingToKeywordInForLoopIterationNumbersCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericLoopIterationNmberStartValueCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericLoopIterationNumberEndValueCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericLoopVariableEndValueCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidForLoopStepSizeKeywordCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericLoopVariableStepsizeValueCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsingNonMinusSymbolAfterStepKeywordCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsingMultipleMinusSymbolsAfterStepkeywordCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingDoKeywordAfterLoopHeaderDefinitionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidDoKeywordAfterLoopHeaderDefinitionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, EmptyLoopBodyCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateDefinitionOfLoopVariableInNestedLoopCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNonNumericExpressionInLoopVariableInitializationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNonNumericExpressionInLoopVariableEndValueDefinitionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNonNumericExpressionInLoopVariableStepsizeDefinitionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNonNumericExpressionInForLoopIterationNumberStartValueDefinitionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNonNumericExpressionInForLoopIterationNumberEndValueDefinitionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNonNumericExpressionInForLoopIterationNumberStepsizeDefinitionCausesError) {
    GTEST_SKIP();
}

// Tests for production if-statement
TEST_F(SyrecParserErrorTestsFixture, InvalidIfKeywordCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketOfIfStatementInNonConstantValueGuardExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketOfIfStatementInNonConstantValueGuardExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketOfIfStatementInNonConstantValueGuardExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketOfIfStatementInNonConstantValueGuardExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingThenKeywordAfterGuardConditionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidThenKeywordAfterGuardConditionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingElseKeywordAfterGuardConditionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidElseKeywordAfterGuardConditionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingFiKeywordAfterGuardConditionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidFiKeywordAfterGuardConditionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketOfIfStatementInNonConstantValueClosingGuardExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketOfIfStatementInNonConstantValueClosingGuardExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketOfIfStatementInNonConstantValueClosingGuardExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketOfIfStatementInNonConstantValueClosingGuardExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, MissingmatchBetweenGuardAndClosingGuardConditionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInGuardExpressionCausesError) {
    GTEST_SKIP();
}

// Tests for production unary-statement
TEST_F(SyrecParserErrorTestsFixture, UsageOfReadonlyVariableInUnaryStatementCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableInUnaryStatementCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUnknownUnaryAssignmentOperationInUnaryStatementCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfExpressionInUnaryStatementCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableInUnaryAssignmentCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInEvaluatedVariableAccessInUnaryAssignmentCausesError) {
    GTEST_SKIP();
}

// Tests for production assign-statement
TEST_F(SyrecParserErrorTestsFixture, UsageOfReadonlyVariableOnLhsOfAssignStatementCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableOnLhsOfAssignStatementCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableOnRhsOfAssignStatementCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUnknownAssignOperationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingEqualSignFromAssignOperationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableAsLhsOperandOfAssignmentCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInEvaluatedVariableAccessInLhsOperandOfAssignmentCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInBitwidthsOfOperandsOfAssignmentCausesError) {
    GTEST_SKIP();
}

// TODO: Tests for overlaping signals

// Tests for production swap-statement
TEST_F(SyrecParserErrorTestsFixture, UsageOfReadonlyVariableOnLhsOfSwapStatementCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableOnLhsOfSwapStatementCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfReadonlyVariableOnRhsOfSwapStatementCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableOnRhsOfSwapStatementCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfExpressionOnLhsOfSwapStatementCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfExpressionOnRhsOfSwapStatementCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, Non1DSignalInEvaluatedVaribleAccessOfLhsSwapOperandCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, Non1DSignalInEvaluatedVaribleAccessOfRhsSwapOperandCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfConstantOperandAsLhsSwapOperandCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfConstantOperandAsRhsSwapOperandCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidSwapOperationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableAsLhsOperandOfSwapOperationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableAsRhsOperandOfSwapOperationCausesError) {
    GTEST_SKIP();
}

// TODO: Test for overlapping signal accesses

// Tests for production binary-expression
// TODO: Tests for truncation of values larger than the expected bitwidth
TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableInBinaryExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUnknownBinaryOperationInBinaryExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketOfBinaryExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketOfBinaryExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketOfBinaryExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketOfBinaryExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInEvaluatedVariableAccessInLhsOperandOfBinaryExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInEvaluatedVariableAccessInRhsOperandOfBinaryExpressionCausesError) {
    GTEST_SKIP();
}

// TODO: Tests for nested expressions

// Tests for production unary-expression
// TODO: Add tests when IR supports unary expressions

// Tests for production shift-expression
// TODO: Tests for truncation of values larger than the expected bitwidth
TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableInLhsOperandOfShiftExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableInRhsOperandOfShiftExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidShiftOperationCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingLhsOperandOfShiftExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingRhsOperandOfShiftExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericExpressionInRhsOperandOfShiftExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableUsedAsLhsOperandOfShiftOperationCausesError) {
    GTEST_SKIP();
}

// Tests for production number
TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableInNumericExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidOperationInNumericExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBrackerOfNumericExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidOpeningBracketInNumericExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBrackerOfNumericExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidClosingBracketInNumericExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInNumericExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInEvaluatedNumericExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredLoopVariableInNumericExpressionCausesError) {
    GTEST_SKIP();
}

// Tests for production signal
// TODO: Tests for indices out of range and division by zero errors in dynamic expressions (i.e. loop variables evaluated at compile time)
// TODO: Tests for truncation of values larger than the expected bitwidth
TEST_F(SyrecParserErrorTestsFixture, OmittingVariableIdentifierInVariableAccessCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketForAccessOnDimensionOfVariableCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketForAccessOnDimensionOfVariableCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, IndexForAccessedValueOfDimensionOutOfRangeCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, AccessedDimensionOfVariableOutOfRangeCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, None1DSizeOfVariableAccessCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, None1DAccessOnExpressionForValueOfDimensionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketForAccessOnDimensionOfVariableCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketForAccessOnDimensionOfVariableCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingBitrangeStartSymbolCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidBitrangeStartSymbolCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, OmittingBitrangEndSymbolCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, InvalidBitrangEndSymbolCausesError) {
    GTEST_SKIP();
}

// TODO: Division by zero error are tested in tests for production 'number', should we explicitly tests the same behaviour for every usage of the production in other productions?
TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInDynamicBitrangeStartValueExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInDynamicBitrangeEndValueExpressionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, BitrangeStartValueOutOfRangeCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, BitrangeEndValueOutOfRangeCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInDynamicExpressionForValueOfDimensionCausesError) {
    GTEST_SKIP();
}

TEST_F(SyrecParserErrorTestsFixture, TestError) {
    ASSERT_NO_FATAL_FAILURE(buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(0, 0), "test"));
    ASSERT_NO_FATAL_FAILURE(performTestExecution("bla"));
}