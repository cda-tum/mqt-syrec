/**
* @file parser.hpp
*
* @brief SyReC parser routines
*/
#ifndef PARSER_HPP
#define PARSER_HPP

#include "core/syrec/expression.hpp"
#include "core/syrec/grammar.hpp"
#include "core/syrec/module.hpp"
#include "core/syrec/number.hpp"
#include "core/syrec/program.hpp"
#include "core/syrec/statement.hpp"
#include "core/syrec/variable.hpp"

#include <string>

namespace syrec {

    struct parser_context {
        explicit parser_context(const read_program_settings& settings):
            current_line_number(0u),
            settings(settings) {
        }

        ast_iterator                 begin;
        unsigned                     current_line_number;
        const read_program_settings& settings;
        std::string                  error_message;
        std::vector<std::string>     loop_variables;
    };

    bool parse_module(module& proc, const ast_module& ast_proc, const program& prog, parser_context& context);

} // namespace syrec

#endif /* PARSER_HPP */
