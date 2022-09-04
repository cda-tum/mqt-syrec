#pragma once

#include "algorithms/synthesis/syrec_synthesis.hpp"

namespace syrec {
    class SyrecSynthesisNoAdditionalLines: public SyrecSynthesis {
    public:
        using SyrecSynthesis::SyrecSynthesis;

        static bool synthesize(circuit& circ, const program& program, const properties::ptr& settings = std::make_shared<properties>(), const properties::ptr& statistics = std::make_shared<properties>());

    protected:
        bool process_statement(const statement::ptr& statement) override {
            return !full_statement(statement) && !SyrecSynthesis::on_statement(statement);
        }

        bool full_statement(const statement::ptr& statement);
        bool full_statement(const assign_statement& statement);

        bool on_statement(const assign_statement& statement) override;

        bool solver(const std::vector<unsigned>& stat_lhs, unsigned stat_op, const std::vector<unsigned>& exp_lhs, unsigned exp_op, const std::vector<unsigned>& exp_rhs);

        bool op_rhs_lhs_expression(const expression::ptr& expression, std::vector<unsigned>& v);
        bool op_rhs_lhs_expression(const variable_expression& expression, std::vector<unsigned>& v);
        bool op_rhs_lhs_expression(const binary_expression& expression, std::vector<unsigned>& v);

        bool flow(const expression::ptr& expression, std::vector<unsigned>& v);
        bool flow(const variable_expression& expression, std::vector<unsigned>& v);
        bool flow(const binary_expression& expression, const std::vector<unsigned>& v);

        bool on_expression(const binary_expression& expression, std::vector<unsigned>& lines, std::vector<unsigned> const& lhs_stat, unsigned op);

        bool decrease_new_assign(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs);
        bool exp_evaluate(std::vector<unsigned>& lines, unsigned op, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs);

        bool expression_op_inverse(unsigned op, const std::vector<unsigned>& exp_lhs, const std::vector<unsigned>& exp_rhs);
        bool expression_single_op(unsigned op, const std::vector<unsigned>& exp_lhs, const std::vector<unsigned>& exp_rhs);
    };
} // namespace syrec
