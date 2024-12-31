#ifndef TEST_SYREC_PARSER_BASE_HPP
#define TEST_SYREC_PARSER_BASE_HPP

#include "core/syrec/program.hpp"

#include <string>
#include <fstream>

#include <fmt/core.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <core/utils/base_syrec_ir_entity_stringifier.hpp>

using json = nlohmann::json;

struct TestFromJsonConfig {
    std::string nameOfJsonFile;
    std::string keyOfTestInJsonFile;
    std::string testCaseName;

    explicit TestFromJsonConfig(const std::string& nameOfJsonFile, const std::string& keyOfTestInJsonFile, const std::string& testCaseName):
        nameOfJsonFile(nameOfJsonFile), keyOfTestInJsonFile(keyOfTestInJsonFile), testCaseName(testCaseName) {}
};

class SyrecParserBaseTestsFixture : public testing::TestWithParam<TestFromJsonConfig> {
protected:
    struct TestFromJson {
        std::string                stringifiedSyrecProgramToProcess;
        std::optional<std::string> stringifedExpectedOptimizedSyrecProgram;
        std::vector<std::string>   expectedErrors;
    };

    std::string jsonKeyInTestCaseDataForCircuit                       = "circuit";
    std::string jsonKeyInTestCaseDataForExpectedOptimizedCircuit      = "optimizedCircuit";
    std::string jsonKeyInTestCaseDataForExpectedErrors                = "expectedErrors";
    std::string jsonKeyInTestCaseDataForExpectedErrorMessage          = "msg";
    std::string jsonKeyInTestCaseDataForExpectedErrorPositionInLine   = "line";
    std::string jsonKeyInTestCaseDataForExpectedErrorPositionInColumn = "col";

    const std::string expectedErrorMessageFormat = "-- line {0:d} col {1:d}: {2:s}";
    TestFromJson      loadedTestCaseData;
    syrec::Program    parserInstance;

    void SetUp() override {
        const TestFromJsonConfig& testParameterData = GetParam();
        const std::string&        testCaseJsonKeyInFile = testParameterData.keyOfTestInJsonFile;
        std::ifstream inputFileStream(testParameterData.nameOfJsonFile, std::ios_base::in);
        ASSERT_TRUE(inputFileStream.good()) << "Input file @" << testParameterData.nameOfJsonFile << " is not in a usable state (e.g. does not exist)";

        const json parsedJsonDataOfFile = json::parse(inputFileStream);
        ASSERT_TRUE(parsedJsonDataOfFile.contains(testCaseJsonKeyInFile)) << "No matching entry with key '" << testCaseJsonKeyInFile << "' was found in the JSON test case data";

        const json& testCaseDataJson = parsedJsonDataOfFile[testCaseJsonKeyInFile];
        ASSERT_TRUE(testCaseDataJson.is_object()) << "Test case data with key '" << testCaseJsonKeyInFile << "' must be defined as a JSON object";
        ASSERT_TRUE(testCaseDataJson.contains(jsonKeyInTestCaseDataForCircuit)) << "Test case data did not contain expected key '" << jsonKeyInTestCaseDataForCircuit << "' for circuit to process";
        ASSERT_TRUE(testCaseDataJson.at(jsonKeyInTestCaseDataForCircuit).is_string()) << "Circuit to process must be defined as a string";
        loadedTestCaseData.stringifiedSyrecProgramToProcess = testCaseDataJson.at(jsonKeyInTestCaseDataForCircuit).get<std::string>();

        if (testCaseDataJson.contains(jsonKeyInTestCaseDataForExpectedOptimizedCircuit)) {
            ASSERT_TRUE(testCaseDataJson.at(jsonKeyInTestCaseDataForExpectedOptimizedCircuit).is_string()) << "Expected optimized circuit must be defined as a string";
            loadedTestCaseData.stringifedExpectedOptimizedSyrecProgram = testCaseDataJson.at(jsonKeyInTestCaseDataForExpectedOptimizedCircuit).get<std::string>();
        }

        if (testCaseDataJson.contains(jsonKeyInTestCaseDataForExpectedErrors)) {
            ASSERT_TRUE(testCaseDataJson.at(jsonKeyInTestCaseDataForExpectedErrors).is_array()) << "Expected errors detected during processing of unoptimized circuit must be defined as a JSON array";
            ASSERT_NO_FATAL_FAILURE(loadExpectedErrorsFromJson(testCaseDataJson.at(jsonKeyInTestCaseDataForExpectedErrors), loadedTestCaseData));
        }
    }

    static void assertKeyInJsonObjectExists(const json& jsonObject, const std::string& key) {
        ASSERT_TRUE(jsonObject.is_object());
        ASSERT_TRUE(jsonObject.contains(key)) << "Required key '" << key << "' was not found in the JSON object";
    }

