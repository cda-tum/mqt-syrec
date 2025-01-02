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
        std::string stringifiedSyrecProgramToProcess;
        std::string stringifedExpectedOptimizedSyrecProgram;
    };

    std::string jsonKeyInTestCaseDataForCircuit                       = "circuit";
    std::string jsonKeyInTestCaseDataForExpectedOptimizedCircuit      = "optimizedCircuit";

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
    }

    static void assertKeyInJsonObjectExists(const json& jsonObject, const std::string& key) {
        ASSERT_TRUE(jsonObject.is_object());
        ASSERT_TRUE(jsonObject.contains(key)) << "Required key '" << key << "' was not found in the JSON object";
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

    virtual void performTestExecution() {
        if (!loadedTestCaseData.stringifedExpectedOptimizedSyrecProgram.empty())
            FAIL() << "Definition of expected optimized syrec program is not supported for now";

        std::string aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram;
        ASSERT_NO_FATAL_FAILURE(aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram = parserInstance.read(loadedTestCaseData.stringifiedSyrecProgramToProcess));
        ASSERT_TRUE(aggregateOfDetectedErrorsDuringProcessingOfSyrecProgram.empty());
        
        std::ostringstream containerForStringifiedProgram;
        ASSERT_NO_FATAL_FAILURE(assertStringificationOfParsedSyrecProgramIsSuccessful(parserInstance, containerForStringifiedProgram));
        ASSERT_EQ(containerForStringifiedProgram.str(), loadedTestCaseData.stringifiedSyrecProgramToProcess) << "SyReC program processed by the parser needs to match the user defined input circuit";
    }
};

#endif