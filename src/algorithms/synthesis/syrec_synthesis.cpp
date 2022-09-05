#include "algorithms/synthesis/syrec_synthesis.hpp"

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

    bool SyrecSynthesis::on_module(const module::ptr& main) {
        for (const auto& stat: main->statements) {
            if (process_statement(stat)) {
                return false;
            }
        }
        return assemble_circuit(cct_man.root);
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

    bool SyrecSynthesis::op_rhs_lhs_expression([[maybe_unused]] const expression::ptr& expression, [[maybe_unused]] std::vector<unsigned>& v) {
        return true;
    }
    bool SyrecSynthesis::op_rhs_lhs_expression([[maybe_unused]] const variable_expression& expression, [[maybe_unused]] std::vector<unsigned>& v) {
        return true;
    }
    bool SyrecSynthesis::op_rhs_lhs_expression([[maybe_unused]] const binary_expression& expression, [[maybe_unused]] std::vector<unsigned>& v) {
        return true;
    }

    bool SyrecSynthesis::on_statement(const statement::ptr& statement) {
        _stmts.push(statement);
        bool okay = false;
        if (auto const* swapStat = dynamic_cast<swap_statement*>(statement.get())) {
            okay = on_statement(*swapStat);
        } else if (auto const* unaryStat = dynamic_cast<unary_statement*>(statement.get())) {
            okay = on_statement(*unaryStat);
        } else if (auto const* assignStat = dynamic_cast<assign_statement*>(statement.get())) {
            okay = on_statement(*assignStat);
        } else if (auto const* ifStat = dynamic_cast<if_statement*>(statement.get())) {
            okay = on_statement(*ifStat);
        } else if (auto const* forStat = dynamic_cast<for_statement*>(statement.get())) {
            okay = on_statement(*forStat);
        } else if (auto const* callStat = dynamic_cast<call_statement*>(statement.get())) {
            okay = on_statement(*callStat);
        } else if (auto const* uncallStat = dynamic_cast<uncall_statement*>(statement.get())) {
            okay = on_statement(*uncallStat);
        } else if (auto const* skipStat = statement.get()) {
            okay = on_statement(*skipStat);
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

    bool SyrecSynthesis::on_statement(const assign_statement& statement) {
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
                assign_add(status, lhs, rhs, statement.op);
            } break;

            case assign_statement::subtract: {
                assign_subtract(status, lhs, rhs, statement.op);
            } break;

            case assign_statement::exor: {
                assign_exor(status, lhs, rhs, statement.op);
            } break;

            default:
                return false;
        }

        return status;
    }

    bool SyrecSynthesis::on_statement(const if_statement& statement) {
        // calculate expression
        std::vector<unsigned> expression_result;
        on_expression(statement.condition, expression_result, {}, 0U);
        assert(expression_result.size() == 1u);

        // add new helper line
        unsigned helper_line = expression_result.front();

        // activate this line
        add_active_control(helper_line);

        for (const statement::ptr& stat: statement.then_statements) {
            if (process_statement(stat)) {
                return false;
            }
        }

        // toggle helper line
        remove_active_control(helper_line);
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(helper_line);
        add_active_control(helper_line);

        for (const statement::ptr& stat: statement.else_statements) {
            if (process_statement(stat)) {
                return false;
            }
        }

        // de-active helper line
        remove_active_control(helper_line);
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(helper_line);

        return true;
    }

    bool SyrecSynthesis::on_statement(const for_statement& statement) {
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
                    if (process_statement(stat)) {
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
                    if (process_statement(stat)) {
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

    bool SyrecSynthesis::on_statement(const call_statement& statement) {
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
            if (process_statement(stat)) {
                return false;
            }
        }
        modules.pop();

        return true;
    }

    bool SyrecSynthesis::on_statement(const uncall_statement& statement) {
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
            if (process_statement(reverse_statement)) {
                return false;
            }
        }

        modules.pop();

        return true;
    }

    bool SyrecSynthesis::on_statement(const skip_statement& statement [[maybe_unused]]) {
        return true;
    }

    bool SyrecSynthesis::on_expression(const expression::ptr& expression, std::vector<unsigned>& lines, std::vector<unsigned> const& lhs_stat, unsigned op) {
        if (auto const* numeric = dynamic_cast<numeric_expression*>(expression.get())) {
            return on_expression(*numeric, lines);
        } else if (auto const* variable = dynamic_cast<variable_expression*>(expression.get())) {
            return on_expression(*variable, lines);
        } else if (auto const* binary = dynamic_cast<binary_expression*>(expression.get())) {
            return on_expression(*binary, lines, lhs_stat, op);
        } else if (auto const* shift = dynamic_cast<shift_expression*>(expression.get())) {
            return on_expression(*shift, lines, lhs_stat, op);
        } else {
            return false;
        }
    }

    bool SyrecSynthesis::on_expression(const shift_expression& expression, std::vector<unsigned>& lines, std::vector<unsigned> const& lhs_stat, unsigned op) {
        std::vector<unsigned> lhs;
        if (!on_expression(expression.lhs, lhs, lhs_stat, op)) {
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

    bool SyrecSynthesis::on_expression(const numeric_expression& expression, std::vector<unsigned>& lines) {
        get_constant_lines(expression.bitwidth(), expression.value->evaluate(loop_map), lines);
        return true;
    }

    bool SyrecSynthesis::on_expression(const variable_expression& expression, std::vector<unsigned>& lines) {
        get_variables(expression.var, lines);
        return true;
    }

    bool SyrecSynthesis::on_expression(const binary_expression& expression, std::vector<unsigned>& lines, std::vector<unsigned> const& lhs_stat, unsigned op) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;

        if (!on_expression(expression.lhs, lhs, lhs_stat, op) || !on_expression(expression.rhs, rhs, lhs_stat, op)) {
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
                exp_add(expression.bitwidth(), lines, lhs, rhs);
                break;
            case binary_expression::subtract: // -
                exp_subtract(expression.bitwidth(), lines, lhs, rhs);
                break;
            case binary_expression::exor: // ^
                exp_exor(expression.bitwidth(), lines, lhs, rhs);
                break;
            case binary_expression::multiply: // *
                get_constant_lines(expression.bitwidth(), 0u, lines);
                multiplication(lines, lhs, rhs);
                break;
            case binary_expression::divide: // /
                get_constant_lines(expression.bitwidth(), 0u, lines);
                division(lines, lhs, rhs);
                break;
            case binary_expression::modulo: { // %
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

    /// Function when the assignment statements consist of binary expressions and does not include repeated input signals

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

    bool SyrecSynthesis::expression_op_inverse([[maybe_unused]] unsigned op, [[maybe_unused]] const std::vector<unsigned>& exp_lhs, [[maybe_unused]] const std::vector<unsigned>& exp_rhs) {
        return true;
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

    bool SyrecSynthesis::synthesize(SyrecSynthesis* synthesizer, circuit& circ, const program& program, const properties::ptr& settings, const properties::ptr& statistics) {
        // Settings parsing
        auto variable_name_format = get<std::string>(settings, "variable_name_format", "%1$s%3$s.%2$d");
        auto main_module          = get<std::string>(settings, "main_module", std::string());
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
        synthesizer->set_main_module(main);

        // create lines for global variables
        synthesizer->add_variables(circ, main->parameters);
        synthesizer->add_variables(circ, main->variables);

        // synthesize the statements
        return synthesizer->on_module(main);
    }

} // namespace syrec