    void loadExpectedErrorsFromJson(const json& testCaseDataFromJson, TestFromJson& parsedTestCaseDataFromJson) const {
        for (const auto& errorJsonObject : testCaseDataFromJson) {
            ASSERT_TRUE(errorJsonObject.is_object()) << "Expected test case data to a json object";
            ASSERT_NO_FATAL_FAILURE(assertKeyInJsonObjectExists(errorJsonObject, jsonKeyInTestCaseDataForExpectedErrorMessage));
            ASSERT_NO_FATAL_FAILURE(assertKeyInJsonObjectExists(errorJsonObject, jsonKeyInTestCaseDataForExpectedErrorPositionInLine));
            ASSERT_NO_FATAL_FAILURE(assertKeyInJsonObjectExists(errorJsonObject, jsonKeyInTestCaseDataForExpectedErrorPositionInColumn));

            ASSERT_TRUE(errorJsonObject.at(jsonKeyInTestCaseDataForExpectedErrorMessage).is_string()) << "Expected error message must be declared as a string in the JSON test case data";
            ASSERT_TRUE(errorJsonObject.at(jsonKeyInTestCaseDataForExpectedErrorPositionInLine).is_number_unsigned()) << "Expected line position of error must be defined as an unsigned integer in the JSON test case data";
            ASSERT_TRUE(errorJsonObject.at(jsonKeyInTestCaseDataForExpectedErrorPositionInColumn).is_number_unsigned()) << "Expected column position of error must be defined as an unsigned integer in the JSON test case data";

            ASSERT_NO_THROW(parsedTestCaseDataFromJson.expectedErrors.emplace_back(
                fmt::format(expectedErrorMessageFormat,
                    errorJsonObject.at(jsonKeyInTestCaseDataForExpectedErrorPositionInLine).get<unsigned int>(),
                    errorJsonObject.at(jsonKeyInTestCaseDataForExpectedErrorPositionInColumn).get<unsigned int>(),
                    errorJsonObject.at(jsonKeyInTestCaseDataForExpectedErrorMessage).get<std::string>()))
            );
        }
    }

    static void assertExpectedErrorsAreDetectedDuringProcessingOfSyrecProgram(const std::string_view& aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram, const std::vector<std::string>& expectedErrorsDetectedDuringProcessingOfSyrecProgram) {
        std::vector<std::string_view> errorsDetectedDuringProcessingOfSyrecProgram;
        // In the best case scenario, no further resizing of the container is necessary (i.e. the number of actually found errors is equal to the number of expected ones).
        errorsDetectedDuringProcessingOfSyrecProgram.reserve(expectedErrorsDetectedDuringProcessingOfSyrecProgram.size());

        std::size_t lastFoundPositionOfNewlineDelimiter   = 0;
        std::size_t currNewLineDelimiterPosition = findNextNewlineDelimiterInString(aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram, lastFoundPositionOfNewlineDelimiter);
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

    static void assertStringificationOfParsedSyrecProgramIsSuccessful(const syrec::Program& syrecProgramToStringifiy, std::ostream& containerForStringifiedProgram) {
        // TODO: Troubleshooting as to why the stringification of the SyReC program failed is currently not possible but should only happen if either the IR representation of
        // the IR representation or of an internal error in the stringifier. Can we handle the former cases better?
        utils::BaseSyrecIrEntityStringifier syrecProgramStringifier(std::nullopt);
        bool                                wasStringificationSuccessful;
        ASSERT_NO_FATAL_FAILURE(wasStringificationSuccessful = syrecProgramStringifier.stringify(containerForStringifiedProgram, syrecProgramToStringifiy)) << "Error during stringification of SyReC program";
        ASSERT_TRUE(wasStringificationSuccessful) << "Failed to stringify SyReC program";
    }

    // TODO: Stringification function to compare generated optimized circuit with user provided version
    // TODO: Processing of parser config from json

    static void assertExpectedAndActualErrorsMatch(const std::vector<std::string>& expectedErrors, const std::vector<std::string_view>& actualErrorsInUnifiedFormat) {
        // TODO: Find better solution ot print errors
        ASSERT_EQ(expectedErrors.size(), actualErrorsInUnifiedFormat.size()) << "Expected " << expectedErrors.size() << " errors but only " << actualErrorsInUnifiedFormat.size() << " were found";
        for (size_t errorIdx = 0; errorIdx < expectedErrors.size(); ++errorIdx) {
            ASSERT_EQ(expectedErrors.at(errorIdx), actualErrorsInUnifiedFormat.at(errorIdx)) << "Expected error: " << expectedErrors.at(errorIdx) << "| Actual Error: " << actualErrorsInUnifiedFormat.at(errorIdx);
        }
    }

    virtual void performTestExecution() {
        if (loadedTestCaseData.stringifedExpectedOptimizedSyrecProgram.has_value())
            FAIL() << "Definition of expected optimized syrec program is not supported for now";

        std::string aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram;
        ASSERT_NO_FATAL_FAILURE(aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram = parserInstance.read(loadedTestCaseData.stringifiedSyrecProgramToProcess));
        ASSERT_NO_FATAL_FAILURE(assertExpectedErrorsAreDetectedDuringProcessingOfSyrecProgram(aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram, loadedTestCaseData.expectedErrors));
        if (!loadedTestCaseData.expectedErrors.empty())
            return;

        std::ostringstream containerForStringifiedProgram;
        ASSERT_NO_FATAL_FAILURE(assertStringificationOfParsedSyrecProgramIsSuccessful(parserInstance, containerForStringifiedProgram));
        ASSERT_EQ(containerForStringifiedProgram.str(), loadedTestCaseData.stringifiedSyrecProgramToProcess) << "SyReC program processed by the parser needs to match the user defined input circuit";
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