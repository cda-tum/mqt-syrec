#ifndef TEST_SYREC_PARSER_UTILS_HPP
#define TEST_SYREC_PARSER_UTILS_HPP

using json = nlohmann::json;

namespace syrecParserTestUtils {
    constexpr auto TEST_NAME_NOT_ALLOWED_CHARACTER       = '-';
    constexpr auto TEST_NAME_COMPONENT_DELIMITER_SYMBOL  = '_';
    constexpr auto GTEST_IGNORE_PREFIX                   = "DISABLED";
    constexpr auto ERROR_REASON_JSON_DATA_NOT_OBJECT     = "DISABLED_TOPLEVEL_JSON_OBJECT_NOT_FOUND";
    constexpr auto ERROR_REASON_JSON_DATA_INVALID        = "DISABLED_JSON_DATA_INVALID";
    constexpr auto ERROR_REASON_FAILED_TO_LOAD_FROM_FILE = "DISABLED_FAILED_TO_LOAD_TEST_DATA_FROM_FILE";

    struct FilenameAndTestNamePrefix {
        std::string relativePath;
        std::string filename;
        std::string namePrefixForTestsLoadedFromFile;

        size_t operator()(const FilenameAndTestNamePrefix& entityToHash) const {
            // For now we use an often referred to implementation: https://stackoverflow.com/a/2595226 that is sufficient for our use case
            std::hash<std::string> hasher;
            std::size_t            seed = 0;
            seed ^= hasher(entityToHash.relativePath) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hasher(entityToHash.filename) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }

        bool operator==(const FilenameAndTestNamePrefix& other) const {
            return relativePath == other.relativePath && filename == other.filename;
        }
    };

    /*
     * Only non-empty ASCII alphanumeric characters without underscores are supported as tests names
     * see https://google.github.io/googletest/advanced.html#specifying-names-for-value-parameterized-test-parameters
     */
    inline bool doesStringContainOnlyAsciiCharactersOrUnderscores(const std::string_view& stringToCheck) {
        return std::all_of(
                stringToCheck.cbegin(),
                stringToCheck.cend(),
                [](const char characterToCheck) {
                    return std::isalnum(characterToCheck) || characterToCheck == TEST_NAME_COMPONENT_DELIMITER_SYMBOL;
                });
    }

    inline std::string concatinateStrings(const std::string& lString, const char delimiter, const std::initializer_list<std::string>& otherComponents) {
        std::string resultContainer      = lString;
        for (const auto& toBeConcatinatedComponents: otherComponents) {
            resultContainer += delimiter;
            resultContainer += toBeConcatinatedComponents;
        }
        return resultContainer;
    }

    inline std::string replaceInvalidCharactersInTestCaseName(const std::string& testCaseName) {
        if (testCaseName.find_first_of(TEST_NAME_NOT_ALLOWED_CHARACTER) == std::string::npos)
            return testCaseName;

        std::string transformedTestCaseName         = testCaseName;
        const auto  replacementForInvalidCharacters = std::to_string(TEST_NAME_COMPONENT_DELIMITER_SYMBOL);
        for (std::size_t indexOfNextInvalidCharacter = 0; (indexOfNextInvalidCharacter = testCaseName.find_first_of(TEST_NAME_NOT_ALLOWED_CHARACTER, indexOfNextInvalidCharacter) != std::string::npos); indexOfNextInvalidCharacter += testCaseName.size())
            transformedTestCaseName.replace(indexOfNextInvalidCharacter, 1, replacementForInvalidCharacters);
        return transformedTestCaseName;
    }

    inline std::string extractFilenameWithoutFileExtension(const std::string& filename) {
        if (const std::size_t fileExtensionPosition = filename.find_last_of('.'); fileExtensionPosition != std::string::npos) {
            return filename.substr(0, fileExtensionPosition);
        }
        return filename;
    }

