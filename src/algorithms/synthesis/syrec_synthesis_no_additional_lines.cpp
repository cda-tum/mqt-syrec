#include "algorithms/synthesis/syrec_synthesis_no_additional_lines.hpp"

namespace syrec {
    /// checking the entire statement
    bool SyrecSynthesisNoAdditionalLines::full_statement(const statement::ptr& statement) {
        bool okay = false;
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

    /// generating LHS and RHS (not whole expressions, just the corresponding variables)
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

    bool SyrecSynthesisNoAdditionalLines::on_statement(const assign_statement& statement) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;
        std::vector<unsigned> d;

        get_variables(statement.lhs, lhs);

        op_rhs_lhs_expression(statement.rhs, d);
        SyrecSynthesis::on_expression(statement.rhs, rhs, lhs, statement.op);
        op_vec.clear();

        bool status = false;

        switch (statement.op) {
            case assign_statement::add: {
                if (!exp_opp.empty() && exp_opp.top() == statement.op) {
                    increase(lhs, exp_lhss.top()); //status = increase(lhs, exp_lhss.top())
                    status = increase(lhs, exp_rhss.top());
                    exp_opp.pop();
                    exp_lhss.pop();
                    exp_rhss.pop();
                } else {
                    status = increase(lhs, rhs);
                }
                while (!exp_opp.empty()) {
                    expression_op_inverse(exp_opp.top(), exp_lhss.top(), exp_rhss.top());
                    sub_flag = false;
                    exp_opp.pop();
                    exp_lhss.pop();
                    exp_rhss.pop();
                }
            } break;

            case assign_statement::subtract: {
                if (!exp_opp.empty() && exp_opp.top() == statement.op) {
                    decrease(lhs, exp_lhss.top()); //status=decrease(lhs, exp_lhss.top())
                    status = increase(lhs, exp_rhss.top());
                    exp_opp.pop();
                    exp_lhss.pop();
                    exp_rhss.pop();
                } else {
                    status = decrease(lhs, rhs);
                }
                while (!exp_opp.empty()) {
                    expression_op_inverse(exp_opp.top(), exp_lhss.top(), exp_rhss.top());
                    sub_flag = false;
                    exp_opp.pop();
                    exp_lhss.pop();
                    exp_rhss.pop();
                }
            } break;

            case assign_statement::exor: {
                if (!exp_opp.empty() && exp_opp.top() == statement.op) {
                    bitwise_cnot(lhs, exp_lhss.top()); //status = bitwise_cnot(lhs, exp_lhss.top())
                    status = bitwise_cnot(lhs, exp_rhss.top());
                    exp_opp.pop();
                    exp_lhss.pop();
                    exp_rhss.pop();
                } else {
                    status = bitwise_cnot(lhs, rhs);
                }
                while (!exp_opp.empty()) {
                    expression_op_inverse(exp_opp.top(), exp_lhss.top(), exp_rhss.top());
                    sub_flag = false;
                    exp_opp.pop();
                    exp_lhss.pop();
                    exp_rhss.pop();
                }
            } break;

            default:
                return false;
        }

        return status;
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

    bool SyrecSynthesisNoAdditionalLines::on_expression(const binary_expression& expression, std::vector<unsigned>& lines, std::vector<unsigned> const& lhs_stat, unsigned op) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;

        if (!SyrecSynthesis::on_expression(expression.lhs, lhs, lhs_stat, op) || !SyrecSynthesis::on_expression(expression.rhs, rhs, lhs_stat, op)) {
            return false;
        }

        exp_lhss.push(lhs);
        exp_rhss.push(rhs);
        exp_opp.push(expression.op);

        if ((exp_opp.size() == op_vec.size()) && (exp_opp.top() == op)) {
            return true;
        }

        switch (expression.op) {
            case binary_expression::add: // +
                increase(rhs, lhs);
                lines = rhs;
                break;
            case binary_expression::subtract: // -
                decrease_new_assign(rhs, lhs);
                lines = rhs;
                break;
            case binary_expression::exor: // ^
                bitwise_cnot(rhs, lhs);   // duplicate lhs
                lines = rhs;
                break;
            case binary_expression::multiply: // *
                get_constant_lines(expression.bitwidth(), 0u, lines);
                multiplication(lines, lhs, rhs);
                break;
            case binary_expression::divide: // /
                get_constant_lines(expression.bitwidth(), 0u, lines);
                division(lines, lhs, rhs);
                break;
            case binary_expression::modulo: {
                get_constant_lines(expression.bitwidth(), 0u, lines);
                std::vector<unsigned> quot;
                get_constant_lines(expression.bitwidth(), 0u, quot);

                bitwise_cnot(lines, lhs); // duplicate lhs
                modulo(quot, lines, rhs);
            } break;
            case binary_expression::logical_and: // &&
                lines.emplace_back(get_constant_line(false));
                conjunction(lines.at(0), lhs.at(0), rhs.at(0));
                break;
            case binary_expression::logical_or: // ||
                lines.emplace_back(get_constant_line(false));
                disjunction(lines.at(0), lhs.at(0), rhs.at(0));
                break;
            case binary_expression::bitwise_and: // &
                get_constant_lines(expression.bitwidth(), 0u, lines);
                bitwise_and(lines, lhs, rhs);
                break;
            case binary_expression::bitwise_or: // |
                get_constant_lines(expression.bitwidth(), 0u, lines);
                bitwise_or(lines, lhs, rhs);
                break;
            case binary_expression::less_than: // <
                lines.emplace_back(get_constant_line(false));
                less_than(lines.at(0), lhs, rhs);
                break;
            case binary_expression::greater_than: // >
                lines.emplace_back(get_constant_line(false));
                greater_than(lines.at(0), lhs, rhs);
                break;
            case binary_expression::equals: // =
                lines.emplace_back(get_constant_line(false));
                equals(lines.at(0), lhs, rhs);
                break;
            case binary_expression::not_equals: // !=
                lines.emplace_back(get_constant_line(false));
                not_equals(lines.at(0), lhs, rhs);
                break;
            case binary_expression::less_equals: // <=
                lines.emplace_back(get_constant_line(false));
                less_equals(lines.at(0), lhs, rhs);
                break;
            case binary_expression::greater_equals: // >=
                lines.emplace_back(get_constant_line(false));
                greater_equals(lines.at(0), lhs, rhs);
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
