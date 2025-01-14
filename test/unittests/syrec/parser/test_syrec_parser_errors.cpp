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

    void recordSyntaxError(Message::Position messagePosition, const std::string& messageText) {
        expectedErrorMessages.emplace_back(Message(Message::Type::Error, "SYNTAX", messagePosition, messageText));
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
    recordSyntaxError(Message::Position(1, 4), "mismatched input 'main' expecting 'module'");
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
    recordSyntaxError(Message::Position(1, 11), "missing '[' at ')'");
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
    recordSyntaxError(Message::Position(1, 25), "mismatched input ']' expecting {'in', 'out', 'inout'}");
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

// TODO: Tests for overload of main module (not using identifier 'main')

// Tests for production parameter
TEST_F(SyrecParserErrorTestsFixture, OmittingVariableTypeCausesError) {
    recordSyntaxError(Message::Position(1, 12), "mismatched input 'a' expected {'in', 'out', 'inout', ')'}");
    performTestExecution("module main(a[2](16)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidVariableTypeCausesError) {
    recordSyntaxError(Message::Position(1, 12), "mismatched input 'int' expected {'in', 'out', 'inout', ')'}");
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
    recordSyntaxError(Message::Position(1, 16), "token recognition error at '{'");
    recordSyntaxError(Message::Position(1, 17), "mismatched input '16' expecting ')'");
    performTestExecution("module main(in a{16](2)) skip");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingVariableValuesForDimensionClosingBracketCausesError) {
    recordSyntaxError(Message::Position(1, 18), "missing ']' at ')'");
    performTestExecution("module main(in a[2) skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidVariableValuesForDimensionClosingBracketCausesError) {
    recordSyntaxError(Message::Position(1, 18), "token recognition error at '}'");
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
    recordSyntaxError(Message::Position(1, 21), "extraneous input ']' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
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
    recordSyntaxError(Message::Position(1, 47), "mismatched input ',' expecting '(");
    performTestExecution("module main(in a(4), in b(4), out c(4)) call (a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleIdentifierNotMatchingAnyDeclaredModuleIdentifierInCallStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingIdentifier>(Message::Position(1, 44), "add");
    performTestExecution("module main(in a(4), in b(4), out c(4)) call add(a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingCallStatementParameterOpeningBracket) {
    recordSyntaxError(Message::Position(1, 101), "mismatched input ',' expecting '('");
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingIdentifier>(Message::Position(1, 97), "adda");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call adda, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidCallStatementParameterOpeningBracket) {
    recordSyntaxError(Message::Position(1, 100), "mismatched input '[' expecting '('");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call add[a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingCallStatementParameterIdentifierCausesError) {
    recordSyntaxError(Message::Position(1, 101), "extraneous input ',' expecting IDENT");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call add(, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, CallStatementParameterIdentifierNotMatchingAnyDeclaredVariableCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 101), "d");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call add(d, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingCallStatementParameterDelimiterCausesError) {
    recordSyntaxError(Message::Position(1, 103), "extraneous input 'b' expecting {',', ')'}");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) call add(a b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingUncallKeywordCausesError) {
    recordSyntaxError(Message::Position(1, 95), "no viable alternative at input 'add('");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) add(a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidUncallKeywordCausesError) {
    recordSyntaxError(Message::Position(1, 95), "no viable alternative at input 'performCall add'");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) performCall add(a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingModuleIdentiferInUncallStatementCausesError) {
    recordSyntaxError(Message::Position(1, 47), "extraneous input '(' expecting IDENT");
    recordSyntaxError(Message::Position(1, 49), "extraneous input ',' expecting '('");
    performTestExecution("module main(in a(4), in b(4), out c(4)) uncall (a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, ModuleIdentifierNotMatchingAnyDeclaredModuleIdentifierInUncallStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoModuleMatchingIdentifier>(Message::Position(1, 47), "add");
    performTestExecution("module main(in a(4), in b(4), out c(4)) uncall add(a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingUncallStatementParameterOpeningBracket) {
    recordSyntaxError(Message::Position(1, 103), "mismatched input ',' expecting '('");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall adda, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidUncallStatementParameterOpeningBracket) {
    recordSyntaxError(Message::Position(1, 102), "mismatched input '[' expecting '('");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall add[a, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingUncallStatementParameterIdentifierCausesError) {
    recordSyntaxError(Message::Position(1, 103), "extraneous input ',' expecting IDENT");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall add(, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, UncallStatementParameterIdentifierNotMatchingAnyDeclaredVariableCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 103), "d");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall add(d, b, c)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingUncallStatementParameterDelimiterCausesError) {
    recordSyntaxError(Message::Position(1, 105), "extraneous input 'b' expecting {',', ')'}");
    performTestExecution("module add(in a(4), in b(4), out c(4)) c ^= (a + b) module main(in a(4), in b(4), out c(4)) uncall add(a b, c)");
}

// TODO: Overload tests
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

// Tests for production for-statement
TEST_F(SyrecParserErrorTestsFixture, OmittingForStatementKeywordCausesError) {
    recordSyntaxError(Message::Position(1, 31), "extraneous input '$' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    recordSyntaxError(Message::Position(1, 34), "no viable alternative at input 'i = '");
    performTestExecution("module main(in a(4), out b(4)) $i = 0 to 3 do b.$i ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidForStatementKeywordCausesError) {
    recordSyntaxError(Message::Position(1, 31), "mismatched inputed 'do' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main(in a(4), out b(4)) do $i = 0 to 3 do b.$i ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingLoopVariableIdentPrefixCausesError) {
    recordSyntaxError(Message::Position(1, 35), "mismatched input 'i' expecting {'$', '#', '(', INT}");
    performTestExecution("module main(in a(4), out b(4)) for i = 0 to 3 do b.$i ^= 1 rof");
}

// TODO: 
TEST_F(SyrecParserErrorTestsFixture, OmittingLoopVariableInitialValueInitializationEqualSignCausesError) {
    recordSyntaxError(Message::Position(1, 38), "mismatched input '0' expecting '='");
    performTestExecution("module main(in a(4), out b(4)) for $i 0 to 3 do b.$i ^= 1 rof");
}

TEST_F(SyrecParserErrorTestsFixture, NonNumericLoopVariableInitialValueInitializationCausesError) {
    recordSyntaxError(Message::Position(1, 38), "mismatched input 'b' expecting {'$', '#', '(', INT}");
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
    recordSyntaxError(Message::Position(1, 37), "no viable alternative at '3 incr'");
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
    recordSyntaxError(Message::Position(1, 48), "mismatched input 'rof' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main(in a(4), out b(4)) for $i=0 to 3 do rof");
}

TEST_F(SyrecParserErrorTestsFixture, DuplicateDefinitionOfLoopVariableInNestedLoopCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::DuplicateVariableDeclaration>(Message::Position(1, 25), "$i");
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
    recordSyntaxError(Message::Position(1, 52), "no viable alternative at input 'done'");
    recordSyntaxError(Message::Position(1, 58), "missing 'rof' at '<EOF>'");
    performTestExecution("module main(in a(4), out b(4)) for 3 step 1 do done; ++=b");
}

// Tests for production if-statement
TEST_F(SyrecParserErrorTestsFixture, OmittingIfKeywordCausesError) {
    recordSyntaxError(Message::Position(1, 22), "extraneous input '(' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    recordSyntaxError(Message::Position(1, 25), "no viable alternative at input 'a >'");
    performTestExecution("module main(out a(4)) (a > 2) then skip else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidIfKeywordCausesError) {
    recordSyntaxError(Message::Position(1, 24), "extraneous input '-' expecting {'!', '~', '$', '#', '(', IDENT, INT}");
    recordSyntaxError(Message::Position(1, 30), "mismatched input '(' expecting 'then'");
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
    recordSyntaxError(Message::Position(1, 52), "mismatched input '<EOF' expecting 'fi'");
    performTestExecution("module main(out a(4)) if (a > 2) then skip else skip");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidFiKeywordAfterGuardConditionCausesError) {
    recordSyntaxError(Message::Position(1, 53), "missing 'fi' at 'done'");
    recordSyntaxError(Message::Position(1, 58), "extraneous input '(' expecting {<EOF>, 'module'}");
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
    recordSyntaxError(Message::Position(1, 62), "missing ']' expecting ')'");
    performTestExecution("module main(out a(4)) if (a > 2) then skip else skip fi (a > 2]");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchBetweenGuardAndClosingGuardConditionBasedOnBitwidthCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 58));
    performTestExecution("module main(out a(4)) if (a.0 > 2) then skip else skip fi (a.1:2 > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchBetweenGuardAndClosingGuardConditionBasedOnAccessedValueOfDimensionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IfGuardExpressionMissmatch>(Message::Position(1, 62));
    performTestExecution("module main(out a[2](4)) if (a[0] > 2) then skip else skip fi (a[2] > 2)");
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

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInGuardExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::TooFewDimensionsAccessed>(Message::Position(1, 28), 0, 1);
    performTestExecution("module main(out a[2](4)) if (a > 2) then skip else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableWithNotCompletelySpecifiedDimensionAccessInGuardExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::TooFewDimensionsAccessed>(Message::Position(1, 31), 1, 2);
    performTestExecution("module main(out a[2][3](4)) if (a[1] > 2) then skip else skip fi (a[1] > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, EmptyTrueBranchInIfStatementCausesError) {
    recordSyntaxError(Message::Position(1, 38), "extraneous input 'else' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    recordSyntaxError(Message::Position(1, 48), "mismatched input 'fi' expecting 'else'");
    performTestExecution("module main(out a(4)) if (a > 2) then else skip fi (a > 2)");
}

TEST_F(SyrecParserErrorTestsFixture, EmptyFalseBranchInIfStatementCausesError) {
    recordSyntaxError(Message::Position(1, 48), "extraneous input 'fi' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
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
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 20), "c");
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 91), "c");
    performTestExecution("module main(in a(4)) wire b(4) if ((2 > 1) || (c > a)) then ++= b else skip fi ((2 > 1) || (c > a))");
}

// Tests for production unary-statement
TEST_F(SyrecParserErrorTestsFixture, UsageOfReadonlyVariableInUnaryStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::AssignmentToReadonlyVariable>(Message::Position(1, 24), "b");
    performTestExecution("module main(in b(4)) ++= b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableInUnaryStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 17), "b");
    performTestExecution("module main() ++= b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUnknownUnaryAssignmentOperationInUnaryStatementCausesError) {
    recordSyntaxError(Message::Position(1, 22), "mismatched input '*' expecting {'++=', '--=', '~=', 'call', 'uncall', 'wire', 'state', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main(out b(4)) *= b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfExpressionInUnaryStatementCausesError) {
    recordSyntaxError(Message::Position(1, 25), "extraneous input '(' expecting IDENT");
    recordSyntaxError(Message::Position(1, 25), "extraneous input '-' expecting {<EOF>, 'module'}");
    performTestExecution("module main(in b(4)) ++= (b - 2)");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableInUnaryAssignmentCausesError) {
    recordSyntaxError(Message::Position(1, 51), "extraneous input '$' expecting IDENT");
    performTestExecution("module main(in b(4)) for $i = 0 to 3 step 1 do ++= $i rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInEvaluatedVariableAccessInUnaryAssignmentCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::TooFewDimensionsAccessed>(Message::Position(1, 28), 0, 1);
    performTestExecution("module main(in b[2](4)) ++= b");
}

// Tests for production assign-statement
TEST_F(SyrecParserErrorTestsFixture, UsageOfReadonlyVariableOnLhsOfAssignStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::AssignmentToReadonlyVariable>(Message::Position(1, 36), "b");
    performTestExecution("module main(in a(4), out b(4)) a += b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableOnLhsOfAssignStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 22), "a");
    performTestExecution("module main(out b(4)) a += b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableOnRhsOfAssignStatementCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 36), "c");
    performTestExecution("module main(in a(4), out b(4)) a += c");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUnknownAssignOperationCausesError) {
    recordSyntaxError(Message::Position(1, 22), "no viable alternative at input 'a :'");
    performTestExecution("module main(out b(4)) a := b");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingEqualSignFromAssignOperationCausesError) {
    recordSyntaxError(Message::Position(1, 22), "no viable alternative at input 'a +'");
    performTestExecution("module main(out b(4)) a + b");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableAsLhsOperandOfAssignmentCausesError) {
    recordSyntaxError(Message::Position(1, 48), "extraneous input '$' expecting {'++=', '--=', '~=', 'call', 'uncall', 'for', 'if', 'skip', IDENT}");
    performTestExecution("module main(out b(4)) for $i = 0 to 3 step 1 do $i += b rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInEvaluatedVariableAccessInLhsOperandOfAssignmentCausesError) {
    // TODO: Define semantic error
    buildAndRecordExpectedSemanticError<SemanticError::OmittingDimensionAccessOnlyPossibleFor1DSignalWithSingleValue>(Message::Position(1, 24), "b");
    performTestExecution("module main(out b[2](4)) b += 2");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInBitwidthsOfOperandsOfAssignmentCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 38), 1, 2);
    performTestExecution("module main(in a(4), out b(4)) a.1 += b.2:3");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessOnBitUsingOnlyConstantIndicesDefinedInAssignmentRhsCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 42), "TODO");
    performTestExecution("module main(inout a[2](4)) a[0].1 += (1 + a[0].1)");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessOnBitrangeUsingOnlyConstantIndicesDefinedInAssignmentRhsCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 44), "TODO");
    performTestExecution("module main(inout a[2](4)) a[0].1:3 += (1 + a[0].1:3)");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessOnBitrangeWithBitrangeStartBeingConstantIndexDefinedInAssignmentRhsCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 71), "TODO");
    performTestExecution("module main(inout a[2](4)) for $i = 0 to 3 step 1 do a[0].1:$i += (1 + a[0].1:$i)");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessOnBitrangeWithBitrangeEndBeingConstantIndexDefinedInAssignmentRhsCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 72), "TODO");
    performTestExecution("module main(inout a[2](4)) for $i = 0 to 3 step 1 do a[0].$i:#a += (1 + a[0].$i:#a)");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessBetweenBitAccessOnLhsAndBitrangeAccessOnRhsWithBitrangeStartOfLatterOverlappingFormerOfAssignmentCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 36), "TODO");
    performTestExecution("module main(inout a(4)) a.0 += (1 + a.0:1)");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessBetweenBitAccessOnLhsAndBitrangeAccessOnRhsWithBitrangeEndOfLatterOverlappingFormerOfAssignmentCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 36), "TODO");
    performTestExecution("module main(inout a(4)) a.0 += (1 + a.1:0)");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessBetweenBitAccessOnRhsAndBitrangeAccessOnLhsWithBitrangeStartOfLatterOverlappingFormerOfAssignmentCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 38), "TODO");
    performTestExecution("module main(inout a(4)) a.0:1 += (1 + a.0)");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessBetweenBitAccessOnRhsAndBitrangeAccessOnLhsWithBitrangeEndOfLatterOverlappingFormerOfAssignmentCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 38), "TODO");
    performTestExecution("module main(inout a(4)) a.1:0 += (1 + a.1)");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessOnSameValueOfDimensionInAssignmentCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 40), "TODO");
    performTestExecution("module main(inout a[2](4)) a[0] += (1 + a[0])");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessOn1DVariableWithSingleValueAndAccessedDimensionNotExplicitlyDefinedInAssignmentCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 34), "TODO");
    performTestExecution("module main(inout a(4)) a += (1 + a)");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessOn1DVariableWithSingleValueAndAccessedDimensionExplicitlyDefinedInAssignmentCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 37), "TODO");
    performTestExecution("module main(inout a(4)) a[0] += (1 + a[0])");
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
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 42), 4, 1);
    performTestExecution("module main(inout a(4), out b(4)) a <=> b.1");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsDefiningNoBitAccessAndRhsDefiningBitrangeAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 42), 4, 2);
    performTestExecution("module main(inout a(4), out b(4)) a <=> b.0:1");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsDefiningNoBitAccessAndRhsDefiningBitrangeWithStartLargerThanEndAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 42), 4, 2);
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
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 42), 2, 4);
    performTestExecution("module main(inout a(4), out b(4)) a.1:2 <=> b");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingBitrangeAccessWithStartLargerThanEndAndRhsDefiningNoBitrangeAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 42), 2, 4);
    performTestExecution("module main(inout a(4), out b(4)) a.2:1 <=> b");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingBitrangeAccessAndRhsDefiningBitAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 42), 2, 1);
    performTestExecution("module main(inout a(4), out b(4)) a.2:3 <=> b.1");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingBitrangeAccessWithStartLargerThanEndAndRhsDefiningBitAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 42), 2, 1);
    performTestExecution("module main(inout a(4), out b(4)) a.3:2 <=> b.1");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingBitrangeAccessAndRhsDefiningBitrangeAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 42), 2, 3);
    performTestExecution("module main(inout a(4), out b(4)) a.0:1 <=> b.1:3");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingBitrangeAccessAndRhsDefiningBitrangeWithStartLargerThanEndAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 42), 2, 3);
    performTestExecution("module main(inout a(4), out b(4)) a.0:1 <=> b.3:1");
}

