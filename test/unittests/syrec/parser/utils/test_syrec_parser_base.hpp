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
        std::string stringifiedExpectedSyrecProgramContent;
    };

    std::string jsonKeyInTestCaseDataForCircuit                              = "inputCircuit";
    std::string jsonKeyInTestCaseDataForExpectedCircuit                      = "expectedCircuit";
    std::string jsonKeyInTestCaseDataForParserConfiguration                  = "parserConfig";
    std::string jsonKeyInTestCaseDataForDefaultSignalBitwidthInParserConfig  = "defaultBitwidth";
    std::string jsonKeyInTestCaseDataForConstantValueTrunctionOperation      = "constValTruncationOp";

    TestFromJson                              loadedTestCaseData;
    syrec::Program                            parserInstance;
    std::optional<syrec::ReadProgramSettings> userDefinedParserConfiguration;

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

        if (testCaseDataJson.contains(jsonKeyInTestCaseDataForExpectedCircuit)) {
            ASSERT_TRUE(testCaseDataJson.at(jsonKeyInTestCaseDataForExpectedCircuit).is_string()) << "Expected circuit processing result must be defined as a string";
            loadedTestCaseData.stringifiedExpectedSyrecProgramContent = testCaseDataJson.at(jsonKeyInTestCaseDataForExpectedCircuit).get<std::string>();
        } else {
            loadedTestCaseData.stringifiedExpectedSyrecProgramContent = loadedTestCaseData.stringifiedSyrecProgramToProcess;
        }

        if (testCaseDataJson.contains(jsonKeyInTestCaseDataForParserConfiguration)) {
            ASSERT_NO_FATAL_FAILURE(loadUserDefinedParserConfigurationFromJson(testCaseDataJson.at(jsonKeyInTestCaseDataForParserConfiguration)));   
        }
    }

    static void assertKeyInJsonObjectExists(const json& jsonObject, const std::string& key) {
        ASSERT_TRUE(jsonObject.is_object());
        ASSERT_TRUE(jsonObject.contains(key)) << "Required key '" << key << "' was not found in the JSON object";
    }

    static void assertStringificationOfParsedSyrecProgramIsSuccessful(const syrec::Program& syrecProgramToStringifiy, std::ostream& containerForStringifiedProgram) {
        // TODO: Troubleshooting as to why the stringification of the SyReC program failed is currently not possible but should only happen if either the IR representation of
        // the IR representation or of an internal error in the stringifier. Can we handle the former cases better?

        utils::BaseSyrecIrEntityStringifier::AdditionalFormattingOptions customFormattingOptions;
        customFormattingOptions.optionalCustomIdentationCharacterSequence = "";
        customFormattingOptions.optionalCustomNewlineCharacterSequence    = " ";

        utils::BaseSyrecIrEntityStringifier syrecProgramStringifier(customFormattingOptions);
        bool                                wasStringificationSuccessful;
        ASSERT_NO_FATAL_FAILURE(wasStringificationSuccessful = syrecProgramStringifier.stringify(containerForStringifiedProgram, syrecProgramToStringifiy)) << "Error during stringification of SyReC program";
        ASSERT_TRUE(wasStringificationSuccessful) << "Failed to stringify SyReC program";
    }

    void loadUserDefinedParserConfigurationFromJson(const json& jsonObject) {
        userDefinedParserConfiguration = syrec::ReadProgramSettings();
        ASSERT_TRUE(jsonObject.is_object()) << "User defined parser configuration needs to be defined as a json object";
        if (jsonObject.contains(jsonKeyInTestCaseDataForDefaultSignalBitwidthInParserConfig)) {
            ASSERT_TRUE(jsonObject.at(jsonKeyInTestCaseDataForDefaultSignalBitwidthInParserConfig).is_number_unsigned()) << "User defined default variable bitwidth needs to be defined as an unsigned integer";
            userDefinedParserConfiguration->defaultBitwidth = jsonObject.at(jsonKeyInTestCaseDataForDefaultSignalBitwidthInParserConfig).get<unsigned int>();
        }
        if (jsonObject.contains(jsonKeyInTestCaseDataForConstantValueTrunctionOperation)) {
            ASSERT_TRUE(jsonObject.at(jsonKeyInTestCaseDataForConstantValueTrunctionOperation).is_string());
            const auto& stringifiedTruncationOperation = jsonObject.at(jsonKeyInTestCaseDataForConstantValueTrunctionOperation).get<std::string>();
            if (stringifiedTruncationOperation == "modulo")
                userDefinedParserConfiguration->integerConstantTruncationOperation = utils::IntegerConstantTruncationOperation::Modulo;
            else if (stringifiedTruncationOperation == "bitwiseAnd")
                userDefinedParserConfiguration->integerConstantTruncationOperation = utils::IntegerConstantTruncationOperation::BitwiseAnd;
            else
                FAIL() << "No mapping for user defined integer constant value truncation operation '" << stringifiedTruncationOperation << "' defined";
        }
    }

    virtual void performTestExecution() {
        const syrec::ReadProgramSettings& parserConfiguration = userDefinedParserConfiguration.value_or(syrec::ReadProgramSettings());
        std::string aggregateOfDetectedErrorsDuringProcessingOfUserProvidedSyrecProgram;
        ASSERT_NO_FATAL_FAILURE(aggregateOfDetectedErrorsDuringProcessingOfUserProvidedSyrecProgram = parserInstance.readFromString(loadedTestCaseData.stringifiedSyrecProgramToProcess, parserConfiguration));
        ASSERT_TRUE(aggregateOfDetectedErrorsDuringProcessingOfUserProvidedSyrecProgram.empty()) << "Expected no errors to be reported when parsing the given SyReC program";

        std::string aggregateOfDetectedErrorsDuringProcessingOfExpectedOutputOfStringificationOfUserProvidedSyrecProgram;
        ASSERT_NO_FATAL_FAILURE(aggregateOfDetectedErrorsDuringProcessingOfExpectedOutputOfStringificationOfUserProvidedSyrecProgram = parserInstance.readFromString(loadedTestCaseData.stringifiedExpectedSyrecProgramContent, parserConfiguration));
        ASSERT_TRUE(aggregateOfDetectedErrorsDuringProcessingOfExpectedOutputOfStringificationOfUserProvidedSyrecProgram.empty()) << "Expected no errors to be reported when parsing the stringified version of the given SyReC program expected to be generated when stringifying the IR of the parser";

        std::ostringstream containerForStringifiedProgram;
        ASSERT_NO_FATAL_FAILURE(assertStringificationOfParsedSyrecProgramIsSuccessful(parserInstance, containerForStringifiedProgram));
        ASSERT_EQ(containerForStringifiedProgram.str(), loadedTestCaseData.stringifiedExpectedSyrecProgramContent) << "SyReC program processed by the parser needs to match the user defined input circuit";
    }
};

#endif