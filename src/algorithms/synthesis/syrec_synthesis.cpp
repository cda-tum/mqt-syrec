#include "algorithms/synthesis/derived_syrec_synthesis.hpp"
#include "core/syrec/expression.hpp"
#include "core/syrec/program.hpp"
#include "core/syrec/variable.hpp"
#include "core/utils/timer.hpp"

#include <boost/dynamic_bitset.hpp>
#include <cmath>
#include <functional>
#include <memory>
#include <numeric>

namespace syrec {

    struct annotater {
        explicit annotater(circuit& circ, const std::stack<statement::ptr>& stmts):
            _circ(circ),
            _stmts(stmts) {}

        // Operator needs this signature to work
        void operator()(gate const& g) const {
            if (!_stmts.empty()) {
                _circ.annotate(g, "lno", std::to_string(_stmts.top()->line_number));
            }
        }

    private:
        circuit&                          _circ;
        const std::stack<statement::ptr>& _stmts;
    };

    // Helper Functions for the synthesis methods
    SyrecSynthesis::SyrecSynthesis(circuit& circ):
        _circ(circ) {
        free_const_lines_map.try_emplace(false /* emplacing a default constructed object */);
        free_const_lines_map.try_emplace(true /* emplacing a default constructed object */);

        // root anlegen
        cct_man.current = add_vertex(cct_man.tree);
        cct_man.root    = cct_man.current;
        // Blatt anlegen
        cct_man.current                                             = add_vertex(cct_man.tree);
        get(boost::vertex_name, cct_man.tree)[cct_man.current].circ = std::make_shared<circuit>();
        get(boost::vertex_name, cct_man.tree)[cct_man.current].circ->gate_added.connect(annotater(*get(boost::vertex_name, cct_man.tree)[cct_man.current].circ, _stmts));
        add_edge(cct_man.root, cct_man.current, cct_man.tree);
    }

    void SyrecSynthesis::set_main_module(const module::ptr& main_module) {
        assert(modules.empty());
        modules.push(main_module);
    }

    bool SyrecSynthesisNoAdditionalLines::on_module(const module::ptr& main) {
        for (const auto& stat: main->statements) {
            if (!full_statement(stat) && !onStatementNoAdditionalLines(stat))
                return false;
        }
        return assemble_circuit(cct_man.root);
    }

    bool SyrecSynthesisAdditionalLines::on_module(const module::ptr& main) {
        for (const statement::ptr& stat: main->statements) {
            if (!onStatementAdditionalLines(stat)) return false;
        }
        return assemble_circuit(cct_man.root);
    }

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

    /// If the input signals are repeated (i.e., rhs input signals are repeated)
    bool SyrecSynthesis::check_repeats() {
        std::vector check_lhs_vec(exp_lhs_vector.cbegin(), exp_lhs_vector.cend());
        std::vector check_rhs_vec(exp_rhs_vector.cbegin(), exp_rhs_vector.cend());

        for (unsigned k = 0; k < check_lhs_vec.size(); k++) {
            if (check_lhs_vec.at(k).empty()) {
                check_lhs_vec.erase(check_lhs_vec.begin() + k);
            }
        }

        for (unsigned k = 0; k < check_rhs_vec.size(); k++) {
            if (check_rhs_vec.at(k).empty()) {
                check_rhs_vec.erase(check_rhs_vec.begin() + k);
            }
        }

        for (int i = 0; i < int(check_rhs_vec.size()); i++) {
            for (int j = 0; j < int(check_rhs_vec.size()); j++) {
                if ((j != i) && (check_rhs_vec.at(i) == check_rhs_vec.at(j))) {
                    exp_op_vector.clear();
                    exp_lhs_vector.clear();
                    exp_rhs_vector.clear();
                    return true;
                }
            }
        }

        for (auto const& i: check_lhs_vec) {
            for (auto const& j: check_rhs_vec) {
                if (i == j) {
                    exp_op_vector.clear();
                    exp_lhs_vector.clear();
                    exp_rhs_vector.clear();
                    return true;
                }
            }
        }

        exp_op_vector.clear();
        exp_lhs_vector.clear();
        exp_rhs_vector.clear();
        return false;
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

    bool SyrecSynthesisAdditionalLines::onStatementAdditionalLines(const statement::ptr& statement) {
        _stmts.push(statement);
        bool okay = false;
        if (auto const* swapStat = dynamic_cast<swap_statement*>(statement.get())) {
            okay = SyrecSynthesis::on_statement(*swapStat);
        } else if (auto const* unaryStat = dynamic_cast<unary_statement*>(statement.get())) {
            okay = SyrecSynthesis::on_statement(*unaryStat);
        } else if (auto const* assignStat = dynamic_cast<assign_statement*>(statement.get())) {
            okay = onStatementAdditionalLines(*assignStat);
        } else if (auto const* ifStat = dynamic_cast<if_statement*>(statement.get())) {
            okay = onStatementAdditionalLines(*ifStat);
        } else if (auto const* forStat = dynamic_cast<for_statement*>(statement.get())) {
            okay = onStatementAdditionalLines(*forStat);
        } else if (auto const* callStat = dynamic_cast<call_statement*>(statement.get())) {
            okay = onStatementAdditionalLines(*callStat);
        } else if (auto const* uncallStat = dynamic_cast<uncall_statement*>(statement.get())) {
            okay = onStatementAdditionalLines(*uncallStat);
        } else if (auto const* skipStat = statement.get()) {
            okay = SyrecSynthesis::on_statement(*skipStat);
        } else {
            return false;
        }

        _stmts.pop();
        return okay;
    }

    bool SyrecSynthesisNoAdditionalLines::onStatementNoAdditionalLines(const statement::ptr& statement) {
        _stmts.push(statement);
        bool okay = false;
        if (auto const* swap_stat = dynamic_cast<swap_statement*>(statement.get())) {
            okay = SyrecSynthesis::on_statement(*swap_stat);
        } else if (auto const* unary_stat = dynamic_cast<unary_statement*>(statement.get())) {
            okay = SyrecSynthesis::on_statement(*unary_stat);
        } else if (auto const* assign_stat = dynamic_cast<assign_statement*>(statement.get())) {
            okay = onStatementNoAdditionalLines(*assign_stat);
        } else if (auto const* if_stat = dynamic_cast<if_statement*>(statement.get())) {
            okay = onStatementNoAdditionalLines(*if_stat);
        } else if (auto const* for_stat = dynamic_cast<for_statement*>(statement.get())) {
            okay = onStatementNoAdditionalLines(*for_stat);
        } else if (auto const* call_stat = dynamic_cast<call_statement*>(statement.get())) {
            okay = onStatementNoAdditionalLines(*call_stat);
        } else if (auto const* uncall_stat = dynamic_cast<uncall_statement*>(statement.get())) {
            okay = onStatementNoAdditionalLines(*uncall_stat);
        } else if (auto const* skip_stat = statement.get()) {
            okay = SyrecSynthesis::on_statement(*skip_stat);
        } else {
            return false;
        }

        _stmts.pop();
        return okay;
    }

    bool SyrecSynthesis::on_statement(const swap_statement& statement) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;

        get_variables(statement.lhs, lhs);
        get_variables(statement.rhs, rhs);

        assert(lhs.size() == rhs.size());

        swap(lhs, rhs);

        return true;
    }