TEST_F(SyrecParserErrorTestsFixture, MissmatchInSwapOperationBitwidthsWithLhsBeingBitrangeAccessWithStartLargerThanEndAndRhsDefiningBitrangeWithStartLargerThanEndAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionBitwidthMissmatches>(Message::Position(1, 42), 2, 3);
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
    performTestExecution("module main(out b(4)) for $i = 0 to 3 do $i <=> b rof");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfLoopVariableAsRhsOperandOfSwapOperationCausesError) {
    recordSyntaxError(Message::Position(1, 47), "extraneous input '$' expecting IDENT");
    performTestExecution("module main(out b(4)) for $i = 0 to 3 do b <=> $i rof");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessOnBitUsingOnlyConstantIndicesDefinedInSwapRhsCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 37));
    performTestExecution("module main(inout a[2](4)) a[0].1 <=> a[0].1");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessOnBitrangeUsingOnlyConstantIndicesDefinedInSwapRhsCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 40));
    performTestExecution("module main(inout a[2](4)) a[0].1:3 <=> a[0].1:3");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessOnBitrangeWithBitrangeStartBeingConstantIndexDefinedInSwapRhsCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 67));
    performTestExecution("module main(inout a[2](4)) for $i = 0 to 3 step 1 do a[0].1:$i <=> a[0].1:$i");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessOnBitrangeWithBitrangeEndBeingConstantIndexDefinedInSwapRhsCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 68));
    performTestExecution("module main(inout a[2](4)) for $i = 0 to 3 step 1 do a[0].$i:#a <=> a[0].$i:#a");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessBetweenBitAccessOnLhsAndBitrangeAccessOnRhsWithBitrangeStartOfLatterOverlappingFormerOfSwapCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 32));
    performTestExecution("module main(inout a(4)) a.0 <=> a.0:1");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessBetweenBitAccessOnLhsAndBitrangeAccessOnRhsWithBitrangeEndOfLatterOverlappingFormerOfSwapCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 32));
    performTestExecution("module main(inout a(4)) a.0 <=> a.1:0");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessBetweenBitAccessOnRhsAndBitrangeAccessOnLhsWithBitrangeStartOfLatterOverlappingFormerOfSwapCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 34));
    performTestExecution("module main(inout a(4)) a.0:1 <=> a.0");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessBetweenBitAccessOnRhsAndBitrangeAccessOnLhsWithBitrangeEndOfLatterOverlappingFormerOfSwapCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 34));
    performTestExecution("module main(inout a(4)) a.1:0 <=> a.1");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessOnSameValueOfDimensionOfSwapCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 36));
    performTestExecution("module main(inout a[2](4)) a[0] <=> a[0]");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessOn1DVariableWithSingleValueAndAccessedDimensionNotExplicitlyInSwapDefinedCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 30));
    performTestExecution("module main(inout a(4)) a <=> a");
}

