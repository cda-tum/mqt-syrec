#include "algorithms/synthesis/syrec_synthesis_no_additional_lines.hpp"

namespace syrec {
    /// checking the entire statement
    bool SyrecSynthesisNoAdditionalLines::full_statement(const statement::ptr& statement) {
        bool okay;
        if (auto const* stat = dynamic_cast<assign_statement*>(statement.get())) {
            okay = full_statement(*stat);
        } else {
            return false;
        }

        return okay;
    }

    bool SyrecSynthesisNoAdditionalLines::full_statement(const assign_statement& statement) {
        std::vector<unsigned> d;
        std::vector<unsigned> dd;
        std::vector<unsigned> stat_lhs;
        std::vector<unsigned> comp;
        std::vector<unsigned> ddd;
        std::vector<unsigned> lines;
        get_variables(statement.lhs, stat_lhs);

        op_rhs_lhs_expression(statement.rhs, d);

        if (op_vec.empty()) {
            return false;
        }
        flow(statement.rhs, ddd);

        /// Only when the rhs input signals are repeated (since the results are stored in the rhs)

        if (check_repeats()) {
            flow(statement.rhs, dd);

            if (exp_op_vector.size() == 1) {
                if (exp_op_vector.at(0) == 1 || exp_op_vector.at(0) == 2) {
                    /// cancel out the signals

                    exp_op_vector.clear();
                    assign_op_vector.clear();
                    exp_lhs_vector.clear();
                    exp_rhs_vector.clear();
                    op_vec.clear();
                } else {
                    if (statement.op == 1) {
                        expression_single_op(1, exp_lhs_vector.at(0), stat_lhs);
                        expression_single_op(1, exp_rhs_vector.at(0), stat_lhs);
                        exp_op_vector.clear();
                        assign_op_vector.clear();
                        exp_lhs_vector.clear();
                        exp_rhs_vector.clear();
                        op_vec.clear();
                    } else {
                        expression_single_op(statement.op, exp_lhs_vector.at(0), stat_lhs);
                        expression_single_op(exp_op_vector.at(0), exp_rhs_vector.at(0), stat_lhs);
                        exp_op_vector.clear();
                        assign_op_vector.clear();
                        exp_lhs_vector.clear();
                        exp_rhs_vector.clear();
                        op_vec.clear();
                    }
                }

            } else {
                if (exp_lhs_vector.at(0) == exp_rhs_vector.at(0)) {
                    if (exp_op_vector.at(0) == 1 || exp_op_vector.at(0) == 2) {
                        /// cancel out the signals
                    } else if (exp_op_vector.at(0) != 1 || exp_op_vector.at(0) != 2) {
                        expression_single_op(statement.op, exp_lhs_vector.at(0), stat_lhs);
                        expression_single_op(exp_op_vector.at(0), exp_rhs_vector.at(0), stat_lhs);
                    }
                } else {
                    solver(stat_lhs, statement.op, exp_lhs_vector.at(0), exp_op_vector.at(0), exp_rhs_vector.at(0));
                }

                unsigned              j = 0;
                unsigned              z;
                std::vector<unsigned> stat_assign_op;
                if ((exp_op_vector.size() % 2) == 0) {
                    z = (static_cast<int>(exp_op_vector.size()) / 2);
                } else {
                    z = (static_cast<int>((exp_op_vector.size()) - 1) / 2);
                }

                for (unsigned k = 0; k <= z - 1; k++) {
                    stat_assign_op.push_back(assign_op_vector.at(k));
                }

                /// Assignment operations

                std::reverse(stat_assign_op.begin(), stat_assign_op.end());

                /// If reversible assignment is "-", the assignment operations must negated appropriately

                if (statement.op == 1) {
                    for (unsigned int& i: stat_assign_op) {
                        if (i == 0) {
                            i = 1;
                        } else if (i == 1) {
                            i = 0;
                        } else {
                            continue;
                        }
                    }
                }

                for (unsigned i = 1; i <= exp_op_vector.size() - 1; i++) {
                    /// when both rhs and lhs exist
                    if ((exp_lhs_vector.at(i) != comp) && (exp_rhs_vector.at(i) != comp)) {
                        if (exp_lhs_vector.at(i) == exp_rhs_vector.at(i)) {
                            if (exp_op_vector.at(i) == 1 || exp_op_vector.at(i) == 2) {
                                /// cancel out the signals
                                j = j + 1;
                            } else if (exp_op_vector.at(i) != 1 || exp_op_vector.at(i) != 2) {
                                if (stat_assign_op.at(j) == 1) {
                                    expression_single_op(1, exp_lhs_vector.at(i), stat_lhs);
                                    expression_single_op(1, exp_rhs_vector.at(i), stat_lhs);
                                    j = j + 1;
                                } else {
                                    expression_single_op(stat_assign_op.at(j), exp_lhs_vector.at(i), stat_lhs);
                                    expression_single_op(exp_op_vector.at(i), exp_rhs_vector.at(i), stat_lhs);
                                    j = j + 1;
                                }
                            }
                        } else {
                            solver(stat_lhs, stat_assign_op.at(j), exp_lhs_vector.at(i), exp_op_vector.at(i), exp_rhs_vector.at(i));
                            j = j + 1;
                        }
                    }

                    /// when only lhs exists o rhs exists
                    else if (((exp_lhs_vector.at(i) == comp) && (exp_rhs_vector.at(i) != comp)) || ((exp_lhs_vector.at(i) != comp) && (exp_rhs_vector.at(i) == comp))) {
                        exp_evaluate(lines, stat_assign_op.at(j), exp_rhs_vector.at(i), stat_lhs);

                        j = j + 1;
                    }
                }
                exp_op_vector.clear();
                assign_op_vector.clear();
                exp_lhs_vector.clear();
                exp_rhs_vector.clear();
                op_vec.clear();
            }
        } else {
            exp_op_vector.clear();
            assign_op_vector.clear();
            exp_lhs_vector.clear();
            exp_rhs_vector.clear();
            op_vec.clear();
            return false;
        }

        exp_op_vector.clear();
        assign_op_vector.clear();
        exp_lhs_vector.clear();
        exp_rhs_vector.clear();
        op_vec.clear();
        return true;
    }

