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

        bool op_rhs_lhs_expression(const expression::ptr& expression, std::vector<unsigned>& v) override;

        bool op_rhs_lhs_expression(const variable_expression& expression, std::vector<unsigned>& v) override;

        bool op_rhs_lhs_expression(const binary_expression& expression, std::vector<unsigned>& v) override;

        void pop_exp();

        void inverse();

        void assign_add(bool& status, std::vector<unsigned>& lhs, std::vector<unsigned>& rhs, const unsigned& op) override;

        void assign_subtract(bool& status, std::vector<unsigned>& lhs, std::vector<unsigned>& rhs, const unsigned& op) override;

        void assign_exor(bool& status, std::vector<unsigned>& lhs, std::vector<unsigned>& rhs, const unsigned& op) override;

        bool solver(const std::vector<unsigned>& stat_lhs, unsigned stat_op, const std::vector<unsigned>& exp_lhs, unsigned exp_op, const std::vector<unsigned>& exp_rhs);

        bool flow(const expression::ptr& expression, std::vector<unsigned>& v);
        bool flow(const variable_expression& expression, std::vector<unsigned>& v);
        bool flow(const binary_expression& expression, const std::vector<unsigned>& v);

        void exp_add([[maybe_unused]] const unsigned& bitwidth, std::vector<unsigned>& lines, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) override {
            SyrecSynthesis::increase(rhs, lhs);
            lines = rhs;
        }

        void exp_subtract([[maybe_unused]] const unsigned& bitwidth, std::vector<unsigned>& lines, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) override {
            decrease_new_assign(rhs, lhs);
            lines = rhs;
        }

        void exp_exor([[maybe_unused]] const unsigned& bitwidth, std::vector<unsigned>& lines, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) override {
            bitwise_cnot(rhs, lhs); // duplicate lhs
            lines = rhs;
        }

        bool exp_evaluate(std::vector<unsigned>& lines, unsigned op, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs);

        bool expression_single_op(unsigned op, const std::vector<unsigned>& exp_lhs, const std::vector<unsigned>& exp_rhs);

        bool decrease_new_assign(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs);

        bool expression_op_inverse(unsigned op, const std::vector<unsigned>& exp_lhs, const std::vector<unsigned>& exp_rhs) override;
    };
} // namespace syrec