TEST_F(SyrecParserErrorTestsFixture, OverlappingVariableAccessOn1DVariableWithSingleValueAndAccessedDimensionExplicitlyDefinedInSwapCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 33));
    performTestExecution("module main(inout a(4)) a[0] <=> a[0]");
}


// TODO: Test for overlapping signal accesses (using only constant indices or dynamic expressions with resolvable operands

// Tests for production binary-expression
// TODO: Tests for truncation of values larger than the expected bitwidth
// TODO: If short circuit evaluation of a binary expression can be performed, should semantic errors be reported? 
TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableInBinaryExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 28), "");
    performTestExecution("module main(out b(4)) b += (a - 2)");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUnknownBinaryOperationInBinaryExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 30), "no viable alternative at input '(a <=>'");
    performTestExecution("module main(out b(4)) b += (a <=> 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketOfBinaryExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 29), "extraneous input '+' expecting {<EOF>, 'module'}");
    performTestExecution("module main(out b(4)) b += a + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketOfBinaryExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 27), "extraneous input '[' expecting {'!', '~', '$', '#', '(', IDENT, INT}");
    recordSyntaxError(Message::Position(1, 30), "extraneous input '+' expecting {<EOF>, 'module'}");
    performTestExecution("module main(out b(4)) b += [a + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketOfBinaryExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 33), "missing ')' at '<EOF>'");
    performTestExecution("module main(out b(4)) b += (a + 2");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidClosingBracketOfBinaryExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 33), "mismatched input ']' expecting ')'");
    performTestExecution("module main(out b(4)) b += (a + 2]");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInEvaluatedVariableAccessInLhsOperandOfBinaryExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::TooFewDimensionsAccessed>(Message::Position(1, 41), 0, 1);
    performTestExecution("module main(out a(4), out b[2](4)) a += (b + 2)");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfNon1DVariableInEvaluatedVariableAccessInRhsOperandOfBinaryExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::TooFewDimensionsAccessed>(Message::Position(1, 44), 0, 1);
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
    buildAndRecordExpectedSemanticError<SemanticError::TooFewDimensionsAccessed>(Message::Position(1, 42), 0, 1);
    performTestExecution("module main(out a(4), out b[2](4)) a += ((b >> #a) + 2)");
}

