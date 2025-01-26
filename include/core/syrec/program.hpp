#ifndef CORE_SYREC_PROGRAM_HPP
#define CORE_SYREC_PROGRAM_HPP
#pragma once

#include "core/syrec/module.hpp"
#include <vector>

namespace syrec {

    struct ReadProgramSettings {
        explicit ReadProgramSettings(unsigned bitwidth = 32U):
            defaultBitwidth(bitwidth) {}
        unsigned defaultBitwidth;
    };

    class Program {
    public:
        Program() = default;

        void addModule(const Module::ptr& module) {
            modulesVec.emplace_back(module);
        }

        [[nodiscard]] const Module::vec& modules() const {
            return modulesVec;
        }

        [[nodiscard]] Module::ptr findModule(const std::string& name) const {
            for (const Module::ptr& p: modulesVec) {
                if (p->name == name) {
                    return p;
                }
            }

            return {};
        }

        std::string read(const std::string& filename, ReadProgramSettings settings = ReadProgramSettings{});
        std::string readFromString(const std::string_view& stringifiedProgram, ReadProgramSettings = ReadProgramSettings{});

    private:
        Module::vec modulesVec;

        /**
        * @brief Parser for a SyReC program
        *
        * This function call performs both the lexical parsing
        * as well as the semantic analysis of the program which
        * creates the corresponding C++ constructs for the
        * program.
        *
        * @param filename File-name to parse from
        * @param settings Settings
        * @param error Error Message, in case the function returns false
        *
        * @return true if parsing was successful, otherwise false
        */
        bool readFile(const std::string& filename, ReadProgramSettings settings, std::string* error = nullptr);
        bool readProgramFromString(const std::string_view& content, const ReadProgramSettings& settings, std::string* error = nullptr);
        [[nodiscard]] static std::optional<std::string> tryReadFileContent(std::string_view filename, std::string* foundFileHandlingErrors);
    };

} // namespace syrec
#endif
