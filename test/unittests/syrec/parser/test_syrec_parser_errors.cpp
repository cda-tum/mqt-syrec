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

TEST_F(SyrecParserErrorTestsFixture, TestError) {
    ASSERT_NO_FATAL_FAILURE(buildAndRecordExpectedSemanticError<SemanticError::UnknownIdentifier>(Message::Position(0, 0), 'c'));
    ASSERT_NO_FATAL_FAILURE(performTestExecution("bla"));
}