// Tests for production number
TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredVariableInNumericExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 43), "c");
    performTestExecution("module main(out a(4), out b[2](4)) ++= a[(#c - 2)]");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidOperationInNumericExpressionCausesError) {
    performTestExecution("module main(out a(8), out b[2](2)) ++= a.(#b << 2)");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingOpeningBracketOfNumericExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 44), "mismatched input '+' expecting ']'");
    performTestExecution("module main(out a(4), out b[2](4)) ++= a[#a + 2)]");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidOpeningBracketInNumericExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 41), "token recognition error at: '{'");
    recordSyntaxError(Message::Position(1, 45), "mismatched input '+' expecting ']'");
    performTestExecution("module main(out a(4), out b[2](4)) ++= a[{#a + 2)]");
}

TEST_F(SyrecParserErrorTestsFixture, OmittingClosingBracketOfNumericExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 48), "missing ')' at '<EOF>'");
    performTestExecution("module main(out a(4), out b[2](4)) ++= a.(#a + 2");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfInvalidClosingBracketInNumericExpressionCausesError) {
    recordSyntaxError(Message::Position(1, 48), "mismatched input ']' expecting ')'");
    performTestExecution("module main(out a(4), out b[2](4)) ++= a.(#a + 2]");
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInNumericExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 40));
    performTestExecution("module main(out a(4), out b[2](4)) ++= a.(2 / 0)");
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInEvaluatedNumericExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 40));
    performTestExecution("module main(out a(4), out b[2](4)) ++= a.(2 / (#a - 4))");
}

