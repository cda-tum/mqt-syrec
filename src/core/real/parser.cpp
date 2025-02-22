#include "core/real/parser.hpp"

#include "Definitions.hpp"
#include "ir/QuantumComputation.hpp"
#include "ir/Register.hpp"
#include "ir/operations/Control.hpp"
#include "ir/operations/OpType.hpp"
#include "ir/operations/StandardOperation.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <istream>
#include <limits>
#include <map>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace syrec {
    namespace {
        std::optional<qc::Qubit> getQubitForVariableIdentFromAnyLookup(
                const std::string& variableIdent, const qc::QuantumRegisterMap& dataQubits,
                const qc::QuantumRegisterMap& ancillaryQubits) {
            if (const auto& matchingEntryInDataQubits = dataQubits.find(variableIdent);
                matchingEntryInDataQubits != dataQubits.end()) {
                return matchingEntryInDataQubits->second.getStartIndex();
            }

            if (const auto& matchingEntryInAncillaryQubits =
                        ancillaryQubits.find(variableIdent);
                matchingEntryInAncillaryQubits != ancillaryQubits.end()) {
                return matchingEntryInAncillaryQubits->second.getStartIndex();
            }

            return std::nullopt;
        }

        /// Determine whether the given io name value, which is not enclosed in quotes,
        /// consists of only letters, digits, and underscore characters.
        /// @param ioName The name to validate
        /// @return Whether the given io name is valid
        bool isValidIoName(const std::string_view& ioName) noexcept {
            return !ioName.empty() &&
                   std::all_of(
                           ioName.cbegin(), ioName.cend(), [](const char ioNameCharacter) {
                               return static_cast<bool>(std::isalnum(
                                              static_cast<unsigned char>(ioNameCharacter))) ||
                                      ioNameCharacter == '_';
                           });
        }

        std::vector<std::string>
        parseVariableNames(const int                              processedLineNumberInRealFile,
                           const std::size_t                      expectedNumberOfVariables,
                           const std::string&                     readInRawVariableIdentValues,
                           const std::unordered_set<std::string>& variableIdentsLookup,
                           const std::string_view&                trimableVariableIdentPrefix) {
            std::vector<std::string> variableNames;
            variableNames.reserve(expectedNumberOfVariables);

            std::unordered_set<std::string> processedVariableIdents;
            std::size_t                     variableIdentStartIdx = 0;
            std::size_t                     variableIdentEndIdx   = 0;

            while (variableIdentStartIdx < readInRawVariableIdentValues.size() &&
                   variableNames.size() < expectedNumberOfVariables) {
                variableIdentEndIdx =
                        readInRawVariableIdentValues.find_first_of(' ', variableIdentStartIdx);

                if (variableIdentEndIdx == std::string::npos) {
                    variableIdentEndIdx = readInRawVariableIdentValues.size();
                }

                std::size_t variableIdentLength =
                        variableIdentEndIdx - variableIdentStartIdx;

                // Remove carriage return character if present
                if (variableIdentLength > 0 &&
                    readInRawVariableIdentValues.at(variableIdentEndIdx - 1) == '\r') {
                    --variableIdentLength;
                }

                auto variableIdent = readInRawVariableIdentValues.substr(
                        variableIdentStartIdx, variableIdentLength);
                const bool trimVariableIdent =
                        variableIdent.find_first_of(trimableVariableIdentPrefix) == 0;
                if (trimVariableIdent) {
                    variableIdent =
                            variableIdent.replace(0, trimableVariableIdentPrefix.size(), "");
                }

                if (!isValidIoName(variableIdent)) {
                    throw std::runtime_error(
                            "[real parser] l: " + std::to_string(processedLineNumberInRealFile) +
                            " msg: invalid variable name: " + variableIdent);
                }

                if (processedVariableIdents.count(variableIdent) > 0) {
                    throw std::runtime_error(
                            "[real parser] l: " + std::to_string(processedLineNumberInRealFile) +
                            " msg: duplicate variable name: " + variableIdent);
                }

                if (!variableIdentsLookup.empty() &&
                    variableIdentsLookup.count(variableIdent) == 0) {
                    throw std::runtime_error(
                            "[real parser] l: " + std::to_string(processedLineNumberInRealFile) +
                            " msg: given variable name " + variableIdent +
                            " was not declared in .variables entry");
                }
                processedVariableIdents.emplace(variableIdent);
                variableNames.emplace_back(trimVariableIdent ? std::string(trimableVariableIdentPrefix) +
                                                                       variableIdent :
                                                               variableIdent);
                variableIdentStartIdx = variableIdentEndIdx + 1;
            }

            if (variableIdentEndIdx < readInRawVariableIdentValues.size() &&
                readInRawVariableIdentValues.at(variableIdentEndIdx) == ' ') {
                throw std::runtime_error(
                        "[real parser] l: " + std::to_string(processedLineNumberInRealFile) +
                        " msg: expected only " + std::to_string(expectedNumberOfVariables) +
                        " variable identifiers to be declared but variable identifier "
                        "delimiter was found"
                        " after " +
                        std::to_string(expectedNumberOfVariables) +
                        " identifiers were detected (which we assume will be followed by "
                        "another io identifier)!");
            }

            if (variableNames.size() < expectedNumberOfVariables) {
                throw std::runtime_error(
                        "[real parser] l:" + std::to_string(processedLineNumberInRealFile) +
                        " msg: Expected " + std::to_string(expectedNumberOfVariables) +
                        " variable idents but only " + std::to_string(variableNames.size()) +
                        " were declared!");
            }
            return variableNames;
        }

        std::unordered_map<std::string, qc::Qubit>
        parseIoNames(const int                              lineInRealFileDefiningIoNames,
                     const std::size_t                      expectedNumberOfIos,
                     const std::string&                     ioNameIdentsRawValues,
                     const std::unordered_set<std::string>& variableIdentLookup) {
            std::unordered_map<std::string, qc::Qubit> foundIoNames;
            std::size_t                                ioNameStartIdx = 0;
            std::size_t                                ioNameEndIdx   = 0;
            std::size_t                                ioIdx          = 0;

            bool searchingForWhitespaceCharacter = false;
            while (ioNameStartIdx < ioNameIdentsRawValues.size() &&
                   foundIoNames.size() <= expectedNumberOfIos) {
                searchingForWhitespaceCharacter =
                        ioNameIdentsRawValues.at(ioNameStartIdx) != '"';
                if (searchingForWhitespaceCharacter) {
                    ioNameEndIdx = ioNameIdentsRawValues.find_first_of(' ', ioNameStartIdx);
                } else {
                    ioNameEndIdx =
                            ioNameIdentsRawValues.find_first_of('"', ioNameStartIdx + 1);
                }

                if (ioNameEndIdx == std::string::npos) {
                    ioNameEndIdx = ioNameIdentsRawValues.size();
                    if (!searchingForWhitespaceCharacter) {
                        throw std::runtime_error(
                                "[real parser] l: " +
                                std::to_string(lineInRealFileDefiningIoNames) +
                                " no matching closing quote found for name of io: " +
                                std::to_string(ioIdx));
                    }
                } else {
                    ioNameEndIdx +=
                            static_cast<std::size_t>(!searchingForWhitespaceCharacter);
                }

                std::size_t ioNameLength = ioNameEndIdx - ioNameStartIdx;
                // Remove carriage return character if present
                if (ioNameLength > 0 &&
                    ioNameIdentsRawValues.at(ioNameEndIdx - 1) == '\r') {
                    --ioNameLength;
                }

                const auto& ioName =
                        ioNameIdentsRawValues.substr(ioNameStartIdx, ioNameLength);

                std::string_view ioNameToValidate = ioName;
                if (!searchingForWhitespaceCharacter) {
                    ioNameToValidate =
                            ioNameToValidate.substr(1, ioNameToValidate.size() - 2);
                }

                if (!isValidIoName(ioNameToValidate)) {
                    throw std::runtime_error(
                            "[real parser] l: " + std::to_string(lineInRealFileDefiningIoNames) +
                            " msg: invalid io name: " + ioName);
                }

                if (variableIdentLookup.count(ioName) > 0) {
                    throw std::runtime_error(
                            "[real parser] l: " + std::to_string(lineInRealFileDefiningIoNames) +
                            " msg: IO ident matched already declared variable with name " +
                            ioName);
                }

                ioNameStartIdx = ioNameEndIdx + 1;
                if (const auto& ioNameInsertionIntoLookupResult =
                            foundIoNames.emplace(ioName, static_cast<qc::Qubit>(ioIdx++));
                    !ioNameInsertionIntoLookupResult.second) {
                    throw std::runtime_error(
                            "[real parser] l:" + std::to_string(lineInRealFileDefiningIoNames) +
                            " msg: duplicate io name: " + ioName);
                }
            }

            if (searchingForWhitespaceCharacter &&
                ioNameEndIdx + 1 < ioNameIdentsRawValues.size() &&
                ioNameIdentsRawValues.at(ioNameEndIdx + 1) == ' ') {
                throw std::runtime_error(
                        "[real parser] l:" + std::to_string(lineInRealFileDefiningIoNames) +
                        " msg: expected only " + std::to_string(expectedNumberOfIos) +
                        " io identifiers to be declared but io identifier delimiter was found"
                        " after " +
                        std::to_string(expectedNumberOfIos) +
                        " identifiers were detected (which we assume will be followed by "
                        "another io identifier)!");
            }
            return foundIoNames;
        }

        void assertRequiredHeaderComponentsAreDefined(
                const int                               processedLine,
                std::initializer_list<std::string_view> requiredHeaderComponentPrefixes,
                const std::set<std::string, std::less<>>&
                        currentUserDeclaredHeaderComponents) {
            for (const auto& requiredHeaderComponentPrefix:
                 requiredHeaderComponentPrefixes) {
                if (currentUserDeclaredHeaderComponents.count(
                            requiredHeaderComponentPrefix) == 0) {
                    throw std::runtime_error(
                            "[real parser] l:" + std::to_string(processedLine) +
                            " msg: Expected " + std::string(requiredHeaderComponentPrefix) +
                            " to have been already defined");
                }
            }
        }

        void trimCommentAndTrailingWhitespaceData(std::string& lineToProcess) {
            if (const auto commentLinePrefixPosition = lineToProcess.find_first_of('#');
                commentLinePrefixPosition != std::string::npos) {
                if (commentLinePrefixPosition != 0) {
                    lineToProcess = lineToProcess.substr(0, commentLinePrefixPosition);
                } else {
                    lineToProcess = "";
                }
            }

            if (lineToProcess.empty()) {
                return;
            }

            if (const std::size_t positionOfLastDataCharacter =
                        lineToProcess.find_last_not_of(" \t");
                positionOfLastDataCharacter != std::string::npos &&
                positionOfLastDataCharacter != lineToProcess.size() - 1) {
                lineToProcess = lineToProcess.substr(0, positionOfLastDataCharacter + 1);
            }
        }
    } // namespace

    using namespace qc;

    auto RealParser::imports(const std::string& realString) -> QuantumComputation {
        std::istringstream iss(realString);
        return import(iss);
    }

    auto RealParser::importf(const std::string& filename) -> QuantumComputation {
        std::ifstream ifs(filename);
        if (!ifs.good()) {
            throw std::runtime_error("[real parser] msg: Failed to open file " + filename);
        }
        return import(ifs);
    }

    auto RealParser::import(std::istream& is) -> QuantumComputation {
        QuantumComputation qc;
        RealParser         parser(qc);
        const auto         line = parser.readRealHeader(is);
        parser.readRealGateDescriptions(is, line);
        return qc;
    }

    int RealParser::readRealHeader(std::istream& is) {
        std::string cmd;
        int         line = 0;

        // We could reuse the QuantumRegisterMap type defined in the qc namespace but
        // to avoid potential errors due to any future refactoring of said type, we
        // use an std::unordered_map instead
        std::unordered_map<std::string, Qubit> userDefinedInputIdents;
        std::unordered_map<std::string, Qubit> userDefinedOutputIdents;
        std::unordered_set<std::string>        userDeclaredVariableIdents;
        std::unordered_set<Qubit>              outputQubitsMarkedAsGarbage;

        constexpr std::string_view numVariablesHeaderComponentPrefix = ".NUMVARS";
        constexpr std::string_view variablesHeaderComponentPrefix    = ".VARIABLES";
        constexpr std::string_view outputsHeaderComponentPrefix      = ".OUTPUTS";

        std::set<std::string, std::less<>> definedHeaderComponents;

        while (true) {
            if (!static_cast<bool>(is >> cmd)) {
                throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                         " msg: Invalid file header");
            }
            std::transform(cmd.begin(), cmd.end(), cmd.begin(), [](unsigned char ch) {
                return static_cast<char>(toupper(ch));
            });
            ++line;

            // skip comments
            if (cmd.front() == '#') {
                is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }

            // valid header commands start with '.'
            if (cmd.front() != '.') {
                throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                         " msg: Invalid file header");
            }

            if (definedHeaderComponents.count(cmd) != 0) {
                throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                         " msg: Duplicate definition of header component " +
                                         cmd);
            }

            definedHeaderComponents.emplace(cmd);
            if (cmd == ".BEGIN") {
                // Entries .numvars and .variables must be declared in all .real files
                assertRequiredHeaderComponentsAreDefined(
                        line,
                        {numVariablesHeaderComponentPrefix, variablesHeaderComponentPrefix},
                        definedHeaderComponents);

                // The garbage declarations in the .real file are defined on the outputs
                // while the garbage state of the quantum computation operates on the
                // defined inputs, thus we perform a mapping from the output marked as
                // garbage back to the input using the output permutation.
                auto& garbage = qc->getGarbage();
                for (const auto& outputQubitMarkedAsGarbage:
                     outputQubitsMarkedAsGarbage) {
                    // Since the call setLogicalQubitAsGarbage(...) assumes that the qubit
                    // parameter is an input qubit, we need to manually mark the output
                    // qubit as garbage by using the output qubit instead.
                    for (const auto& [inputQubit, outputQubit]: qc->outputPermutation) {
                        if (outputQubit == outputQubitMarkedAsGarbage) {
                            garbage[inputQubit] = true;
                            break;
                        }
                    }
                    qc->outputPermutation.erase(outputQubitMarkedAsGarbage);
                }

                // header read complete
                return line;
            }

            if (cmd == ".NUMVARS") {
                if (std::size_t nq{}; !static_cast<bool>(is >> nq)) {
                    nqubits = 0;
                } else {
                    nqubits = nq;
                }
                nclassics = nqubits;
            } else if (cmd == ".VARIABLES") {
                is >> std::ws;
                assertRequiredHeaderComponentsAreDefined(
                        line, {numVariablesHeaderComponentPrefix}, definedHeaderComponents);
                userDeclaredVariableIdents.reserve(nclassics);

                std::string variableDefinitionEntry;
                if (!std::getline(is, variableDefinitionEntry)) {
                    throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                             " msg: Failed read in '.variables' line");
                }

                trimCommentAndTrailingWhitespaceData(variableDefinitionEntry);
                const auto& processedVariableIdents =
                        parseVariableNames(line, nclassics, variableDefinitionEntry, {}, "");
                userDeclaredVariableIdents.insert(processedVariableIdents.cbegin(),
                                                  processedVariableIdents.cend());

                for (std::size_t i = 0; i < nclassics; ++i) {
                    const std::string& quantumRegisterIdentifier =
                            processedVariableIdents.at(i);
                    qc->addQubitRegister(1, quantumRegisterIdentifier);

                    const std::string& classicalRegisterIdentifier =
                            "c_" + quantumRegisterIdentifier;
                    qc->addClassicalRegister(1, classicalRegisterIdentifier);
                }
            } else if (cmd == ".INITIAL_LAYOUT") {
                is >> std::ws;
                assertRequiredHeaderComponentsAreDefined(
                        line,
                        {numVariablesHeaderComponentPrefix, variablesHeaderComponentPrefix},
                        definedHeaderComponents);

                std::string initialLayoutDefinitionEntry;
                if (!std::getline(is, initialLayoutDefinitionEntry)) {
                    throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                             " msg: Failed read in '.initial_layout' line");
                }

                trimCommentAndTrailingWhitespaceData(initialLayoutDefinitionEntry);
                const auto& processedVariableIdents =
                        parseVariableNames(line, nclassics, initialLayoutDefinitionEntry,
                                           userDeclaredVariableIdents, "");

                // Map the user declared variable idents in the .variable entry to the
                // ones declared in the .initial_layout as explained in
                // https://mqt.readthedocs.io/projects/core/en/latest/mqt_core_ir.html#layout-information
                const auto& quantumRegisters = qc->getQuantumRegisters();
                for (std::size_t i = 0; i < nclassics; ++i) {
                    const auto algorithmicQubit = static_cast<Qubit>(i);
                    const auto deviceQubitForVariableIdentInInitialLayout =
                            quantumRegisters.at(processedVariableIdents.at(i)).getStartIndex();
                    qc->initialLayout[deviceQubitForVariableIdentInInitialLayout] =
                            algorithmicQubit;
                }
            } else if (cmd == ".CONSTANTS") {
                is >> std::ws;
                assertRequiredHeaderComponentsAreDefined(
                        line, {numVariablesHeaderComponentPrefix}, definedHeaderComponents);

                std::string constantsValuePerIoDefinition;
                if (!std::getline(is, constantsValuePerIoDefinition)) {
                    throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                             " msg: Failed read in '.constants' line");
                }

                trimCommentAndTrailingWhitespaceData(constantsValuePerIoDefinition);
                if (constantsValuePerIoDefinition.size() != nclassics) {
                    throw std::runtime_error(
                            "[real parser] l:" + std::to_string(line) + " msg: Expected " +
                            std::to_string(nclassics) + " constant values but " +
                            std::to_string(constantsValuePerIoDefinition.size()) +
                            " were declared!");
                }

                std::size_t constantValueIdx = 0;
                for (const auto constantValuePerIo: constantsValuePerIoDefinition) {
                    if (const bool isCurrentQubitMarkedAsAncillary =
                                constantValuePerIo == '0' || constantValuePerIo == '1';
                        isCurrentQubitMarkedAsAncillary) {
                        const auto& ancillaryQubit = static_cast<Qubit>(constantValueIdx);
                        // Since ancillary qubits are assumed to have an initial value of
                        // zero, we need to add an inversion gate to derive the correct
                        // initial value of 1.
                        if (constantValuePerIo == '1') {
                            qc->x(ancillaryQubit);
                        }

                        qc->setLogicalQubitAncillary(ancillaryQubit);
                    } else if (constantValuePerIo != '-') {
                        throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                                 " msg: Invalid value in '.constants' header: '" +
                                                 std::to_string(constantValuePerIo) + "'");
                    }
                    ++constantValueIdx;
                }
            } else if (cmd == ".GARBAGE") {
                is >> std::ws;
                assertRequiredHeaderComponentsAreDefined(
                        line, {numVariablesHeaderComponentPrefix}, definedHeaderComponents);

                std::string garbageStatePerIoDefinition;
                if (!std::getline(is, garbageStatePerIoDefinition)) {
                    throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                             " msg: Failed read in '.garbage' line");
                }

                trimCommentAndTrailingWhitespaceData(garbageStatePerIoDefinition);
                if (garbageStatePerIoDefinition.size() != nclassics) {
                    throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                             " msg: Expected " + std::to_string(nclassics) +
                                             " garbage state values but " +
                                             std::to_string(garbageStatePerIoDefinition.size()) +
                                             " were declared!");
                }

                std::size_t garbageStateIdx = 0;
                for (const auto garbageStateValue: garbageStatePerIoDefinition) {
                    if (const bool isCurrentQubitMarkedAsGarbage = garbageStateValue == '1';
                        isCurrentQubitMarkedAsGarbage) {
                        outputQubitsMarkedAsGarbage.emplace(
                                static_cast<Qubit>(garbageStateIdx));
                    } else if (garbageStateValue != '-') {
                        throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                                 " msg: Invalid value in '.garbage' header: '" +
                                                 std::to_string(garbageStateValue) + "'");
                    }
                    garbageStateIdx++;
                }
            } else if (cmd == ".INPUTS") {
                // .INPUT: specifies initial layout
                is >> std::ws;
                assertRequiredHeaderComponentsAreDefined(
                        line,
                        {numVariablesHeaderComponentPrefix, variablesHeaderComponentPrefix},
                        definedHeaderComponents);

                if (definedHeaderComponents.count(outputsHeaderComponentPrefix) > 0) {
                    throw std::runtime_error(
                            "[real parser] l:" + std::to_string(line) +
                            " msg: .inputs entry must be declared prior to the .outputs entry");
                }

                const std::size_t expectedNumInputIos = nclassics;
                std::string       ioNameIdentsLine;
                if (!std::getline(is, ioNameIdentsLine)) {
                    throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                             " msg: Failed read in '.inputs' line");
                }

                trimCommentAndTrailingWhitespaceData(ioNameIdentsLine);
                userDefinedInputIdents =
                        parseIoNames(line, expectedNumInputIos, ioNameIdentsLine,
                                     userDeclaredVariableIdents);

                if (userDefinedInputIdents.size() != expectedNumInputIos) {
                    throw std::runtime_error(
                            "[real parser] l:" + std::to_string(line) + "msg: Expected " +
                            std::to_string(expectedNumInputIos) + " inputs to be declared!");
                }
            } else if (cmd == ".OUTPUTS") {
                // .OUTPUTS: specifies output permutation
                is >> std::ws;
                assertRequiredHeaderComponentsAreDefined(
                        line,
                        {numVariablesHeaderComponentPrefix, variablesHeaderComponentPrefix},
                        definedHeaderComponents);

                const std::size_t expectedNumOutputIos = nclassics;
                std::string       ioNameIdentsLine;
                if (!std::getline(is, ioNameIdentsLine)) {
                    throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                             " msg: Failed read in '.outputs' line");
                }

                trimCommentAndTrailingWhitespaceData(ioNameIdentsLine);
                userDefinedOutputIdents =
                        parseIoNames(line, expectedNumOutputIos, ioNameIdentsLine,
                                     userDeclaredVariableIdents);

                if (userDefinedOutputIdents.size() != expectedNumOutputIos) {
                    throw std::runtime_error(
                            "[real parser] l:" + std::to_string(line) + "msg: Expected " +
                            std::to_string(expectedNumOutputIos) + " outputs to be declared!");
                }

                if (userDefinedInputIdents.empty()) {
                    continue;
                }

                for (const auto& [outputIoIdent, outputIoQubit]:
                     userDefinedOutputIdents) {
                    // We assume that a permutation of a given input qubit Q at index i
                    // is performed in the circuit if an entry in both in the .output
                    // as well as the .input definition using the same literal is found,
                    // with the input literal being defined at position i in the .input
                    // definition. If no such matching is found, we require that the output
                    // is marked as garbage.
                    //
                    // The outputPermutation map will use be structured as shown in the
                    // documentation
                    // (https://mqt.readthedocs.io/projects/core/en/latest/mqt_core_ir.html#layout-information)
                    // with the output qubit being used as the key while the input qubit
                    // serves as the map entries value.
                    //
                    if (userDefinedInputIdents.count(outputIoIdent) == 0) {
                        // The current implementation requires that the .garbage definition is
                        // define prior to the .output one.
                        if (outputQubitsMarkedAsGarbage.count(outputIoQubit) == 0) {
                            throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                                     " msg: outputs without matching inputs are "
                                                     "expected to be marked as garbage");
                        }
                    } else if (const Qubit matchingInputQubitForOutputLiteral =
                                       userDefinedInputIdents.at(outputIoIdent);
                               matchingInputQubitForOutputLiteral != outputIoQubit &&
                               !qc->logicalQubitIsGarbage(outputIoQubit)) {
                        // We do not need to check whether a mapping from one input to any
                        // output exists, since we require that the idents defined in either
                        // of the .input as well as the .output definition are unique in their
                        // definition.
                        //
                        // Only if the matching entries where defined at different indices
                        // in their respective IO declaration do we update the existing
                        // identity mapping for the given output qubit
                        qc->outputPermutation.insert_or_assign(
                                outputIoQubit, matchingInputQubitForOutputLiteral);

                        // If we have determined a non-identity permutation of an input qubit,
                        // (i.e. output 2 <- input 1) any existing identity permutation
                        // of the input qubit will be removed since the previously mapped to
                        // output (output 1) of the identity permutation must have another
                        // non-identity permutation defined or must be declared as a garbage
                        // output.
                        if (qc->outputPermutation.count(matchingInputQubitForOutputLiteral) > 0 &&
                            qc->outputPermutation[matchingInputQubitForOutputLiteral] ==
                                    matchingInputQubitForOutputLiteral) {
                            qc->outputPermutation.erase(matchingInputQubitForOutputLiteral);
                        }
                    }
                }
            } else if (cmd == ".VERSION" || cmd == ".INPUTBUS" || cmd == ".OUTPUTBUS") {
                is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            } else if (cmd == ".DEFINE") {
                // TODO: Defines currently not supported
                std::cerr << "[WARN] File contains 'define' statement, which is "
                             "currently not supported and thus simply skipped.\n";
                while (cmd != ".ENDDEFINE") {
                    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    is >> cmd;
                    std::transform(cmd.begin(), cmd.end(), cmd.begin(),
                                   [](const unsigned char c) {
                                       return static_cast<char>(toupper(c));
                                   });
                }
            } else {
                throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                         " msg: Unknown command: " + cmd);
            }
        }
    }

    void RealParser::readRealGateDescriptions(std::istream& is,
                                              int           line) const {
        const std::regex gateRegex =
                std::regex("(r[xyz]|i[df]|q|[0a-z](?:[+ip])?)(\\d+)?(?::([-+]?[0-9]+[.]?["
                           "0-9]*(?:[eE][-+]?[0-9]+)?))?");
        std::smatch m;
        std::string cmd;

        static const std::map<std::string, OpType> IDENTIFIER_MAP{
                {"0", I}, {"id", I}, {"h", H}, {"n", X}, {"c", X}, {"x", X}, {"y", Y}, {"z", Z}, {"s", S}, {"si", Sdg}, {"sp", Sdg}, {"s+", Sdg}, {"v", V}, {"vi", Vdg}, {"vp", Vdg}, {"v+", Vdg}, {"rx", RX}, {"ry", RY}, {"rz", RZ}, {"f", SWAP}, {"if", SWAP}, {"p", Peres}, {"pi", Peresdg}, {"p+", Peresdg}, {"q", P}};

        while (!is.eof()) {
            if (!static_cast<bool>(is >> cmd)) {
                throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                         " msg: Failed to read command");
            }
            std::transform(
                    cmd.begin(), cmd.end(), cmd.begin(),
                    [](const unsigned char c) { return static_cast<char>(tolower(c)); });
            ++line;

            if (cmd.front() == '#') {
                is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }

            if (cmd == ".end") {
                break;
            }

            // match gate declaration
            if (!std::regex_match(cmd, m, gateRegex)) {
                throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                         " msg: Unsupported gate detected: " + cmd);
            }

            // extract gate information (identifier, #controls, divisor)
            OpType gate{};
            if (m.str(1) == "t") { // special treatment of t(offoli) for real format
                gate = X;
            } else {
                auto it = IDENTIFIER_MAP.find(m.str(1));
                if (it == IDENTIFIER_MAP.end()) {
                    throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                             " msg: Unknown gate identifier: " + m.str(1));
                }
                gate = (*it).second;
            }
            auto ncontrols =
                    m.str(2).empty() ? 0 : std::stoul(m.str(2), nullptr, 0) - 1;
            const fp lambda = m.str(3).empty() ? static_cast<fp>(0L) : static_cast<fp>(std::stold(m.str(3)));

            if (gate == V || gate == Vdg || m.str(1) == "c") {
                ncontrols = 1;
            } else if (gate == Peres || gate == Peresdg) {
                ncontrols = 2;
            }

            if (ncontrols >= qc->getNqubits()) {
                throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                         " msg: Gate acts on " + std::to_string(ncontrols + 1) +
                                         " qubits, but only " + std::to_string(qc->getNqubits()) +
                                         " qubits are available.");
            }

            std::string qubits;
            if (!getline(is, qubits)) {
                throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                         " msg: Failed read in gate definition");
            }
            trimCommentAndTrailingWhitespaceData(qubits);

            // If we cannot determine how many gate lines are to be expected from the
            // gate definition (i.e. the gate definition 'c a b' does not define the
            // number of gate lines) we assume that the number of whitespaces left of
            // the gate type define the number of gate lines.
            std::size_t numberOfGateLines = 0;
            if (const std::string& stringifiedNumberOfGateLines = m.str(2);
                !stringifiedNumberOfGateLines.empty()) {
                numberOfGateLines = static_cast<std::size_t>(
                        std::stoul(stringifiedNumberOfGateLines, nullptr, 0));
            } else {
                numberOfGateLines = static_cast<std::size_t>(
                        std::count(qubits.cbegin(), qubits.cend(), ' '));
            }

            // Current parser implementation defines number of expected control lines
            // (nControl) as nLines (of gate definition) - 1. Controlled swap gate has
            // at most two target lines so we define the number of control lines as
            // nLines - 2.
            if (gate == SWAP) {
                if (numberOfGateLines < 2) {
                    throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                             "msg: SWAP gate is expected to operate on at least "
                                             "two qubits but only " +
                                             std::to_string(ncontrols) + " were defined");
                }
                ncontrols = static_cast<uint32_t>(numberOfGateLines - 2);
            }

            std::vector<Control>            controls(ncontrols, Qubit());
            const auto&                     gateLines = qubits.empty() ? "" : qubits.substr(1);
            std::unordered_set<std::string> validVariableIdentLookup;

            // Use the entries of the creg register map prefixed with 'c_' to determine
            // the declared variable idents in the .variable entry
            for (const auto& qregNameAndQubitIndexPair: qc->getClassicalRegisters()) {
                validVariableIdentLookup.emplace(
                        qregNameAndQubitIndexPair.first.substr(2));
            }

            // We will ignore the prefix '-' when validating a given gate line ident
            auto processedGateLines = parseVariableNames(
                    line, numberOfGateLines, gateLines, validVariableIdentLookup, "-");

            std::size_t lineIdx = 0;
            // get controls and target
            for (std::size_t i = 0; i < ncontrols; ++i) {
                std::string_view gateIdent       = processedGateLines.at(lineIdx++);
                const bool       negativeControl = gateIdent.front() == '-';
                if (negativeControl) {
                    gateIdent = gateIdent.substr(1);
                }

                // Since variable qubits can either be data or ancillary qubits our search
                // will have to be conducted in both lookups
                if (const std::optional<Qubit> controlLineQubit =
                            getQubitForVariableIdentFromAnyLookup(
                                    std::string(gateIdent), qc->getQuantumRegisters(), qc->getAncillaRegisters());
                    controlLineQubit.has_value()) {
                    controls[i] =
                            Control(*controlLineQubit,
                                    negativeControl ? Control::Type::Neg : Control::Type::Pos);
                } else {
                    throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                             " msg: Matching qubit for control line " +
                                             std::string(gateIdent) + " not found!");
                }
            }

            const auto  numberOfTargetLines = numberOfGateLines - ncontrols;
            std::vector targetLineQubits(numberOfTargetLines, Qubit());
            for (std::size_t i = 0; i < numberOfTargetLines; ++i) {
                const auto& targetLineIdent = processedGateLines.at(lineIdx++);
                // Since variable qubits can either be data or ancillary qubits our search
                // will have to be conducted in both lookups
                if (const std::optional<Qubit> targetLineQubit =
                            getQubitForVariableIdentFromAnyLookup(
                                    targetLineIdent, qc->getQuantumRegisters(), qc->getAncillaRegisters());
                    targetLineQubit.has_value()) {
                    targetLineQubits[i] = *targetLineQubit;
                } else {
                    throw std::runtime_error("[real parser] l:" + std::to_string(line) +
                                             " msg: Matching qubit for target line " +
                                             targetLineIdent + " not found!");
                }
            }

            switch (gate) {
                case I:
                case H:
                case Y:
                case Z:
                case S:
                case Sdg:
                case T:
                case Tdg:
                case V:
                case Vdg:
                    qc->emplace_back<StandardOperation>(
                            Controls{controls.cbegin(), controls.cend()},
                            targetLineQubits.front(), gate);
                    break;
                case X:
                    qc->mcx(Controls{controls.cbegin(), controls.cend()},
                            targetLineQubits.front());
                    break;
                case RX:
                case RY:
                case RZ:
                case P:
                    qc->emplace_back<StandardOperation>(
                            Controls{controls.cbegin(), controls.cend()},
                            targetLineQubits.front(), gate, std::vector{PI / (lambda)});
                    break;
                case SWAP:
                case iSWAP:
                    qc->emplace_back<StandardOperation>(
                            Controls{controls.cbegin(), controls.cend()},
                            Targets{targetLineQubits.cbegin(), targetLineQubits.cend()}, gate);
                    break;
                case Peres:
                case Peresdg: {
                    const auto target1 = controls.back().qubit;
                    controls.pop_back();
                    qc->emplace_back<StandardOperation>(
                            Controls{controls.cbegin(), controls.cend()}, target1,
                            targetLineQubits.front(), gate);
                    break;
                }
                default:
                    std::cerr << "Unsupported operation encountered:  " << gate << "!\n";
                    break;
            }
        }
    }
} // namespace syrec
