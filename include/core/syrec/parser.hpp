#pragma once

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

    number::ptr parse_number(const ast_number& ast_num, const module& proc, parser_context& context);

    expression::ptr parse_expression(const ast_expression& ast_exp, const module& proc, unsigned bitwidth, parser_context& context);

    statement::ptr parse_statement(const ast_statement& ast_stat, const program& prog, const module& proc, parser_context& context);

    unsigned parse_variable_type(const std::string& name);

    variable_access::ptr parse_variable_access(const ast_variable& ast_var, const module& proc, parser_context& context);
} // namespace syrec