TEST_F(SyrecParserErrorTestsFixture, UsageOfUndeclaredLoopVariableInNumericExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::NoVariableMatchingIdentifier>(Message::Position(1, 68), "$i");
    performTestExecution("module main(out a(4), out b[2](4)) for $i = 0 to 3 step 1 do ++= a.(($i + 2) / 0) rof");
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
    performTestExecution("module main(out a[2](4)) ++= a1].0");
}

TEST_F(SyrecParserErrorTestsFixture, InvalidOpeningBracketForAccessOnDimensionOfVariableCausesError) {
    recordSyntaxError(Message::Position(1, 30), "extraneous input '(' expecting {<EOF>, 'module'}");
    performTestExecution("module main(out a[2](4)) ++= a(1].0");
}

TEST_F(SyrecParserErrorTestsFixture, IndexForAccessedValueOfDimensionOutOfRangeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedValueForDimensionOutOfRange>(Message::Position(1, 41), 2, 0, 2);
    performTestExecution("module main(out a[2](4)) ++= a[2].0");
}

TEST_F(SyrecParserErrorTestsFixture, IndexForAccessedValueOfDimensionInNonConstantExpressionOutOfRangeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedValueForDimensionOutOfRange>(Message::Position(1, 41), 4, 0, 2);
    performTestExecution("module main(out a[2](4)) ++= a[#a].0");
}