    bool SyrecSynthesisNoAdditionalLines::flow(const expression::ptr& expression, std::vector<unsigned>& v) {
        if (auto const* binary = dynamic_cast<binary_expression*>(expression.get())) {
            return flow(*binary, v);
        } else if (auto const* var = dynamic_cast<variable_expression*>(expression.get())) {
            return flow(*var, v);
        } else {
            return false;
        }
    }

    bool SyrecSynthesisNoAdditionalLines::flow(const variable_expression& expression, std::vector<unsigned>& v) {
        get_variables(expression.var, v);
        return true;
    }

    /// generating LHS and RHS (can be whole expressions as well)
    bool SyrecSynthesisNoAdditionalLines::flow(const binary_expression& expression, const std::vector<unsigned>& v [[maybe_unused]]) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;
        std::vector<unsigned> comp;
        assign_op_vector.push_back(expression.op);

        if (!flow(expression.lhs, lhs) || !flow(expression.rhs, rhs)) {
            return false;
        }

        exp_lhs_vector.push_back(lhs);
        exp_rhs_vector.push_back(rhs);
        exp_op_vector.push_back(expression.op);
        return true;
    }

    bool SyrecSynthesisNoAdditionalLines::solver(const std::vector<unsigned>& stat_lhs, unsigned stat_op, const std::vector<unsigned>& exp_lhs, unsigned exp_op, const std::vector<unsigned>& exp_rhs) {
        std::vector<unsigned> lines;
        if (stat_op == exp_op) {
            if (exp_op == 1) {
                expression_single_op(1, exp_lhs, stat_lhs);
                expression_single_op(0, exp_rhs, stat_lhs);
            } else {
                expression_single_op(stat_op, exp_lhs, stat_lhs);
                expression_single_op(stat_op, exp_rhs, stat_lhs);
            }
        } else {
            sub_flag = true;
            exp_evaluate(lines, exp_op, exp_lhs, exp_rhs);
            sub_flag = false;
            exp_evaluate(lines, stat_op, lines, stat_lhs);
            sub_flag = true;
            if (exp_op < 3) {
                expression_op_inverse(exp_op, exp_lhs, exp_rhs);
            }
        }
        sub_flag = false;
        return true;
    }

    bool SyrecSynthesisNoAdditionalLines::op_rhs_lhs_expression(const expression::ptr& expression, std::vector<unsigned>& v) {
        if (auto const* binary = dynamic_cast<binary_expression*>(expression.get())) {
            return op_rhs_lhs_expression(*binary, v);
        } else if (auto const* var = dynamic_cast<variable_expression*>(expression.get())) {
            return op_rhs_lhs_expression(*var, v);
        } else {
            return false;
        }
    }

    bool SyrecSynthesisNoAdditionalLines::op_rhs_lhs_expression(const variable_expression& expression, std::vector<unsigned>& v) {
        get_variables(expression.var, v);
        return true;
    }

    bool SyrecSynthesisNoAdditionalLines::op_rhs_lhs_expression(const binary_expression& expression, std::vector<unsigned>& v) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;

        if (!op_rhs_lhs_expression(expression.lhs, lhs) || !op_rhs_lhs_expression(expression.rhs, rhs)) {
            return false;
        }

        v = rhs;
        op_vec.push_back(expression.op);
        return true;
    }

    void SyrecSynthesisNoAdditionalLines::pop_exp() {
        SyrecSynthesis::exp_opp.pop();
        SyrecSynthesis::exp_lhss.pop();
        SyrecSynthesis::exp_rhss.pop();
    }

    void SyrecSynthesisNoAdditionalLines::inverse() {
        expression_op_inverse(SyrecSynthesis::exp_opp.top(), SyrecSynthesis::exp_lhss.top(), SyrecSynthesis::exp_rhss.top());
        SyrecSynthesis::sub_flag = false;
        pop_exp();
    }

    void SyrecSynthesisNoAdditionalLines::assign_add(bool& status, std::vector<unsigned>& lhs, std::vector<unsigned>& rhs, const unsigned& op) {
        if (!SyrecSynthesis::exp_opp.empty() && SyrecSynthesis::exp_opp.top() == op) {
            SyrecSynthesis::increase(lhs, SyrecSynthesis::exp_lhss.top()); //status = bitwise_cnot(lhs, exp_lhss.top())
            status = SyrecSynthesis::increase(lhs, SyrecSynthesis::exp_rhss.top());
            pop_exp();
        } else {
            status = SyrecSynthesis::increase(lhs, rhs);
        }
        while (!SyrecSynthesis::exp_opp.empty()) {
            inverse();
        }
    }

    void SyrecSynthesisNoAdditionalLines::assign_subtract(bool& status, std::vector<unsigned>& lhs, std::vector<unsigned>& rhs, const unsigned& op) {
        if (!SyrecSynthesis::exp_opp.empty() && SyrecSynthesis::exp_opp.top() == op) {
            SyrecSynthesis::decrease(lhs, SyrecSynthesis::exp_lhss.top()); //status = bitwise_cnot(lhs, exp_lhss.top())
            status = SyrecSynthesis::increase(lhs, SyrecSynthesis::exp_rhss.top());
            pop_exp();
        } else {
            status = SyrecSynthesis::decrease(lhs, rhs);
        }
        while (!SyrecSynthesis::exp_opp.empty()) {
            inverse();
        }
    }

    void SyrecSynthesisNoAdditionalLines::assign_exor(bool& status, std::vector<unsigned>& lhs, std::vector<unsigned>& rhs, const unsigned& op) {
        if (!SyrecSynthesis::exp_opp.empty() && SyrecSynthesis::exp_opp.top() == op) {
            SyrecSynthesis::bitwise_cnot(lhs, SyrecSynthesis::exp_lhss.top()); //status = bitwise_cnot(lhs, exp_lhss.top())
            status = SyrecSynthesis::bitwise_cnot(lhs, SyrecSynthesis::exp_rhss.top());
            pop_exp();
        } else {
            status = SyrecSynthesis::bitwise_cnot(lhs, rhs);
        }
        while (!SyrecSynthesis::exp_opp.empty()) {
            inverse();
        }
    }
    /// This function is used when input signals (rhs) are equal (just to solve statements individually)
    bool SyrecSynthesisNoAdditionalLines::exp_evaluate(std::vector<unsigned>& lines, unsigned op, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) {
        switch (op) {
            case binary_expression::add: // +
                increase(rhs, lhs);
                lines = rhs;
                break;
            case binary_expression::subtract: // -
                if (sub_flag) {
                    decrease_new_assign(rhs, lhs);
                    lines = rhs;
                } else {
                    decrease(rhs, lhs);
                    lines = rhs;
                }
                break;
            case binary_expression::exor: // ^
                bitwise_cnot(rhs, lhs);   // duplicate lhs
                lines = rhs;
                break;
            default:
                return false;
        }

        return true;
    }

    bool SyrecSynthesisNoAdditionalLines::decrease_new_assign(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs) {
        for (unsigned int lh: lhs) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(lh);
        }

        increase(rhs, lhs);

        for (unsigned int lh: lhs) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(lh);
        }

        for (unsigned i = 0u; i < lhs.size(); ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(rhs.at(i));
        }
        return true;
    }

    bool SyrecSynthesisNoAdditionalLines::expression_single_op(unsigned op, const std::vector<unsigned>& exp_lhs, const std::vector<unsigned>& exp_rhs) {
        switch (op) {
            case binary_expression::add: // +
                increase(exp_rhs, exp_lhs);
                break;
            case binary_expression::subtract: // -
                if (sub_flag) {
                    decrease_new_assign(exp_rhs, exp_lhs);
                } else {
                    decrease(exp_rhs, exp_lhs);
                }
                break;
            case binary_expression::exor: // ^
                bitwise_cnot(exp_rhs, exp_lhs);
                break;
            default:
                return false;
        }
        return true;
    }

    bool SyrecSynthesisNoAdditionalLines::expression_op_inverse(unsigned op, const std::vector<unsigned>& exp_lhs, const std::vector<unsigned>& exp_rhs) {
        switch (op) {
            case binary_expression::add: // +
                decrease(exp_rhs, exp_lhs);
                break;
            case binary_expression::subtract: // -
                decrease_new_assign(exp_rhs, exp_lhs);
                break;
            case binary_expression::exor: // ^
                bitwise_cnot(exp_rhs, exp_lhs);
                break;
            default:
                return false;
        }
        return true;
    }

    bool SyrecSynthesisNoAdditionalLines::synthesize(circuit& circ, const program& program, const properties::ptr& settings, const properties::ptr& statistics) {
        SyrecSynthesisNoAdditionalLines synthesizer(circ);
        return SyrecSynthesis::synthesize(&synthesizer, circ, program, settings, statistics);
    }
} // namespace syrec
