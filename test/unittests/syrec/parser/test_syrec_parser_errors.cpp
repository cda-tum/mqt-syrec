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
        syrec::Program program;

        std::string aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram;
        // We needed to modifiy the syrec::Program interface to allow processing of programs from a string due to the missing cross platform support
        // to create temporary or in-memory files (see mkstemp and fmemopen functions which are POSIX specific ones) without using the boost library.
        // Using the tmpfile/tmpfile_s of the C++ standard library is also not viable for the creating of temporary files due to the missing ability
        // to determine the path/filename of the generated file descriptor on all platforms.
        ASSERT_NO_FATAL_FAILURE(aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram = program.readFromString(stringifiedSyrecProgramToProcess));
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
// TODO: Check whether swaps or assignments (both unary and binary ones) between N-d signals are possible

// Tests for production module
TEST_F(SyrecParserErrorTestsFixture, OmittingModuleKeywordCausesError) {
    performTestExecution("main() skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidModuleKeywordUsageCausesError) {
    performTestExecution("modul main() skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidSymbolInModuleIdentifierCausesError) {
    performTestExecution("mod-ule main() skip");
}

TEST_F(SyrecParserErrorTestsFixture, EmptyModuleIdentifierCausesError) {
    performTestExecution("module () skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingModuleParameterListOpeningBracketCausesError) {
    performTestExecution("module main) skip");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidModuleParameterListOpeningBracketCausesError) {
    performTestExecution("module main[) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingModuleParameterListClosingBracketCausesError) {
    performTestExecution("module main( skip");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidModuleParameterListClosingBracketCausesError) {
    performTestExecution("module main(] skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingModuleParameterDelimiterCausesError) {
    performTestExecution("module main(in a[2](16) out b(16)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableDeclarationAfterModuleParameterDelimiterCausesError) {
    performTestExecution("module main(in a[2](16), ) skip");
}

TEST_F(SyrecParserErrorTestsFixture, EmptyModuleBodyCausesError) {
    performTestExecution("module main(in a[2](16))");
}

// TODO: According to the specification, an overload of the top level module not named 'main' is possible
// TODO: The specification also does not disallow the definition of an overload of the module named 'main' but specifies that the user can define a top-level module with the special identifier 'main'
// see section 2.1
TEST_F(SyrecParserErrorTestsFixture, OverloadOfModuleNamedMainCausesError) {
    performTestExecution("module main(in a[2](16)) skip module main(out b[1](16)) skip");
}

// TODO: Tests for overload of main module (not using identifier 'main')

// Tests for production parameter
TEST_F(SyrecParserErrorTestsFixture, OmittingVariableTypeCausesError) {
    performTestExecution("module main(a[2](16)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidVariableTypeCausesError) {
    performTestExecution("module main(int a(16)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableIdentifierInModuleParameterDeclarationCausesError) {
    performTestExecution("module main(in (16))");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateVariableIdentifierSharingSameVariableTypeCausesError) {
    performTestExecution("module main(in a(16), in a(8))");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateVariableIdentifierUsingDifferentVariableTypeCausesError) {
    performTestExecution("module main(in a(16), out a(16)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidSymbolInModuleParameterIdentifierCausesError) {
    performTestExecution("module main(in a-t(16)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableValuesForDimensionOpeningBracketCausesError) {
    performTestExecution("module main(in a16](16)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidVariableValuesForDimensionOpeningBracketCausesError) {
    performTestExecution("module main(in a{16](2)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableValuesForDimensionClosingBracketCausesError) {
    performTestExecution("module main(in a[2) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidVariableValuesForDimensionClosingBracketCausesError) {
    performTestExecution("module main(in a[2}) skip");
}

TEST_F(SyrecParserErrorTestsFixture, NoneNumericValueForNumberOfValuesForDimensionCausesError) {
    performTestExecution("module main(in a[test](2)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingValueForNumberOfValuesForDimensionCausesError) {
    performTestExecution("module main(in a[]) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketForModuleParameterBitwidthDeclarationCausesError) {
    performTestExecution("module main(in a16)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketForModuleParameterBitwidthDeclarationCausesError) {
    performTestExecution("module main(in a{16)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketForModuleParameterBitwidthDeclarationCausesError) {
    performTestExecution("module main(in a(16) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketForModuleParameterBitwidthDeclarationCausesError) {
    performTestExecution("module main(in a(16}) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingModuleParameterBitwidthWithBracketsDefinedCausesError) {
    performTestExecution("module main(in a()) skip");
}

TEST_F(SyrecParserErrorTestsFixture, NoneNumericModuleParameterBitwidthCausesError) {
    performTestExecution("module main(in a(2 -3)) skip");
}

// Tests for signal-list
TEST_F(SyrecParserErrorTestsFixture, OmittingLocalModuleParameterTypeInDeclarationCausesError) {
    performTestExecution("module main() a(16) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidLocalModuleParameterTypeInDeclarationCausesError) {
    performTestExecution("module main() int a(16) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingLocalVariableDelimiterInDeclarationsNotSharingSameVariableType) {
    performTestExecution("module main() wire a(16) state b(16) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingLocalVariableDelimiterInDeclarationsSharingSameVariableType) {
    performTestExecution("module main() wire a(16) b(16) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableIdentifierInLocalModuleParameterDeclarationCausesError) {
    performTestExecution("module main() wire (16) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableIdentifierInLocalModuleParameterDeclarationSharingSameVariableTypeCausesError) {
    performTestExecution("module main() wire a(16), [2](4) skip");
}

// TODO: Test in declaration sharing variable type and single variable type declaration
TEST_F(SyrecParserErrorTestsFixture, DuplicateVariableIdentiferSharingSameVariableTypeInLocalModuleParameterDeclarationCausesError) {
    performTestExecution("module main() wire a(16), b(8), a(4) skip");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateVariableIdentiferSharingSameVariableTypeInSeparateDeclarationInLocalModuleParameterDeclarationCausesError) {
    performTestExecution("module main() wire a(16), b(8) wire a(4) skip");
}

// TODO: Test in declaration sharing variable type and single variable type declaration
TEST_F(SyrecParserErrorTestsFixture, DuplicateVariableIdentiferUsingDifferentVariableTypeInLocalModuleParameterDeclarationCausesError) {
    performTestExecution("module main() wire a(16) state b(8), a(4) skip");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateVariableIdentiferBetweenModuleParameterAndLocalVariableInSeparateLocalVariableDeclaration) {
    performTestExecution("module main(in a(16)) wire b(8) state a(4) skip");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateVariableIdentiferBetweenModuleParameterAndLocalVariableDeclarationWithLatterDefiningMultipleVariablesOfSameType) {
    performTestExecution("module main(in a(16)) wire c(4) state b(8), a(4) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidSymbolInLocalModuleVariableIdentifierDefinedInSeparateDeclarationCausesError) {
    performTestExecution("module main() wire a-2(16) state b(4) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidSymbolInLocalModuleVariableIdentifierDefinedInDeclarationSharingVariableTypesCausesError) {
    performTestExecution("module main() wire a(16), b#2 skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableValuesForDimensionOpeningBracketInLocalModuleParameterDeclarationCausesError) {
    performTestExecution("module main() wire a2](4) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidVariableValuesForDimensionOpeningBracketInLocalModuleParameterDeclarationCausesError) {
    performTestExecution("module main() wire a{2] skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableValuesForDimensionClosingBracketInLocalModuleParameterDeclarationCausesError) {
    performTestExecution("module main() wire a[2 skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidVariableValuesForDimensionClosingBracketInLocalModuleParameterDeclarationCausesError) {
    performTestExecution("module main() wire a[2) skip");
}

TEST_F(SyrecParserErrorTestsFixture, NoneNumericValueForNumberOfValuesForDimensionInLocalModuleParameterDeclarationCausesError) {
    performTestExecution("module main() wire b(4) wire a[#b) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingValueForNumberOfValuesForDimensionInLocalModuleVariableDeclarationCausesError) {
    performTestExecution("module main() wire a[] skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketOfBitwidthInLocalModuleVariableDeclarationCausesError) {
    performTestExecution("module main() wire a16) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketOfBitwidthInLocalModuleVariableDeclarationCausesError) {
    performTestExecution("module main() wire a16} skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketOfBitwidthInLocalModuleVariableDeclarationCausesError) {
    performTestExecution("module main() wire a(16 skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketOfBitwidthInModuleVariableDeclarationCausesError) {
    performTestExecution("module main() wire a(16} skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingLocalModuleVariableBitwidthValueWhenBracketsWereDefinedCausesError) {
    performTestExecution("module main() wire a() skip");
}

TEST_F(SyrecParserErrorTestsFixture, NoneNumericLocalModuleVariableBitwidthCausesError) {
    performTestExecution("module main() wire a(test) skip");
}

// Tests for production call-statement
// TODO: All call statements should be repeated with the uncall keyword
TEST_F(SyrecParserErrorTestsFixture, OmittingCallKeywordCausesError) {
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) add(a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidCallKeywordCausesError) {
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) performCall add(a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingModuleIdentiferInCallStatementCausesError) {
    performTestExecution("module main(in a(4), in b(4), out c(4)) call (a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleIdentifierNotMatchingAnyDeclaredModuleIdentifierInCallStatementCausesError) {
    performTestExecution("module main(in a(4), in b(4), out c(4)) call add(a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingCallStatementParameterOpeningBracket) {
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call adda, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidCallStatementParameterOpeningBracket) {
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call add[a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingCallStatementParameterIdentifierCausesError) {
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call add(, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, CallStatementParameterIdentifierNotMatchingAnyDeclaredVariableCausesError) {
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call add(d, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingCallStatementParameterDelimiterCausesError) {
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call add(a b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingUncallKeywordCausesError) {
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) add(a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidUncallKeywordCausesError) {
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) performCall add(a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingModuleIdentiferInUncallStatementCausesError) {
    performTestExecution("module main(in a(4), in b(4), out c(4)) uncall (a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleIdentifierNotMatchingAnyDeclaredModuleIdentifierInUncallStatementCausesError) {
    performTestExecution("module main(in a(4), in b(4), out c(4)) uncall add(a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingUncallStatementParameterOpeningBracket) {
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall adda, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidUncallStatementParameterOpeningBracket) {
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall add[a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingUncallStatementParameterIdentifierCausesError) {
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall add(, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, UncallStatementParameterIdentifierNotMatchingAnyDeclaredVariableCausesError) {
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall add(d, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingUncallStatementParameterDelimiterCausesError) {
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall add(a b, c)");
}

// TODO: Overload tests
// TODO: Modifiable parameter overlap tests (i.e. module x(inout a(4), out b(4)) a <=> b ... module main() wire t(4) call x(t, t)
TEST_F(SyrecParserErrorTestsFixture, OmittingCallStatementParameterClosingBracket) {
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call add(a, b, c");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidCallStatementParameterClosingBracket) {
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call add(a, b, c]");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingUncallStatementParameterClosingBracket) {
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall add(a, b, c");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidUncallStatementParameterClosingBracket) {
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall add(a, b, c]");
}

// Tests for production for-statement
TEST_F(SyrecParserErrorTestsFixture, OmittingForStatementKeywordCausesError) {
    performTestExecution("module main(in a(4), out b(4)) $i = 0 to 3 do b.$i ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidForStatementKeywordCausesError) {
    performTestExecution("module main(in a(4), out b(4)) do $i = 0 to 3 do b.$i ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingLoopVariableIdentPrefixCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for i = 0 to 3 do b.$i ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingLoopVariableInitialValueInitializationEqualSignCausesError) {
    performTestExecution("module main(in a(4), out b(4)) do $i 0 to 3 do b.$i ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericLoopVariableInitialValueInitializationCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for $i = b to 3 do b.$i ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableInInitialValueInitializationCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for $i = $i to 3 do b.$i ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingToKeywordInLoopVariableDefinitionCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for $i = 0 3 do b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingToKeywordInForLoopIterationNumbersCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for 0 3 do b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericLoopIterationNumberStartValueInLoopVariableInitializationCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for $i=b to 3 do b.$i ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericLoopIterationNumberStartValueCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for b to 3 do b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericLoopIterationNumberEndValueCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for to b do b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericLoopVariableEndValueCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for $i = 0 to b do b.$i ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidForLoopStepSizeKeywordCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for 3 incr 1 do b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericLoopVariableStepsizeValueCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for 3 step b do b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsingNonMinusSymbolAfterStepKeywordCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for 3 step 1 - do b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsingMultipleMinusSymbolsAfterStepkeywordCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for 3 step --1 do b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingDoKeywordAfterLoopHeaderDefinitionCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for 3 step 1 b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidDoKeywordAfterLoopHeaderDefinitionCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for $i=0 to 3 loop b.0 ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, EmptyLoopBodyCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for $i=0 to 3 do rof");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateDefinitionOfLoopVariableInNestedLoopCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for $i=0 to 3 do for $i=1 to 3 do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNonNumericExpressionInLoopVariableInitializationCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for $i = (b - 2) step 1 do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNonNumericExpressionInLoopVariableEndValueDefinitionCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for $i = 0 to (b - 2) step 1 do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNonNumericExpressionInLoopVariableStepsizeDefinitionCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for 3 step (b - 2) do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNonNumericExpressionInForLoopIterationNumberStartValueDefinitionCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for (b - 2) step 1 do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNonNumericExpressionInForLoopIterationNumberEndValueDefinitionCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for 0 to (b - 2) step 1 do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNonNumericExpressionInForLoopIterationNumberStepsizeDefinitionCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for 0 to 3 step (b - 2) do skip rof");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingRofKeywordAfterLoopBodyCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for 3 step 1 do  ++=b");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidRofKeywordAfterLoopBodyCausesError) {
    performTestExecution("module main(in a(4), out b(4)) for 3 step 1 do  done; ++=b");
}

// Tests for production if-statement
TEST_F(SyrecParserErrorTestsFixture, OmittingIfKeywordCausesError) {
    performTestExecution("module main(out a(4)) (a > 2) then skip else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidIfKeywordCausesError) {
    performTestExecution("module main(out a(4)) if-cond (a > 2) then skip else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketOfIfStatementInNonConstantValueGuardExpressionCausesError) {
    performTestExecution("module main(out a(4)) if a > 2) then skip else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketOfIfStatementInNonConstantValueGuardExpressionCausesError) {
    performTestExecution("module main(out a(4)) if {a > 2) then skip else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketOfIfStatementInNonConstantValueGuardExpressionCausesError) {
    performTestExecution("module main(out a(4)) if (a > 2 then skip else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketOfIfStatementInNonConstantValueGuardExpressionCausesError) {
    performTestExecution("module main(out a(4)) if (a > 2} then skip else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingThenKeywordAfterGuardConditionCausesError) {
    performTestExecution("module main(out a(4)) if (a > 2) skip else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidThenKeywordAfterGuardConditionCausesError) {
    performTestExecution("module main(out a(4)) if (a > 2) do skip else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingElseKeywordAfterGuardConditionCausesError) {
    performTestExecution("module main(out a(4)) if (a > 2) then skip skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidElseKeywordAfterGuardConditionCausesError) {
    performTestExecution("module main(out a(4)) if (a > 2) then skip elif skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingFiKeywordAfterGuardConditionCausesError) {
    performTestExecution("module main(out a(4)) if (a > 2) then skip else skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidFiKeywordAfterGuardConditionCausesError) {
    performTestExecution("module main(out a(4)) if (a > 2) then skip else skip done (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketOfIfStatementInNonConstantValueClosingGuardExpressionCausesError) {
    performTestExecution("module main(out a(4)) if (a > 2) then skip else skip fi a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketOfIfStatementInNonConstantValueClosingGuardExpressionCausesError) {
    performTestExecution("module main(out a(4)) if (a > 2) then skip else skip fi {a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketOfIfStatementInNonConstantValueClosingGuardExpressionCausesError) {
    performTestExecution("module main(out a(4)) if (a > 2) then skip else skip fi (a > 2");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketOfIfStatementInNonConstantValueClosingGuardExpressionCausesError) {
    performTestExecution("module main(out a(4)) if (a > 2) then skip else skip fi (a > 2]");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchBetweenGuardAndClosingGuardConditionBasedOnBitwidthCausesError) {
    performTestExecution("module main(out a(4)) if (a.0 > 2) then skip else skip fi (a.1:2 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchBetweenGuardAndClosingGuardConditionBasedOnAccessedValueOfDimensionCausesError) {
    performTestExecution("module main(out a[2](4)) if (a[0] > 2) then skip else skip fi (a[2] > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchBetweenGuardAndClosingGuardConditionBasedOnNumberOfAccessedDimensionsCausesError) {
    performTestExecution("module main(out a[2][3](4)) if (a[0] > 2) then skip else skip fi (a[1][0] > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchBetweenGuardAndClosingGuardConditionBasedOnOperandOrderCausesError) {
    performTestExecution("module main(out a[2](4)) if (a[0] > 2) then skip else skip fi (2 < a[1])");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchBetweenGuardAndClosingGuardConditionBasedOnTypeOfExpressionCausesError) {
    performTestExecution("module main(out a[2](4)) if (a[0] > 2) then skip else skip fi ((a[0] << 2) > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchBetweenGuardAndClosingGuardConditionBasedOnConstantValuesCausesError) {
    performTestExecution("module main(out a[2](4), out b(2)) if (#a > 2) then skip else skip fi (#b > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchBetweenGuardAndClosingGuardConditionUsingLoopVariablesCausesError) {
    performTestExecution("module main(out a[2](4), out b(2)) for $i = 0 to 2 step 1 do if ($i > 2) then skip else skip fi ($i << 2) rof");
}

// TODO: Should numeric expressions that evaluate to the same value but are defined using a different structure be considered as usable in the guard/closing-guard condition of the if statement?
// TODO: Should omitting of the accessed value of a 1-D signal in the guard condition while the closing guard condition explicitly defined the accessed value of the dimension be causing an error?

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInGuardExpressionCausesError) {
    performTestExecution("module main(out a[2](4)) if (a > 2) then skip else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, EmptyTrueBranchInIfStatementCausesError) {
    performTestExecution("module main(out a(4)) if (a > 2) then else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, EmptyFalseBranchInIfStatementCausesError) {
    performTestExecution("module main(out a(4)) if (a > 2) then skip else fi (a > 2)");
}

// Tests for production unary-statement
TEST_F(SyrecParserErrorTestsFixture, UsageOfReadonlyVariableInUnaryStatementCausesError) {
    performTestExecution("module main(in b(4)) ++=b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableInUnaryStatementCausesError) {
    performTestExecution("module main() ++=b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUnknownUnaryAssignmentOperationInUnaryStatementCausesError) {
    performTestExecution("module main(out b(4)) *=b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfExpressionInUnaryStatementCausesError) {
    performTestExecution("module main(in b(4)) ++= (b - 2)");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableInUnaryAssignmentCausesError) {
    performTestExecution("module main(in b(4)) for $i = 0 to 3 step 1 do ++= $i rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInEvaluatedVariableAccessInUnaryAssignmentCausesError) {
    performTestExecution("module main(in b[2](4)) ++=b");
}

// Tests for production assign-statement
TEST_F(SyrecParserErrorTestsFixture, UsageOfReadonlyVariableOnLhsOfAssignStatementCausesError) {
    performTestExecution("module main(in a(4), out b(4)) a += b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableOnLhsOfAssignStatementCausesError) {
    performTestExecution("module main(out b(4)) a += b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableOnRhsOfAssignStatementCausesError) {
    performTestExecution("module main(in a(4), out b(4)) a += c");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUnknownAssignOperationCausesError) {
    performTestExecution("module main(out b(4)) a := b");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingEqualSignFromAssignOperationCausesError) {
    performTestExecution("module main(out b(4)) a + b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableAsLhsOperandOfAssignmentCausesError) {
    performTestExecution("module main(out b(4)) for $i = 0 to 3 step 1 do $i += b rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInEvaluatedVariableAccessInLhsOperandOfAssignmentCausesError) {
    performTestExecution("module main(out b[2](4)) b += 2");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInBitwidthsOfOperandsOfAssignmentCausesError) {
    performTestExecution("module main(in a(4), out b(4)) a.1 += b.2:3");
}

// TODO: Tests for overlaping signals

// Tests for production swap-statement
TEST_F(SyrecParserErrorTestsFixture, UsageOfReadonlyVariableOnLhsOfSwapStatementCausesError) {
    performTestExecution("module main(in a(4), out b(4)) a <=> b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableOnLhsOfSwapStatementCausesError) {
    performTestExecution("module main(in a(4), out b(4)) c <=> b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfReadonlyVariableOnRhsOfSwapStatementCausesError) {
    performTestExecution("module main(in a(4), out b(4)) b <=> a");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableOnRhsOfSwapStatementCausesError) {
    performTestExecution("module main(inout a(4), out b(4)) a <=> c");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfExpressionOnLhsOfSwapStatementCausesError) {
    performTestExecution("module main(inout a(4), out b(4)) (a - 2) <=> b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfExpressionOnRhsOfSwapStatementCausesError) {
    performTestExecution("module main(inout a(4), out b(4)) a <=> (b - 2)");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsCausesError) {
    performTestExecution("module main(inout a(4), out b(4)) a.1 <=> b:2.3");
}

TEST_F(SyrecParserErrorTestsFixture, Non1DSignalInEvaluatedVaribleAccessOfLhsSwapOperandCausesError) {
    performTestExecution("module main(inout a[2](4), out b(4)) a <=> b");
}

TEST_F(SyrecParserErrorTestsFixture, Non1DSignalInEvaluatedVaribleAccessOfRhsSwapOperandCausesError) {
    performTestExecution("module main(inout a(4), out b[2](4)) a <=> b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfConstantOperandAsLhsSwapOperandCausesError) {
    performTestExecution("module main(out b(4)) 2 <=> b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfConstantOperandAsRhsSwapOperandCausesError) {
    performTestExecution("module main(out b(4)) b <=> 2");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidSwapOperationCausesError) {
    performTestExecution("module main(inout a(4), out b(4)) a => b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableAsLhsOperandOfSwapOperationCausesError) {
    performTestExecution("module main(out b(4)) for $i = 0 to 3 do $i <=> b rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableAsRhsOperandOfSwapOperationCausesError) {
    performTestExecution("module main(out b(4)) for $i = 0 to 3 do b <=> $i rof");
}

// TODO: Test for overlapping signal accesses

// Tests for production binary-expression
// TODO: Tests for truncation of values larger than the expected bitwidth
TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableInBinaryExpressionCausesError) {
    performTestExecution("module main(out b(4)) b += (a - 2)");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUnknownBinaryOperationInBinaryExpressionCausesError) {
    performTestExecution("module main(out b(4)) b += (a <=> 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketOfBinaryExpressionCausesError) {
    performTestExecution("module main(out b(4)) b += a + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketOfBinaryExpressionCausesError) {
    performTestExecution("module main(out b(4)) b += [a + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketOfBinaryExpressionCausesError) {
    performTestExecution("module main(out b(4)) b += (a + 2");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketOfBinaryExpressionCausesError) {
    performTestExecution("module main(out b(4)) b += (a + 2]");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInEvaluatedVariableAccessInLhsOperandOfBinaryExpressionCausesError) {
    performTestExecution("module main(out a(4), out b[2](4)) a += (b + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInEvaluatedVariableAccessInRhsOperandOfBinaryExpressionCausesError) {
    performTestExecution("module main(out a(4), out b[2](4)) a += (2 - b)");
}

// TODO: Tests for nested expressions

// Tests for production unary-expression
// TODO: Add tests when IR supports unary expressions

// Tests for production shift-expression
// TODO: Tests for truncation of values larger than the expected bitwidth
TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableInLhsOperandOfShiftExpressionCausesError) {
    performTestExecution("module main(out a(4), out b[2](4)) a += ((c << 2) + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableInRhsOperandOfShiftExpressionCausesError) {
    performTestExecution("module main(out a(4), out b[2](4)) a += ((b[0] << #c) + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidShiftOperationCausesError) {
    performTestExecution("module main(out a(4), out b[2](4)) a += ((b[0] <=> 2) + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingLhsOperandOfShiftExpressionCausesError) {
    performTestExecution("module main(out a(4), out b[2](4)) a += ((<< #b) + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingRhsOperandOfShiftExpressionCausesError) {
    performTestExecution("module main(out a(4), out b[2](4)) a += (b[1] + (b[0] >>))");
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericExpressionInRhsOperandOfShiftExpressionCausesError) {
    performTestExecution("module main(out a(4), out b[2](4)) a += ((b[0] << (b[1] - 2) + 2))");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableUsedAsLhsOperandOfShiftOperationCausesError) {
    performTestExecution("module main(out a(4), out b[2](4)) a += ((b >> #a) + 2)");
}

// Tests for production number
TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableInNumericExpressionCausesError) {
    performTestExecution("module main(out a(4), out b[2](4)) ++= a[(#c - 2)]");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidOperationInNumericExpressionCausesError) {
    performTestExecution("module main(out a(4), out b[2](4)) ++= a[(#a << 2)]");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBrackerOfNumericExpressionCausesError) {
    performTestExecution("module main(out a(4), out b[2](4)) ++= a[#a + 2)]");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidOpeningBracketInNumericExpressionCausesError) {
    performTestExecution("module main(out a(4), out b[2](4)) ++= a[{#a + 2)]");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBrackerOfNumericExpressionCausesError) {
    performTestExecution("module main(out a(4), out b[2](4)) ++= a[(#a + 2]");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidClosingBracketInNumericExpressionCausesError) {
    performTestExecution("module main(out a(4), out b[2](4)) ++= a[(#a + 2]]");
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInNumericExpressionCausesError) {
    performTestExecution("module main(out a(4), out b[2](4)) ++= a[((#a + 2) / 0)]");
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInEvaluatedNumericExpressionCausesError) {
    performTestExecution("module main(out a(4), out b[2](4)) ++= a[(#a - 4)]");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredLoopVariableInNumericExpressionCausesError) {
    performTestExecution("module main(out a(4), out b[2](4)) for $i = 0 to 3 step 1 do ++= a[(($i + 2) / 0)] rof");
}

// Tests for production signal
// TODO: Tests for indices out of range and division by zero errors in dynamic expressions (i.e. loop variables evaluated at compile time)
// TODO: Tests for truncation of values larger than the expected bitwidth
TEST_F(SyrecParserErrorTestsFixture, OmittingVariableIdentifierInVariableAccessCausesError) {
    performTestExecution("module main(out a(4)) ++= .0");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketForAccessOnDimensionOfVariableCausesError) {
    performTestExecution("module main(out a[2](4)) ++= a1].0");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketForAccessOnDimensionOfVariableCausesError) {
    performTestExecution("module main(out a[2](4)) ++= a(1].0");
}

TEST_F(SyrecParserErrorTestsFixture, IndexForAccessedValueOfDimensionOutOfRangeCausesError) {
    performTestExecution("module main(out a[2](4)) ++= a[2].0");
}

TEST_F(SyrecParserErrorTestsFixture, IndexForAccessedValueOfDimensionInNonConstantExpressionOutOfRangeCausesError) {
    performTestExecution("module main(out a[2](4)) ++= a[#a].0");
}

TEST_F(SyrecParserErrorTestsFixture, AccessedDimensionOfVariableOutOfRangeCausesError) {
    performTestExecution("module main(out a[2](4)) ++= a[0][1].0");
}

TEST_F(SyrecParserErrorTestsFixture, None1DSizeOfVariableAccessCausesError) {
    performTestExecution("module main(out a[2](4)) ++= a.0");
}

TEST_F(SyrecParserErrorTestsFixture, None1DAccessOnExpressionForValueOfDimensionCausesError) {
    performTestExecution("module main(out a[2](4), out b[2](2)) ++= a[b].0");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketForAccessOnDimensionOfVariableCausesError) {
    performTestExecution("module main(out a[2](4)) ++= a[0.0");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketForAccessOnDimensionOfVariableCausesError) {
    performTestExecution("module main(out a[2](4)) ++= a[0}.0");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingBitrangeStartSymbolCausesError) {
    performTestExecution("module main(out a[2](4)) ++= a[0]0");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidBitrangeStartSymbolCausesError) {
    performTestExecution("module main(out a[2](4)) ++= a[0]:0");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingBitrangEndSymbolCausesError) {
    performTestExecution("module main(out a[2](4)) ++= a[0].00");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidBitrangEndSymbolCausesError) {
    performTestExecution("module main(out a[2](4)) ++= a[0].0.0");
}

// TODO: Division by zero error are tested in tests for production 'number', should we explicitly tests the same behaviour for every usage of the production in other productions?
TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInDynamicBitrangeStartValueExpressionCausesError) {
    performTestExecution("module main(out a[2](4)) ++= a.(2 / (#a - 4))");
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInDynamicBitrangeEndValueExpressionCausesError) {
    performTestExecution("module main(out a[2](4)) ++= a.0:(2 / (#a - 4))");
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInDynamicExpressionForAccessValueOfDimensionCausesError) {
    performTestExecution("module main(out a[2](4)) ++= a[(2 / (#a - 4))]");
}

TEST_F(SyrecParserErrorTestsFixture, BitrangeStartValueIsConstantAndOutOfRangeCausesError) {
    performTestExecution("module main(out a[2](4)) ++= a.5");
}

TEST_F(SyrecParserErrorTestsFixture, BitrangeEndValueIsConstantAndOutOfRangeCausesError) {
    performTestExecution("module main(out a[2](4)) ++= a.0:5");
}

TEST_F(SyrecParserErrorTestsFixture, BitrangeStartValueIsDynamicExpressionAndOutOfRangeCausesError) {
    performTestExecution("module main(out a[2](4)) ++= a.(#a + 1):5");
}

TEST_F(SyrecParserErrorTestsFixture, BitrangeEndValueIsDynamicExpressionAndOutOfRangeCausesError) {
    performTestExecution("module main(out a[2](4)) ++= a.0:(#a + 1)");
}

TEST_F(SyrecParserErrorTestsFixture, TestError) {
    ASSERT_NO_FATAL_FAILURE(buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(0, 0), "test"));
    ASSERT_NO_FATAL_FAILURE(performTestExecution("bla"));
}