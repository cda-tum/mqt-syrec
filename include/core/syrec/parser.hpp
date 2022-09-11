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

    struct ParserContext {
        explicit ParserContext(const ReadProgramSettings& settings):
            settings(settings) {
        }

        ast_iterator               begin;
        unsigned                   currentLineNumber{0U};
        const ReadProgramSettings& settings;
        std::string                errorMessage;
        std::vector<std::string>   loopVariables;
    };

    bool parseModule(Module& proc, const ast_module& astProc, const program& prog, ParserContext& context);

    Number::ptr parseNumber(const ast_number& astNum, const Module& proc, ParserContext& context);

    expression::ptr parseExpression(const ast_expression& astExp, const Module& proc, unsigned bitwidth, ParserContext& context);

    Statement::ptr parseStatement(const ast_statement& astStat, const program& prog, const Module& proc, ParserContext& context);

    unsigned parseVariableType(const std::string& name);

    VariableAccess::ptr parseVariableAccess(const ast_variable& astVar, const Module& proc, ParserContext& context);
} // namespace syrec
