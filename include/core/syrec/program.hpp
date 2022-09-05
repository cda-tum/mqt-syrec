#pragma once

#include "core/syrec/expression.hpp"
#include "core/syrec/grammar.hpp"
#include "core/syrec/module.hpp"
#include "core/syrec/number.hpp"
#include "core/syrec/statement.hpp"
#include "core/syrec/variable.hpp"

#include <fstream>
#include <vector>

namespace syrec {

    struct read_program_settings {
        explicit read_program_settings(unsigned bitwidth = 32U):
            default_bitwidth(bitwidth){};
        unsigned default_bitwidth;
    };

    class program {
    public:
        program() = default;

        void add_module(const module::ptr& module) {
            modules_vec.emplace_back(module);
        }

        [[nodiscard]] const module::vec& modules() const {
            return modules_vec;
        }

        [[nodiscard]] module::ptr find_module(const std::string& name) const {
            for (const module::ptr& p: modules_vec) {
                if (p->name == name) {
                    return p;
                }
            }

            return {};
        }

        std::string read(const std::string& filename, read_program_settings settings = read_program_settings{});

    private:
        module::vec modules_vec;

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
        bool read_file(const std::string& filename, read_program_settings settings, std::string* error = nullptr);
        bool read_program_from_string(const std::string& content, const read_program_settings& settings, std::string* error);
    };

} // namespace syrec