    inline std::vector<std::string> loadTestCaseNamesFromFile(const std::string& relativePathToFile, const std::string& inputFileName, const std::string& testCaseNamePrefix) {
        std::ifstream inputFileStream(relativePathToFile + "/" + inputFileName, std::ios_base::in);
        if (!inputFileStream.good())
            return {concatinateStrings(ERROR_REASON_FAILED_TO_LOAD_FROM_FILE, TEST_NAME_COMPONENT_DELIMITER_SYMBOL, {extractFilenameWithoutFileExtension(inputFileName)})};

        const auto parsedJson = json::parse(inputFileStream, nullptr, false);
        if (parsedJson.is_discarded()) {
            return {concatinateStrings(ERROR_REASON_JSON_DATA_INVALID, TEST_NAME_COMPONENT_DELIMITER_SYMBOL, {extractFilenameWithoutFileExtension(inputFileName)})};
        }
        if (!parsedJson.is_object()) {
            return {concatinateStrings(ERROR_REASON_JSON_DATA_NOT_OBJECT, TEST_NAME_COMPONENT_DELIMITER_SYMBOL, {extractFilenameWithoutFileExtension(inputFileName)})};
        }

        std::vector<std::string> foundTestCases;
        std::size_t              testCaseIndexInFile = 0;
        foundTestCases.reserve(std::distance(parsedJson.items().begin(), parsedJson.items().end()));

        for (const auto& topLevelJsonObjectKeysIteratable: parsedJson.items()) {
            const std::string& testCaseName = topLevelJsonObjectKeysIteratable.key();
            if (testCaseName.empty())
                continue;

            if (doesStringContainOnlyAsciiCharactersOrUnderscores(testCaseName))
                foundTestCases.emplace_back(testCaseName);
            else 
                foundTestCases.emplace_back(concatinateStrings(ERROR_REASON_JSON_DATA_INVALID, TEST_NAME_COMPONENT_DELIMITER_SYMBOL, {extractFilenameWithoutFileExtension(inputFileName), std::string(GTEST_IGNORE_PREFIX), "Test", std::to_string(testCaseIndexInFile), "ContainedNonConformingCharactersInName", testCaseNamePrefix}));

            ++testCaseIndexInFile;
        }
        return foundTestCases;
    }

    inline std::vector<TestFromJsonConfig> loadTestCaseNamesFromFiles(const std::vector<FilenameAndTestNamePrefix>& inputFileNamesData) {
        std::vector<TestFromJsonConfig> loadedTestCases;
        for (const auto& inputFileNameData : inputFileNamesData) {
            if (inputFileNameData.filename.empty())
                continue;

            const std::vector<std::string> loadedTestCaseNamesFromFile = !inputFileNameData.filename.empty()
                ? loadTestCaseNamesFromFile(inputFileNameData.relativePath, inputFileNameData.filename, inputFileNameData.namePrefixForTestsLoadedFromFile)
                : std::vector<std::string>();

            if (loadedTestCaseNamesFromFile.empty())
                continue;

            if (loadedTestCaseNamesFromFile.size() == 1 && loadedTestCaseNamesFromFile.front().find_first_of(GTEST_IGNORE_PREFIX) == 0) {
                loadedTestCases.emplace_back(loadedTestCaseNamesFromFile.front(),"", replaceInvalidCharactersInTestCaseName(loadedTestCaseNamesFromFile.front()));
                continue;
            }

            std::transform(
                    loadedTestCaseNamesFromFile.cbegin(),
                    loadedTestCaseNamesFromFile.cend(),
                    std::back_inserter(loadedTestCases),
                    [&inputFileNameData](const std::string& keyOfTestCaseInJsonFile) {
                        return TestFromJsonConfig(
                            inputFileNameData.relativePath + "/" + inputFileNameData.filename,
                            keyOfTestCaseInJsonFile,
                                replaceInvalidCharactersInTestCaseName(!inputFileNameData.namePrefixForTestsLoadedFromFile.empty() 
                                    ? concatinateStrings(inputFileNameData.namePrefixForTestsLoadedFromFile, TEST_NAME_COMPONENT_DELIMITER_SYMBOL, {keyOfTestCaseInJsonFile})
                                    : keyOfTestCaseInJsonFile));
                    });
        }
        return loadedTestCases;
    }
}
#endif