TEST_F(SyrecParserErrorTestsFixture, AccessedDimensionOfVariableOutOfRangeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::TooManyDimensionsAccessed>(Message::Position(1, 1), 2, 1);
    performTestExecution("module main(out a[2](4)) ++= a[0][1].0");
}

TEST_F(SyrecParserErrorTestsFixture, None1DSizeOfVariableAccessCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::TooFewDimensionsAccessed>(Message::Position(1, 29), 0, 1);
    performTestExecution("module main(out a[2](4)) ++= a.0");
}

TEST_F(SyrecParserErrorTestsFixture, None1DAccessOnExpressionForValueOfDimensionCausesError) {
    //buildAndRecordExpectedSemanticError<SemanticError::TODO>(Message::Position(1, 44), "TODO");
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
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 41));
    performTestExecution("module main(out a[2](4)) ++= a.(2 / (#a - 4))");
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInDynamicBitrangeEndValueExpressionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 33));
    performTestExecution("module main(out a[2](4)) ++= a.0:(2 / (#a - 4))");
}

TEST_F(SyrecParserErrorTestsFixture, DivisionByZeroInDynamicExpressionForAccessValueOfDimensionCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::ExpressionEvaluationFailedDueToDivisionByZero>(Message::Position(1, 41));
    performTestExecution("module main(out a[2](4)) ++= a[(2 / (#a - 4))]");
}

TEST_F(SyrecParserErrorTestsFixture, BitrangeStartValueIsConstantAndOutOfRangeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 31), 5, 4);
    performTestExecution("module main(out a[2](4)) ++= a.5");
}

TEST_F(SyrecParserErrorTestsFixture, BitrangeEndValueIsConstantAndOutOfRangeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 33), 5, 4);
    performTestExecution("module main(out a[2](4)) ++= a.0:5");
}

TEST_F(SyrecParserErrorTestsFixture, BitrangeStartValueIsDynamicExpressionAndOutOfRangeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 31), 5, 4);
    performTestExecution("module main(out a[2](4)) ++= a.(#a + 1):3");
}

TEST_F(SyrecParserErrorTestsFixture, BitrangeEndValueIsDynamicExpressionAndOutOfRangeCausesError) {
    buildAndRecordExpectedSemanticError<SemanticError::IndexOfAccessedBitOutOfRange>(Message::Position(1, 33), 5, 4);
    performTestExecution("module main(out a[2](4)) ++= a.0:(#a + 1)");
}