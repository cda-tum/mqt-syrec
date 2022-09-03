#pragma once

#include "algorithms/synthesis/syrec_synthesis.hpp"

namespace syrec {

    class SyrecSynthesisNoAdditionalLines: public SyrecSynthesis {
    public:
        using SyrecSynthesis::SyrecSynthesis;

        bool on_module(const module::ptr&);

        bool full_statement(const statement::ptr& statement);
        bool full_statement(const assign_statement& statement);

        bool onStatementNoAdditionalLines(const statement::ptr& statement);
        bool onStatementNoAdditionalLines(const assign_statement& statement);
        bool onStatementNoAdditionalLines(const if_statement& statement);
        bool onStatementNoAdditionalLines(const for_statement& statement);
        bool onStatementNoAdditionalLines(const call_statement& statement);
        bool onStatementNoAdditionalLines(const uncall_statement& statement);

        bool solver(const std::vector<unsigned>& stat_lhs, unsigned stat_op, const std::vector<unsigned>& exp_lhs, unsigned exp_op, const std::vector<unsigned>& exp_rhs);

        bool op_rhs_lhs_expression(const expression::ptr& expression, std::vector<unsigned>& v);
        bool op_rhs_lhs_expression(const variable_expression& expression, std::vector<unsigned>& v);
        bool op_rhs_lhs_expression(const binary_expression& expression, std::vector<unsigned>& v);

        bool flow(const expression::ptr& expression, std::vector<unsigned>& v);
        bool flow(const variable_expression& expression, std::vector<unsigned>& v);
        bool flow(const binary_expression& expression, const std::vector<unsigned>& v);

        bool onExpressionNoAdditionalLines(const expression::ptr& expression, std::vector<unsigned>& lines, std::vector<unsigned> const& lhs_stat, unsigned op);
        bool onExpressionNoAdditionalLines(const binary_expression& expression, std::vector<unsigned>& lines, std::vector<unsigned> const& lhs_stat, unsigned op);
        bool onExpressionNoAdditionalLines(const shift_expression& expression, std::vector<unsigned>& lines, std::vector<unsigned> const& lhs_stat, unsigned op);

        bool decrease_new_assign(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs);
        bool exp_evaluate(std::vector<unsigned>& lines, unsigned op, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs);

        bool expression_op_inverse(unsigned op, const std::vector<unsigned>& exp_lhs, const std::vector<unsigned>& exp_rhs);
        bool expression_single_op(unsigned op, const std::vector<unsigned>& exp_lhs, const std::vector<unsigned>& exp_rhs);

        static bool synthesize(circuit& circ, const program& program, const properties::ptr& settings = std::make_shared<properties>(), const properties::ptr& statistics = std::make_shared<properties>());
    };

    class SyrecSynthesisAdditionalLines: public SyrecSynthesis {
    public:
        using SyrecSynthesis::SyrecSynthesis;

        bool on_module(const module::ptr&);

        bool onStatementAdditionalLines(const statement::ptr& statement);
        bool onStatementAdditionalLines(const assign_statement& statement);
        bool onStatementAdditionalLines(const if_statement& statement);
        bool onStatementAdditionalLines(const for_statement& statement);
        bool onStatementAdditionalLines(const call_statement& statement);
        bool onStatementAdditionalLines(const uncall_statement& statement);

        bool onExpressionAdditionalLines(const expression::ptr& expression, std::vector<unsigned>& lines);
        bool onExpressionAdditionalLines(const binary_expression& expression, std::vector<unsigned>& lines);
        bool onExpressionAdditionalLines(const shift_expression& expression, std::vector<unsigned>& lines);

        static bool synthesize(circuit& circ, const program& program, const properties::ptr& settings = std::make_shared<properties>(), const properties::ptr& statistics = std::make_shared<properties>());
    };

} // namespace syrec