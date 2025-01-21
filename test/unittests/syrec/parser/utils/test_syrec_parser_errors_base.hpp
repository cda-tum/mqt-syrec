#ifndef UNITTESTS_SYREC_PARSER_UTILS_TEST_SYREC_PARSER_ERRORS_BASE_HPP
#define UNITTESTS_SYREC_PARSER_UTILS_TEST_SYREC_PARSER_ERRORS_BASE_HPP

#include "core/syrec/program.hpp"
#include "core/syrec/parser/utils/custom_error_mesages.hpp"
#include "core/syrec/parser/utils/parser_messages_container.hpp"
#include "core/syrec/parser/utils/variable_overlap_check.hpp"
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

    void performTestExecution(const std::string& stringifiedSyrecProgramToProcess, const syrec::ReadProgramSettings& userProvidedParserConfiguration = syrec::ReadProgramSettings()) const {
        syrec::Program program;

        std::string aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram;
        // We needed to modifiy the syrec::Program interface to allow processing of programs from a string due to the missing cross platform support
        // to create temporary or in-memory files (see mkstemp and fmemopen functions which are POSIX specific ones) without using the boost library.
        // Using the tmpfile/tmpfile_s of the C++ standard library is also not viable for the creating of temporary files due to the missing ability
        // to determine the path/filename of the generated file descriptor on all platforms.
        ASSERT_NO_FATAL_FAILURE(aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram = program.readFromString(stringifiedSyrecProgramToProcess, userProvidedParserConfiguration));
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
            if (const std::size_t lengthOfErrorMessage = currNewLineDelimiterPosition - lastFoundPositionOfNewlineDelimiter; lengthOfErrorMessage) {
                const auto actualCurrErrorMessage = aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram.substr(lastFoundPositionOfNewlineDelimiter, lengthOfErrorMessage);
                errorsDetectedDuringProcessingOfSyrecProgram.emplace_back(actualCurrErrorMessage);
            }

            // On Windows system we assume that the newline is encoded as the '\r\n' character sequence while on all other system it should be equal to the '\n' character
            lastFoundPositionOfNewlineDelimiter = currNewLineDelimiterPosition + 1;

            #if _WIN32
                ++lastFoundPositionOfNewlineDelimiter;
            #endif

            currNewLineDelimiterPosition = findNextNewlineDelimiterInString(aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram, lastFoundPositionOfNewlineDelimiter);
        }

        if (lastFoundPositionOfNewlineDelimiter < aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram.size()) {
            if (const std::size_t lengthOfLastErrorMessage = aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram.size() - lastFoundPositionOfNewlineDelimiter; lengthOfLastErrorMessage)
                errorsDetectedDuringProcessingOfSyrecProgram.emplace_back(aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram.substr(lastFoundPositionOfNewlineDelimiter, lengthOfLastErrorMessage));
        }
        ASSERT_NO_FATAL_FAILURE(assertExpectedAndActualErrorsMatch(expectedErrorsDetectedDuringProcessingOfSyrecProgram, errorsDetectedDuringProcessingOfSyrecProgram));
    }

    static void assertExpectedAndActualErrorsMatch(const MessagesContainer& expectedErrors, const std::vector<std::string_view>& actualErrorsInUnifiedFormat) {
        // TODO: Find better solution ot print errors
        ASSERT_EQ(expectedErrors.size(), actualErrorsInUnifiedFormat.size()) << "Expected " << expectedErrors.size() << " errors but " << actualErrorsInUnifiedFormat.size() << " were found";
        for (size_t errorIdx = 0; errorIdx < expectedErrors.size(); ++errorIdx) {
            ASSERT_EQ(expectedErrors.at(errorIdx).stringify(), actualErrorsInUnifiedFormat.at(errorIdx)) << "Error " << std::to_string(errorIdx) << ":\nExpected error: " << expectedErrors.at(errorIdx).stringify() << "\nActual Error: " << actualErrorsInUnifiedFormat.at(errorIdx);
        }
    }

    [[nodiscard]] static utils::VariableAccessOverlapCheckResult generateVariableAccessOverlappingIndicesDataContainer(const std::initializer_list<unsigned int>& accessedValuePerOverlappingDimension, unsigned int overlappingBit) {
        auto resultContainer                           = utils::VariableAccessOverlapCheckResult(utils::VariableAccessOverlapCheckResult::OverlapState::Overlapping);
        resultContainer.overlappingIndicesInformations = utils::VariableAccessOverlapCheckResult::OverlappingIndicesContainer({accessedValuePerOverlappingDimension, overlappingBit});
        return resultContainer;
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
#endif