    bool SyrecSynthesis::on_statement(const unary_statement& statement) {
        // load variable
        std::vector<unsigned> var;
        get_variables(statement.var, var);

        switch (statement.op) {
            case unary_statement::invert:
                bitwise_negation(var);
                break;
            case unary_statement::increment:
                increment(var);
                break;
            case unary_statement::decrement:
                decrement(var);
                break;
            default:
                return false;
        }
        return true;
    }

    bool SyrecSynthesisAdditionalLines::onStatementAdditionalLines(const assign_statement& statement) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;
        get_variables(statement.lhs, lhs);
        onExpressionAdditionalLines(statement.rhs, rhs);

        assert(lhs.size() == rhs.size());

        bool status = false;

        switch (statement.op) {
            case assign_statement::add: {
                status = increase(lhs, rhs);
            } break;

            case assign_statement::subtract: {
                status = decrease(lhs, rhs);
            } break;

            case assign_statement::exor: {
                status = bitwise_cnot(lhs, rhs);
            } break;

            default:
                return false;
        }
        return status;
    }

    bool SyrecSynthesisNoAdditionalLines::onStatementNoAdditionalLines(const assign_statement& statement) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;
        std::vector<unsigned> d;

        get_variables(statement.lhs, lhs);

        op_rhs_lhs_expression(statement.rhs, d);
        onExpressionNoAdditionalLines(statement.rhs, rhs, lhs, statement.op);
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

    bool SyrecSynthesisAdditionalLines::onStatementAdditionalLines(const if_statement& statement) {
        // calculate expression
        std::vector<unsigned> expression_result;
        onExpressionAdditionalLines(statement.condition, expression_result);
        assert(expression_result.size() == 1u);

        // add new helper line
        unsigned helper_line = expression_result.front();

        // activate this line
        add_active_control(helper_line);

        for (const statement::ptr& stat: statement.then_statements) {
            if (!onStatementAdditionalLines(stat)) return false;
        }

        // toggle helper line
        remove_active_control(helper_line);
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(helper_line);
        add_active_control(helper_line);

        for (const statement::ptr& stat: statement.else_statements) {
            if (!onStatementAdditionalLines(stat)) return false;
        }

        // de-active helper line
        remove_active_control(helper_line);
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(helper_line);

        return true;
    }

    bool SyrecSynthesisNoAdditionalLines::onStatementNoAdditionalLines(const if_statement& statement) {
        // calculate expression
        std::vector<unsigned> expression_result;
        std::vector<unsigned> lhs_stat;
        unsigned              op = 0u;
        onExpressionNoAdditionalLines(statement.condition, expression_result, lhs_stat, op);
        assert(expression_result.size() == 1u);

        // add new helper line
        unsigned helper_line = expression_result.front();

        // activate this line
        add_active_control(helper_line);

        for (const auto& stat: statement.then_statements) {
            if ((!full_statement(stat)) && (!onStatementNoAdditionalLines(stat)))
                return false;
        }

        // toggle helper line
        remove_active_control(helper_line);
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(helper_line);
        add_active_control(helper_line);

        for (const auto& stat: statement.else_statements) {
            if ((!full_statement(stat)) && (!onStatementNoAdditionalLines(stat)))
                return false;
        }

        // de-active helper line
        remove_active_control(helper_line);
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(helper_line);

        return true;
    }

    bool SyrecSynthesisAdditionalLines::onStatementAdditionalLines(const for_statement& statement) {
        const auto& [nfrom, nto] = statement.range;

        const unsigned     from          = nfrom ? nfrom->evaluate(loop_map) : 1u; // default value is 1u
        const unsigned     to            = nto->evaluate(loop_map);
        const unsigned     step          = statement.step ? statement.step->evaluate(loop_map) : 1u; // default step is +1
        const std::string& loop_variable = statement.loop_variable;

        if (from <= to) {
            for (unsigned i = from; i <= to; i += step) {
                // adjust loop variable if necessary

                if (!loop_variable.empty()) {
                    loop_map[loop_variable] = i;
                }

                for (const auto& stat: statement.statements) {
                    if (!onStatementAdditionalLines(stat)) {
                        return false;
                    }
                }
            }
        }

        else if (from > to) {
            for (auto i = static_cast<int>(from); i >= static_cast<int>(to); i -= static_cast<int>(step)) {
                // adjust loop variable if necessary

                if (!loop_variable.empty()) {
                    loop_map[loop_variable] = i;
                }

                for (const auto& stat: statement.statements) {
                    if (!onStatementAdditionalLines(stat)) {
                        return false;
                    }
                }
            }
        }
        // clear loop variable if necessary
        if (!loop_variable.empty()) {
            assert(loop_map.erase(loop_variable) == 1u);
        }

        return true;
    }

    bool SyrecSynthesisNoAdditionalLines::onStatementNoAdditionalLines(const for_statement& statement) {
        const auto& [nfrom, nto] = statement.range;

        const unsigned     from          = nfrom ? nfrom->evaluate(SyrecSynthesis::loop_map) : 1u; // default value is 1u
        const unsigned     to            = nto->evaluate(SyrecSynthesis::loop_map);
        const unsigned     step          = statement.step ? statement.step->evaluate(loop_map) : 1u; // default step is +1
        const std::string& loop_variable = statement.loop_variable;

        if (from <= to) {
            for (unsigned i = from; i <= to; i += step) {
                // adjust loop variable if necessary

                if (!loop_variable.empty()) {
                    loop_map[loop_variable] = i;
                }

                for (const auto& stat: statement.statements) {
                    if ((!full_statement(stat)) && (!onStatementNoAdditionalLines(stat)))
                        return false;
                }
            }
        }

        else if (from > to) {
            for (auto i = static_cast<int>(from); i >= static_cast<int>(to); i -= static_cast<int>(step)) {
                // adjust loop variable if necessary

                if (!loop_variable.empty()) {
                    loop_map[loop_variable] = i;
                }

                for (const auto& stat: statement.statements) {
                    if ((!full_statement(stat)) && (!onStatementNoAdditionalLines(stat)))
                        return false;
                }
            }
        }
        // clear loop variable if necessary
        if (!loop_variable.empty()) {
            assert(loop_map.erase(loop_variable) == 1u);
        }

        return true;
    }

    bool SyrecSynthesisAdditionalLines::onStatementAdditionalLines(const call_statement& statement) {
        // 1. Adjust the references module's parameters to the call arguments
        for (unsigned i = 0u; i < statement.parameters.size(); ++i) {
            const std::string&   parameter        = statement.parameters.at(i);
            const variable::ptr& module_parameter = statement.target->parameters.at(i);

            module_parameter->set_reference(modules.top()->find_parameter_or_variable(parameter));
        }

        // 2. Create new lines for the module's variables
        add_variables(_circ, statement.target->variables);

        modules.push(statement.target);
        for (const statement::ptr& stat: statement.target->statements) {
            if (!onStatementAdditionalLines(stat)) return false;
        }
        modules.pop();

        return true;
    }

    bool SyrecSynthesisNoAdditionalLines::onStatementNoAdditionalLines(const call_statement& statement) {
        // 1. Adjust the references module's parameters to the call arguments
        for (unsigned i = 0u; i < statement.parameters.size(); ++i) {
            const std::string& parameter        = statement.parameters.at(i);
            const auto&        module_parameter = statement.target->parameters.at(i);

            module_parameter->set_reference(modules.top()->find_parameter_or_variable(parameter));
        }

        // 2. Create new lines for the module's variables
        add_variables(_circ, statement.target->variables);

        modules.push(statement.target);
        for (const auto& stat: statement.target->statements) {
            if ((!full_statement(stat)) && (!onStatementNoAdditionalLines(stat)))
                return false;
        }

        modules.pop();

        return true;
    }

    bool SyrecSynthesisAdditionalLines::onStatementAdditionalLines(const uncall_statement& statement) {
        // 1. Adjust the references module's parameters to the call arguments
        for (unsigned i = 0u; i < statement.parameters.size(); ++i) {
            const std::string& parameter        = statement.parameters.at(i);
            const auto&        module_parameter = statement.target->parameters.at(i);

            module_parameter->set_reference(modules.top()->find_parameter_or_variable(parameter));
        }

        // 2. Create new lines for the module's variables
        add_variables(_circ, statement.target->variables);

        modules.push(statement.target);

        const auto statements = statement.target->statements;
        for (auto it = statements.rbegin(); it != statements.rend(); ++it) {
            const auto reverse_statement = (*it)->reverse();
            if (!onStatementAdditionalLines(reverse_statement)) {
                return false;
            }
        }

        modules.pop();

        return true;
    }

    bool SyrecSynthesisNoAdditionalLines::onStatementNoAdditionalLines(const uncall_statement& statement) {
        // 1. Adjust the references module's parameters to the call arguments
        for (unsigned i = 0u; i < statement.parameters.size(); ++i) {
            const std::string& parameter        = statement.parameters.at(i);
            const auto&        module_parameter = statement.target->parameters.at(i);

            module_parameter->set_reference(modules.top()->find_parameter_or_variable(parameter));
        }

        // 2. Create new lines for the module's variables
        add_variables(_circ, statement.target->variables);

        modules.push(statement.target);

        const auto statements = statement.target->statements;
        for (auto it = statements.rbegin(); it != statements.rend(); ++it) {
            const auto reverse_statement = (*it)->reverse();
            if ((!full_statement(reverse_statement)) && (!onStatementNoAdditionalLines(reverse_statement)))
                return false;
        }

        modules.pop();

        return true;
    }

    bool SyrecSynthesis::on_statement(const skip_statement& statement [[maybe_unused]]) const {
        return true;
    }

    bool SyrecSynthesisAdditionalLines::onExpressionAdditionalLines(const expression::ptr& expression, std::vector<unsigned>& lines) {
        if (const auto* numExp = dynamic_cast<numeric_expression*>(expression.get())) {
            return SyrecSynthesis::on_expression(*numExp, lines);
        } else if (const auto* varExp = dynamic_cast<variable_expression*>(expression.get())) {
            return SyrecSynthesis::on_expression(*varExp, lines);
        } else if (const auto* binExp = dynamic_cast<binary_expression*>(expression.get())) {
            return onExpressionAdditionalLines(*binExp, lines);
        } else if (const auto* shiftExp = dynamic_cast<shift_expression*>(expression.get())) {
            return onExpressionAdditionalLines(*shiftExp, lines);
        } else {
            return false;
        }
    }

    bool SyrecSynthesisAdditionalLines::onExpressionAdditionalLines(const binary_expression& expression, std::vector<unsigned>& lines) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;

        if (!onExpressionAdditionalLines(expression.lhs, lhs) || !onExpressionAdditionalLines(expression.rhs, rhs)) {
            return false;
        }

        switch (expression.op) {
            case binary_expression::add: // +
            {
                get_constant_lines(expression.bitwidth(), 0u, lines);

                bitwise_cnot(lines, lhs); // duplicate lhs

                increase(lines, rhs);
            } break;

            case binary_expression::subtract: // -
            {
                get_constant_lines(expression.bitwidth(), 0u, lines);

                bitwise_cnot(lines, lhs); // duplicate lhs

                decrease(lines, rhs);
            } break;

            case binary_expression::exor: // ^
            {
                get_constant_lines(expression.bitwidth(), 0u, lines);

                bitwise_cnot(lines, lhs); // duplicate lhs

                bitwise_cnot(lines, rhs);
            } break;

            case binary_expression::multiply: // *
            {
                get_constant_lines(expression.bitwidth(), 0u, lines);

                multiplication(lines, lhs, rhs);
            } break;

            case binary_expression::divide: // /
            {
                get_constant_lines(expression.bitwidth(), 0u, lines);

                division(lines, lhs, rhs);
            } break;

            case binary_expression::modulo: // %
            {
                get_constant_lines(expression.bitwidth(), 0u, lines);
                std::vector<unsigned> quot;
                get_constant_lines(expression.bitwidth(), 0u, quot);

                bitwise_cnot(lines, lhs); // duplicate lhs
                modulo(quot, lines, rhs);
            } break;

            case binary_expression::logical_and: // &&
            {
                lines.emplace_back(get_constant_line(false));

                conjunction(lines.at(0), lhs.at(0), rhs.at(0));
            } break;

            case binary_expression::logical_or: // ||
            {
                lines.emplace_back(get_constant_line(false));

                disjunction(lines.at(0), lhs.at(0), rhs.at(0));
            } break;

            case binary_expression::bitwise_and: // &
            {
                get_constant_lines(expression.bitwidth(), 0u, lines);

                bitwise_and(lines, lhs, rhs);
            } break;

            case binary_expression::bitwise_or: // |
            {
                get_constant_lines(expression.bitwidth(), 0u, lines);

                bitwise_or(lines, lhs, rhs);
            } break;

            case binary_expression::less_than: // <
            {
                lines.emplace_back(get_constant_line(false));

                less_than(lines.at(0), lhs, rhs);
            } break;

            case binary_expression::greater_than: // >
            {
                lines.emplace_back(get_constant_line(false));

                greater_than(lines.at(0), lhs, rhs);
            } break;

            case binary_expression::equals: // =
            {
                lines.emplace_back(get_constant_line(false));

                equals(lines.at(0), lhs, rhs);
            } break;

            case binary_expression::not_equals: // !=
            {
                lines.emplace_back(get_constant_line(false));

                not_equals(lines.at(0), lhs, rhs);
            } break;

            case binary_expression::less_equals: // <=
            {
                lines.emplace_back(get_constant_line(false));

                less_equals(lines.at(0), lhs, rhs);
            } break;

            case binary_expression::greater_equals: // >=
            {
                lines.emplace_back(get_constant_line(false));

                greater_equals(lines.at(0), lhs, rhs);
            } break;

            default:
                return false;
        }

        return true;
    }

    bool SyrecSynthesisNoAdditionalLines::onExpressionNoAdditionalLines(const expression::ptr& expression, std::vector<unsigned>& lines, std::vector<unsigned> const& lhs_stat, unsigned op) {
        if (auto const* numeric = dynamic_cast<numeric_expression*>(expression.get())) {
            return SyrecSynthesis::on_expression(*numeric, lines);
        } else if (auto const* variable = dynamic_cast<variable_expression*>(expression.get())) {
            return SyrecSynthesis::on_expression(*variable, lines);
        } else if (auto const* binary = dynamic_cast<binary_expression*>(expression.get())) {
            return onExpressionNoAdditionalLines(*binary, lines, lhs_stat, op);
        } else if (auto const* shift = dynamic_cast<shift_expression*>(expression.get())) {
            return onExpressionNoAdditionalLines(*shift, lines, lhs_stat, op);
        } else {
            return false;
        }
    }

    bool SyrecSynthesis::on_expression(const numeric_expression& expression, std::vector<unsigned>& lines) {
        get_constant_lines(expression.bitwidth(), expression.value->evaluate(loop_map), lines);
        return true;
    }

    bool SyrecSynthesis::on_expression(const variable_expression& expression, std::vector<unsigned>& lines) {
        get_variables(expression.var, lines);
        return true;
    }

    /// Function when the assignment statements consist of binary expressions and does not include repeated input signals

    bool SyrecSynthesisNoAdditionalLines::onExpressionNoAdditionalLines(const binary_expression& expression, std::vector<unsigned>& lines, std::vector<unsigned> const& lhs_stat, unsigned op) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;

        if (!onExpressionNoAdditionalLines(expression.lhs, lhs, lhs_stat, op) || !onExpressionNoAdditionalLines(expression.rhs, rhs, lhs_stat, op)) {
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

    bool SyrecSynthesisAdditionalLines::onExpressionAdditionalLines(const shift_expression& expression, std::vector<unsigned>& lines) {
        std::vector<unsigned> lhs;

        if (!onExpressionAdditionalLines(expression.lhs, lhs)) {
            return false;
        }

        unsigned rhs = expression.rhs->evaluate(loop_map);

        switch (expression.op) {
            case shift_expression::left: // <<
            {
                get_constant_lines(expression.bitwidth(), 0u, lines);

                left_shift(lines, lhs, rhs);
            } break;

            case shift_expression::right: // <<
            {
                get_constant_lines(expression.bitwidth(), 0u, lines);

                right_shift(lines, lhs, rhs);
            } break;

            default:
                return false;
        }

        return true;
    }

    bool SyrecSynthesisNoAdditionalLines::onExpressionNoAdditionalLines(const shift_expression& expression, std::vector<unsigned>& lines, std::vector<unsigned> const& lhs_stat, unsigned op) {
        std::vector<unsigned> lhs;
        if (!onExpressionNoAdditionalLines(expression.lhs, lhs, lhs_stat, op)) {
            return false;
        }

        unsigned rhs = expression.rhs->evaluate(loop_map);

        switch (expression.op) {
            case shift_expression::left: // <<
                get_constant_lines(expression.bitwidth(), 0u, lines);
                left_shift(lines, lhs, rhs);
                break;
            case shift_expression::right: // <<
                get_constant_lines(expression.bitwidth(), 0u, lines);
                right_shift(lines, lhs, rhs);
                break;
            default:
                return false;
        }

        return true;
    }

    //**********************************************************************
    //*****                      Unary Operations                      *****
    //**********************************************************************

    bool SyrecSynthesis::bitwise_negation(const std::vector<unsigned>& dest) {
        for (unsigned idx: dest) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(idx);
        }
        return true;
    }

    bool SyrecSynthesis::decrement(const std::vector<unsigned>& dest) {
        for (unsigned int i: dest) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(i);
            add_active_control(i);
        }

        for (unsigned int i: dest) {
            remove_active_control(i);
        }

        return true;
    }

    bool SyrecSynthesis::increment(const std::vector<unsigned>& dest) {
        for (unsigned int i: dest) {
            add_active_control(i);
        }

        for (int i = int(dest.size()) - 1; i >= 0; --i) {
            remove_active_control(dest.at(i));
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(dest.at(i));
        }

        return true;
    }

    //**********************************************************************
    //*****                     Binary Operations                      *****
    //**********************************************************************

    bool SyrecSynthesis::bitwise_and(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        bool ok = true;
        for (unsigned i = 0u; i < dest.size(); ++i) {
            ok &= conjunction(dest.at(i), src1.at(i), src2.at(i));
        }
        return ok;
    }

    bool SyrecSynthesis::bitwise_cnot(const std::vector<unsigned>& dest, const std::vector<unsigned>& src) {
        for (unsigned i = 0u; i < src.size(); ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src.at(i), dest.at(i));
        }
        return true;
    }

    bool SyrecSynthesis::bitwise_or(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        bool ok = true;
        for (unsigned i = 0u; i < dest.size(); ++i) {
            ok &= disjunction(dest.at(i), src1.at(i), src2.at(i));
        }
        return ok;
    }

    bool SyrecSynthesis::conjunction(unsigned dest, unsigned src1, unsigned src2) {
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_toffoli(src1, src2, dest);

        return true;
    }

    bool SyrecSynthesis::decrease_with_carry(const std::vector<unsigned>& dest, const std::vector<unsigned>& src, unsigned carry) {
        for (unsigned i = 0u; i < src.size(); ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(dest.at(i));
        }

        increase_with_carry(dest, src, carry);

        for (unsigned i = 0u; i < src.size(); ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(dest.at(i));
        }

        return true;
    }

    bool SyrecSynthesis::disjunction(unsigned dest, unsigned src1, unsigned src2) {
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src1, dest);
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src2, dest);
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_toffoli(src1, src2, dest);

        return true;
    }

    bool SyrecSynthesis::division(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if (!modulo(dest, src1, src2)) return false;

        std::vector<unsigned> sum;
        std::vector<unsigned> partial;

        for (unsigned i = 1u; i < src1.size(); ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(src2.at(i));
        }

        for (unsigned i = 1u; i < src1.size(); ++i) {
            add_active_control(src2.at(i));
        }

        for (int i = int(src1.size()) - 1; i >= 0; --i) {
            partial.push_back(src2.at(src1.size() - 1u - i));
            sum.insert(sum.begin(), src1.at(i));
            add_active_control(dest.at(i));
            increase(sum, partial);
            remove_active_control(dest.at(i));
            if (i > 0) {
                for (unsigned j = (static_cast<int>(src1.size()) - i); j < src1.size(); ++j) {
                    remove_active_control(src2.at(j));
                }
                (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(src2.at(src1.size() - i));
                for (unsigned j = (static_cast<int>(src1.size()) + 1u - i); j < src1.size(); ++j) {
                    add_active_control(src2.at(j));
                }
            }
        }

        return true;
    }

    bool SyrecSynthesis::equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        for (unsigned i = 0u; i < src1.size(); ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src2.at(i), src1.at(i));
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(src1.at(i));
        }

        gate::line_container controls(src1.begin(), src1.end());
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_multi_control_toffoli(controls, dest);

        for (unsigned i = 0u; i < src1.size(); ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src2.at(i), src1.at(i));
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(src1.at(i));
        }

        return true;
    }

    bool SyrecSynthesis::greater_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if (!greater_than(dest, src2, src1)) return false;
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(dest);

        return true;
    }

    bool SyrecSynthesis::greater_than(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        return less_than(dest, src2, src1);
    }

    bool SyrecSynthesis::increase(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs) {
        if (auto bitwidth = static_cast<int>(rhs.size()); bitwidth == 1) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(lhs.at(0), rhs.at(0));
        } else {
            for (int i = 1; i <= bitwidth - 1; ++i) {
                (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(lhs.at(i), rhs.at(i));
            }
            for (int i = bitwidth - 2; i >= 1; --i) {
                (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(lhs.at(i), lhs.at(i + 1));
            }
            for (int i = 0; i <= bitwidth - 2; ++i) {
                (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_toffoli(rhs.at(i), lhs.at(i), lhs.at(i + 1));
            }

            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(lhs.at(bitwidth - 1), rhs.at(bitwidth - 1));

            for (int i = bitwidth - 2; i >= 1; --i) {
                (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_toffoli(lhs.at(i), rhs.at(i), lhs.at(i + 1));
                (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(lhs.at(i), rhs.at(i));
            }
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_toffoli(lhs.at(0), rhs.at(0), lhs.at(1));
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(lhs.at(0), rhs.at(0));

            for (int i = 1; i <= bitwidth - 2; ++i) {
                (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(lhs.at(i), lhs.at(i + 1));
            }
            for (int i = 1; i <= bitwidth - 1; ++i) {
                (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(lhs.at(i), rhs.at(i));
            }
        }

        return true;
    }

    bool SyrecSynthesis::decrease(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs) {
        for (unsigned int rh: rhs) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(rh);
        }

        increase(rhs, lhs);

        for (unsigned int rh: rhs) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(rh);
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

    bool SyrecSynthesis::increase_with_carry(const std::vector<unsigned>& dest, const std::vector<unsigned>& src, unsigned carry) {
        auto bitwidth = static_cast<int>(src.size());

        if (bitwidth == 0) return true;

        for (int i = 1u; i < bitwidth; ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src.at(i), dest.at(i));
        }

        if (bitwidth > 1) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src.at(bitwidth - 1), carry);
        }
        for (int i = bitwidth - 2; i > 0; --i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src.at(i), src.at(i + 1));
        }

        for (int i = 0u; i < bitwidth - 1; ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_toffoli(src.at(i), dest.at(i), src.at(i + 1));
        }
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_toffoli(src.at(bitwidth - 1), dest.at(bitwidth - 1), carry);

        for (int i = bitwidth - 1; i > 0; --i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src.at(i), dest.at(i));
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_toffoli(dest.at(i - 1), src.at(i - 1), src.at(i));
        }

        for (int i = 1u; i < bitwidth - 1; ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src.at(i), src.at(i + 1));
        }

        for (int i = 0u; i < bitwidth; ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src.at(i), dest.at(i));
        }

        return true;
    }

    bool SyrecSynthesis::less_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if (!less_than(dest, src2, src1)) return false;
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(dest);

        return true;
    }

    bool SyrecSynthesis::less_than(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        return (decrease_with_carry(src1, src2, dest) && increase(src1, src2));
    }

    bool SyrecSynthesis::modulo(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        std::vector<unsigned> sum;
        std::vector<unsigned> partial;

        for (unsigned i = 1u; i < src1.size(); ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(src2.at(i));
        }

        for (unsigned i = 1u; i < src1.size(); ++i) {
            add_active_control(src2.at(i));
        }

        for (int i = int(src1.size()) - 1; i >= 0; --i) {
            partial.push_back(src2.at(src1.size() - 1u - i));
            sum.insert(sum.begin(), src1.at(i));
            decrease_with_carry(sum, partial, dest.at(i));
            add_active_control(dest.at(i));
            increase(sum, partial);
            remove_active_control(dest.at(i));
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(dest.at(i));
            if (i > 0) {
                for (unsigned j = (static_cast<int>(src1.size()) - i); j < src1.size(); ++j) {
                    remove_active_control(src2.at(j));
                }
                (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(src2.at(src1.size() - i));
                for (unsigned j = (static_cast<int>(src1.size()) + 1u - i); j < src1.size(); ++j) {
                    add_active_control(src2.at(j));
                }
            }
        }
        return true;
    }

    bool SyrecSynthesis::multiplication(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if ((src1.empty()) || (dest.empty())) return true;

        std::vector<unsigned> sum     = dest;
        std::vector<unsigned> partial = src2;

        bool ok = true;

        add_active_control(src1.at(0));
        ok = ok && bitwise_cnot(sum, partial);
        remove_active_control(src1.at(0));

        for (unsigned i = 1; i < dest.size(); ++i) {
            sum.erase(sum.begin());
            partial.pop_back();
            add_active_control(src1.at(i));
            ok = ok && increase(sum, partial);
            remove_active_control(src1.at(i));
        }

        return ok;
    }

    bool SyrecSynthesis::not_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if (!equals(dest, src1, src2)) return false;
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(dest);
        return true;
    }

    void SyrecSynthesis::swap(const std::vector<unsigned>& dest1, const std::vector<unsigned>& dest2) {
        for (unsigned i = 0u; i < dest1.size(); ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_fredkin(dest1.at(i), dest2.at(i));
        }
    }

    //**********************************************************************
    //*****                      Shift Operations                      *****
    //**********************************************************************

    void SyrecSynthesis::left_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2) {
        for (unsigned i = 0u; (i + src2) < dest.size(); ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src1.at(i), dest.at(i + src2));
        }
    }

    void SyrecSynthesis::right_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2) {
        for (unsigned i = src2; i < dest.size(); ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src1.at(i), dest.at(i - src2));
        }
    }

    //**********************************************************************
    //*****                     Efficient Controls                     *****
    //**********************************************************************

    void SyrecSynthesis::add_active_control(unsigned control) {
        // aktuelles Blatt vollendet, zurueck zum parent
        cct_man.current = source(*(in_edges(cct_man.current, cct_man.tree).first), cct_man.tree);

        // child fuer neuen control anlegen
        cct_node child                                        = add_vertex(cct_man.tree);
        get(boost::vertex_name, cct_man.tree)[child].control  = control;
        get(boost::vertex_name, cct_man.tree)[child].controls = get(boost::vertex_name, cct_man.tree)[cct_man.current].controls;
        get(boost::vertex_name, cct_man.tree)[child].controls.insert(control);
        add_edge(cct_man.current, child, cct_man.tree);
        cct_man.current = child;

        // neues Blatt anlegen
        cct_node leaf                                        = add_vertex(cct_man.tree);
        get(boost::vertex_name, cct_man.tree)[leaf].controls = get(boost::vertex_name, cct_man.tree)[cct_man.current].controls;
        get(boost::vertex_name, cct_man.tree)[leaf].circ     = std::make_shared<circuit>();
        get(boost::vertex_name, cct_man.tree)[leaf].circ->gate_added.connect(annotater(*get(boost::vertex_name, cct_man.tree)[leaf].circ, _stmts));
        add_edge(cct_man.current, leaf, cct_man.tree);
        cct_man.current = leaf;
    }

    void SyrecSynthesis::remove_active_control(unsigned control [[maybe_unused]]) {
        // aktuelles Blatt vollendet, zurueck zum parent
        cct_man.current = source(*(in_edges(cct_man.current, cct_man.tree).first), cct_man.tree);

        // aktueller Knoten abgeschlossen, zurueck zum parent
        cct_man.current = source(*(in_edges(cct_man.current, cct_man.tree).first), cct_man.tree);

        // neues Blatt anlegen
        cct_node leaf                                        = add_vertex(cct_man.tree);
        get(boost::vertex_name, cct_man.tree)[leaf].controls = get(boost::vertex_name, cct_man.tree)[cct_man.current].controls;
        get(boost::vertex_name, cct_man.tree)[leaf].circ     = std::make_shared<circuit>();
        get(boost::vertex_name, cct_man.tree)[leaf].circ->gate_added.connect(annotater(*get(boost::vertex_name, cct_man.tree)[leaf].circ, _stmts));
        add_edge(cct_man.current, leaf, cct_man.tree);
        cct_man.current = leaf;
    }

    bool SyrecSynthesis::assemble_circuit(const cct_node& current) {
        // leaf
        if (out_edges(current, cct_man.tree).first == out_edges(current, cct_man.tree).second /*get( boost::vertex_name, cct_man.tree )[current].circ.get()->num_gates() > 0u*/) {
            _circ.insert_circuit(_circ.num_gates(), *(get(boost::vertex_name, cct_man.tree)[current].circ), get(boost::vertex_name, cct_man.tree)[current].controls);
            return true;
        }
        // assemble optimized circuits of successors
        for (auto edge_it = out_edges(current, cct_man.tree).first; edge_it != out_edges(current, cct_man.tree).second; ++edge_it) {
            if (!assemble_circuit(target(*edge_it, cct_man.tree))) {
                return false;
            }
        }
        return true;
    }

    void SyrecSynthesis::get_variables(const variable_access::ptr& var, std::vector<unsigned>& lines) {
        unsigned offset = _var_lines[var->get_var()];

        if (!var->indexes.empty()) {
            // check if it is all numeric_expressions
            unsigned n = static_cast<int>(var->get_var()->dimensions.size()); // dimensions
            if ((unsigned)std::count_if(var->indexes.cbegin(), var->indexes.cend(), [&](const auto& p) { return dynamic_cast<numeric_expression*>(p.get()); }) == n) {
                for (unsigned i = 0u; i < n; ++i) {
                    offset += dynamic_cast<numeric_expression*>(var->indexes.at(i).get())->value->evaluate(loop_map) *
                              std::accumulate(var->get_var()->dimensions.begin() + i + 1u, var->get_var()->dimensions.end(), 1u, std::multiplies<>()) *
                              var->get_var()->bitwidth;
                }
            }
        }

        if (var->range) {
            auto [nfirst, nsecond] = *var->range;

            unsigned first  = nfirst->evaluate(loop_map);
            unsigned second = nsecond->evaluate(loop_map);

            if (first < second) {
                for (unsigned i = first; i <= second; ++i) {
                    lines.emplace_back(offset + i);
                }
            } else {
                for (auto i = static_cast<int>(first); i >= static_cast<int>(second); --i) {
                    lines.emplace_back(offset + i);
                }
            }
        } else {
            for (unsigned i = 0u; i < var->get_var()->bitwidth; ++i) {
                lines.emplace_back(offset + i);
            }
        }
    }

    /**
   * Function to access array variables
   *
   * The array variable that corresponds to the given indexes is exchanged (via swap operations) with some given helper lines
   *
   * \param offset is the first line number associated to the array
   * \param dimensions is the dimensions of the array
   * \param indexes is the indexes of the array
   * \param bitwidth is the bitwidth of the variables within the array
   * \param lines is the destination, where
   */
    unsigned SyrecSynthesis::get_constant_line(bool value) {
        unsigned const_line = 0u;

        if (!free_const_lines_map[value].empty()) {
            const_line = free_const_lines_map[value].back();
            free_const_lines_map[value].pop_back();
        } else if (!free_const_lines_map[!value].empty()) {
            const_line = free_const_lines_map[!value].back();
            free_const_lines_map[!value].pop_back();
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(const_line);
        } else {
            const_line = _circ.add_line((std::string("const_") + std::to_string(value)), "garbage", value, true);
        }

        return const_line;
    }

    void SyrecSynthesis::get_constant_lines(unsigned bitwidth, unsigned value, std::vector<unsigned>& lines) {
        boost::dynamic_bitset<> number(bitwidth, value);

        for (unsigned i = 0u; i < bitwidth; ++i) {
            lines.emplace_back(get_constant_line(number.test(i)));
        }
    }

    void SyrecSynthesis::add_variable(circuit& circ, const std::vector<unsigned>& dimensions, const variable::ptr& var,
                                      constant _constant, bool _garbage, const std::string& arraystr) {
        if (dimensions.empty()) {
            for (unsigned i = 0u; i < var->bitwidth; ++i) {
                std::string name = var->name + arraystr + "." + std::to_string(i);
                circ.add_line(name, name, _constant, _garbage);
            }
        } else {
            unsigned              len = dimensions.front();
            std::vector<unsigned> new_dimensions(dimensions.begin() + 1u, dimensions.end());

            for (unsigned i = 0u; i < len; ++i) {
                add_variable(circ, new_dimensions, var, _constant, _garbage, arraystr + "[" + std::to_string(i) + "]");
            }
        }
    }

    void SyrecSynthesis::add_variables(circuit& circ, const variable::vec& variables) {
        for (const auto& var: variables) {
            // entry in var lines map
            _var_lines.try_emplace(var, circ.get_lines());

            // types of constant and garbage
            constant _constant = (var->type == variable::out || var->type == variable::wire) ? constant(false) : constant();
            bool     _garbage  = (var->type == variable::in || var->type == variable::wire);

            add_variable(circ, var->dimensions, var, _constant, _garbage, std::string());
        }
    }

    bool SyrecSynthesisAdditionalLines::synthesize(circuit& circ, const program& program, const properties::ptr& settings, const properties::ptr& statistics) {
        // Settings parsing
        auto                          variable_name_format = get<std::string>(settings, "variable_name_format", "%1$s%3$s.%2$d");
        auto                          main_module          = get<std::string>(settings, "main_module", std::string());
        SyrecSynthesisAdditionalLines statement_synthesizer(circ);
        // Run-time measuring
        timer<properties_timer> t;

        if (statistics) {
            properties_timer rt(statistics);
            t.start(rt);
        }

        // get the main module
        module::ptr main;

        if (!main_module.empty()) {
            main = program.find_module(main_module);
            if (!main) {
                std::cerr << "Program has no module: " << main_module << std::endl;
                return false;
            }
        } else {
            main = program.find_module("main");
            if (!main) {
                main = program.modules().front();
            }
        }

        // declare as top module
        statement_synthesizer.set_main_module(main);

        // create lines for global variables
        statement_synthesizer.add_variables(circ, main->parameters);
        statement_synthesizer.add_variables(circ, main->variables);

        // synthesize the statements
        return statement_synthesizer.on_module(main);
    }

    bool SyrecSynthesisNoAdditionalLines::synthesize(circuit& circ, const program& program, const properties::ptr& settings, const properties::ptr& statistics) {
        // Settings parsing
        auto                            variable_name_format = get<std::string>(settings, "variable_name_format", "%1$s%3$s.%2$d");
        auto                            main_module          = get<std::string>(settings, "main_module", std::string());
        SyrecSynthesisNoAdditionalLines statement_synthesizer(circ);
        // get the main module
        module::ptr main;

        if (!main_module.empty()) {
            main = program.find_module(main_module);
            if (!main) {
                std::cerr << "Program has no module: " << main_module << std::endl;
                return false;
            }
        } else {
            main = program.find_module("main");
            if (!main) {
                main = program.modules().front();
            }
        }

        // declare as top module
        statement_synthesizer.set_main_module(main);

        // create lines for global variables
        statement_synthesizer.add_variables(circ, main->parameters);
        statement_synthesizer.add_variables(circ, main->variables);

        // Run-time measuring
        timer<properties_timer> t;

        if (statistics) {
            properties_timer rt(statistics);
            t.start(rt);
        }
        // synthesize the statements
        bool synth_okay = statement_synthesizer.on_module(main);

        if (statistics) {
            t.stop();
        }

        return synth_okay;
    }

} // namespace syrec
