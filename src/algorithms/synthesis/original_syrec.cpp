#include "algorithms/synthesis/original_syrec.hpp"

#include "core/syrec/variable.hpp"
#include "core/utils/timer.hpp"

#include <functional>
#include <numeric>

namespace syrec {
    struct annotater {
        explicit annotater(circuit& circ, const std::stack<statement::ptr>& stmts):
            _circ(circ),
            _stmts(stmts) {}

        // Operator needs this signature to work
        void operator()(gate const& g) const {
            if (!_stmts.empty()) {
                _circ.annotate(g, "lno", std::to_string(_stmts.top()->line_number)); //(gate, key,value)
            }
        }

    private:
        circuit&                          _circ;
        const std::stack<statement::ptr>& _stmts;
    };

    standardSyrecSynthesizerAdditionalLines::standardSyrecSynthesizerAdditionalLines(circuit& circ, [[maybe_unused]] const syrec::program& prog):
        _circ(circ) {
        free_const_lines_map.try_emplace(false, std::vector<unsigned>());
        free_const_lines_map.try_emplace(true, std::vector<unsigned>());

        // root anlegen
        cct_man.current = add_vertex(cct_man.tree);
        cct_man.root    = cct_man.current;
        // Blatt anlegen
        cct_man.current                                             = add_vertex(cct_man.tree);
        get(boost::vertex_name, cct_man.tree)[cct_man.current].circ = std::make_shared<circuit>();
        get(boost::vertex_name, cct_man.tree)[cct_man.current].circ->gate_added.connect(annotater(*get(boost::vertex_name, cct_man.tree)[cct_man.current].circ, _stmts)); //previous func: lno: line number
        add_edge(cct_man.root, cct_man.current, cct_man.tree);                                                                                                            // (u,v,graph)
    }

    // for compiling with MacOS

    void standardSyrecSynthesizerAdditionalLines::set_main_module(const module::ptr& main_module) {
        assert(modules.empty());
        modules.push(main_module);
    }

    bool standardSyrecSynthesizerAdditionalLines::on_module(const module::ptr& main) {
        for (const statement::ptr& stat: main->statements) {
            if (!on_statement(stat)) return false;
        }
        return assemble_circuit(cct_man.root);
    }

    bool standardSyrecSynthesizerAdditionalLines::on_statement(const statement::ptr& statement) {
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
        } else if (auto const* skipStat = dynamic_cast<skip_statement*>(statement.get())) {
            okay = on_statement(*skipStat);
        } else {
            return false;
        }

        _stmts.pop();
        return okay;
    }

    bool standardSyrecSynthesizerAdditionalLines::on_statement(const swap_statement& statement) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;

        get_variables(statement.lhs, lhs);
        get_variables(statement.rhs, rhs);

        assert(lhs.size() == rhs.size());

        swap(lhs, rhs);

        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::on_statement(const unary_statement& statement) {
        std::vector<unsigned> var;
        get_variables(statement.var, var);

        switch (statement.op) {
            case unary_statement::invert: {
                bitwise_negation(var);
            } break;

            case unary_statement::increment: {
                increment(var);
            } break;

            case unary_statement::decrement: {
                decrement(var);
            } break;
            default: {
                return false;
            }
        }
        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::on_statement(const assign_statement& statement) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;
        get_variables(statement.lhs, lhs);
        on_expression(statement.rhs, rhs);

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

    bool standardSyrecSynthesizerAdditionalLines::on_statement(const if_statement& statement) {
        // calculate expression
        std::vector<unsigned> expression_result;
        on_expression(statement.condition, expression_result);
        assert(expression_result.size() == 1u);

        // add new helper line
        unsigned helper_line = expression_result.front();

        // activate this line
        add_active_control(helper_line);

        for (const statement::ptr& stat: statement.then_statements) {
            if (!on_statement(stat)) return false;
        }

        // toggle helper line
        remove_active_control(helper_line);
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(helper_line);
        add_active_control(helper_line);

        for (const statement::ptr& stat: statement.else_statements) {
            if (!on_statement(stat)) return false;
        }

        // de-active helper line
        remove_active_control(helper_line);
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(helper_line);

        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::on_statement(const for_statement& statement) {
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
                    if (!on_statement(stat)) {
                        return false;
                    }
                }
            }
        }

        else if (from > to) {
            for (auto i = (int)from; i >= (int)to; i -= (int)step) {
                // adjust loop variable if necessary

                if (!loop_variable.empty()) {
                    loop_map[loop_variable] = i;
                }

                for (const auto& stat: statement.statements) {
                    if (!on_statement(stat)) {
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

    bool standardSyrecSynthesizerAdditionalLines::on_statement(const call_statement& statement) {
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
            if (!on_statement(stat)) return false;
        }
        modules.pop();

        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::on_statement(const uncall_statement& statement) {
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
            if (!on_statement(reverse_statement)) {
                return false;
            }
        }

        modules.pop();

        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::on_statement([[maybe_unused]] const skip_statement& statement) {
        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::on_expression(const expression::ptr& expression, std::vector<unsigned>& lines) {
        if (const auto* numExp = dynamic_cast<numeric_expression*>(expression.get())) {
            return on_expression(*numExp, lines);
        } else if (const auto* varExp = dynamic_cast<variable_expression*>(expression.get())) {
            return on_expression(*varExp, lines);
        } else if (const auto* binExp = dynamic_cast<binary_expression*>(expression.get())) {
            return on_expression(*binExp, lines);
        } else if (const auto* shiftExp = dynamic_cast<shift_expression*>(expression.get())) {
            return on_expression(*shiftExp, lines);
        } else {
            return false;
        }
    }

    bool standardSyrecSynthesizerAdditionalLines::on_expression(const numeric_expression& expression, std::vector<unsigned>& lines) {
        get_constant_lines(expression.bitwidth(), expression.value->evaluate(loop_map), lines);
        return true; //value of 'lines' is set
    }

    bool standardSyrecSynthesizerAdditionalLines::on_expression(const variable_expression& expression, std::vector<unsigned>& lines) {
        get_variables(expression.var, lines);
        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::on_expression(const binary_expression& expression, std::vector<unsigned>& lines) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;

        if (!on_expression(expression.lhs, lhs) || !on_expression(expression.rhs, rhs)) {
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

    bool standardSyrecSynthesizerAdditionalLines::on_expression(const shift_expression& expression, std::vector<unsigned>& lines) {
        std::vector<unsigned> lhs;

        if (!on_expression(expression.lhs, lhs)) {
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

    //**********************************************************************
    //*****                      Unary Operations                      *****
    //**********************************************************************

    bool standardSyrecSynthesizerAdditionalLines::bitwise_negation(const std::vector<unsigned>& dest) // see paper for synthesis
    {
        for (unsigned idx: dest) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(idx);
        }
        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::decrement(const std::vector<unsigned>& dest) {
        for (unsigned int i: dest) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(i);
            add_active_control(i);
        }

        for (unsigned int i: dest) {
            remove_active_control(i);
        }

        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::increment(const std::vector<unsigned>& dest) {
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

    bool standardSyrecSynthesizerAdditionalLines::bitwise_and(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        bool ok = true;
        for (unsigned i = 0u; i < dest.size(); ++i) {
            ok = ok && conjunction(dest.at(i), src1.at(i), src2.at(i));
        }
        return ok;
    }

    bool standardSyrecSynthesizerAdditionalLines::bitwise_cnot(const std::vector<unsigned>& dest, const std::vector<unsigned>& src) {
        for (unsigned i = 0u; i < src.size(); ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src.at(i), dest.at(i));
        }
        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::bitwise_or(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        bool ok = true;
        for (unsigned i = 0u; i < dest.size(); ++i) {
            ok = ok && disjunction(dest.at(i), src1.at(i), src2.at(i));
        }
        return ok;
    }

    bool standardSyrecSynthesizerAdditionalLines::conjunction(unsigned dest, unsigned src1, unsigned src2) {
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_toffoli(src1, src2, dest);

        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::decrease(const std::vector<unsigned>& dest, const std::vector<unsigned>& src) {
        for (unsigned i = 0u; i < src.size(); ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(dest.at(i));
        }

        increase(dest, src);

        for (unsigned i = 0u; i < src.size(); ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(dest.at(i));
        }

        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::decrease_with_carry(const std::vector<unsigned>& dest, const std::vector<unsigned>& src, unsigned carry) {
        for (unsigned i = 0u; i < src.size(); ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(dest.at(i));
        }

        increase_with_carry(dest, src, carry);

        for (unsigned i = 0u; i < src.size(); ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(dest.at(i));
        }

        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::disjunction(unsigned dest, unsigned src1, unsigned src2) {
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src1, dest);
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src2, dest);
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_toffoli(src1, src2, dest);

        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::division(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        // Computation of quotient and reminder (src1 is overwritten by reminder) [Code is identical with modulo]
        if (!modulo(dest, src1, src2)) return false;

        // Back computation of first source (thereby reminder is overwritten)

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
                for (std::size_t j = (src1.size() - i); j < src1.size(); ++j) {
                    remove_active_control(src2.at(j));
                }
                (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(src2.at(src1.size() - i));
                for (std::size_t j = (src1.size() + 1u - i); j < src1.size(); ++j) {
                    add_active_control(src2.at(j));
                }
            }
        }

        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
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

    bool standardSyrecSynthesizerAdditionalLines::greater_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if (!less_than(dest, src2, src1)) return false;
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(dest);

        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::greater_than(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        return less_than(dest, src2, src1);
    }

    bool standardSyrecSynthesizerAdditionalLines::increase(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs) {
        if (std::size_t bitwidth = rhs.size(); bitwidth == 1) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(lhs.at(0), rhs.at(0));
        } else {
            for (std::size_t i = 1; i <= bitwidth - 1; ++i) {
                (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(lhs.at(i), rhs.at(i));
            }
            for (std::size_t i = bitwidth - 2; i >= 1; --i) {
                (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(lhs.at(i), lhs.at(i + 1));
            }
            for (std::size_t i = 0; i <= bitwidth - 2; ++i) {
                (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_toffoli(rhs.at(i), lhs.at(i), lhs.at(i + 1));
            }

            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(lhs.at(bitwidth - 1), rhs.at(bitwidth - 1));

            for (std::size_t i = bitwidth - 2; i >= 1; --i) {
                (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_toffoli(lhs.at(i), rhs.at(i), lhs.at(i + 1));
                (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(lhs.at(i), rhs.at(i));
            }
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_toffoli(lhs.at(0), rhs.at(0), lhs.at(1));
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(lhs.at(0), rhs.at(0));

            for (std::size_t i = 1; i <= bitwidth - 2; ++i) {
                (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(lhs.at(i), lhs.at(i + 1));
            }
            for (std::size_t i = 1; i <= bitwidth - 1; ++i) {
                (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(lhs.at(i), rhs.at(i));
            }
        }

        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::increase_with_carry(const std::vector<unsigned>& dest, const std::vector<unsigned>& src, unsigned carry) {
        std::size_t bitwidth = src.size();

        if (bitwidth == 0) return true;

        for (std::size_t i = 1u; i < bitwidth; ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src.at(i), dest.at(i));
        }

        if (bitwidth > 1) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src.at(bitwidth - 1), carry);
        }
        for (int i = (int)bitwidth - 2; i > 0; --i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src.at(i), src.at(i + 1));
        }

        for (std::size_t i = 0u; i < bitwidth - 1; ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_toffoli(src.at(i), dest.at(i), src.at(i + 1));
        }
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_toffoli(src.at(bitwidth - 1), dest.at(bitwidth - 1), carry);

        for (int i = (int)bitwidth - 1; i > 0; --i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src.at(i), dest.at(i));
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_toffoli(dest.at(i - 1), src.at(i - 1), src.at(i));
        }

        for (std::size_t i = 1u; i < bitwidth - 1u; ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src.at(i), src.at(i + 1));
        }

        for (unsigned i = 0u; i < bitwidth; ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src.at(i), dest.at(i));
        }

        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::less_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if (!greater_than(dest, src2, src1)) return false;
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(dest);

        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::less_than(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        return (decrease_with_carry(src1, src2, dest) && increase(src1, src2));
    }

    bool standardSyrecSynthesizerAdditionalLines::modulo(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
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
                for (std::size_t j = (src1.size() - i); j < src1.size(); ++j) {
                    remove_active_control(src2.at(j));
                }
                (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(src2.at(src1.size() - i));
                for (std::size_t j = (src1.size() + 1u - i); j < src1.size(); ++j) {
                    add_active_control(src2.at(j));
                }
            }
        }
        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::multiplication(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if ((src1.empty()) || (dest.empty())) return true;

        std::vector<unsigned> sum     = dest;
        std::vector<unsigned> partial = src2;

        bool ok = true;

        add_active_control(src1.at(0));
        ok = ok && bitwise_cnot(sum, partial);
        remove_active_control(src1.at(0));

        for (std::size_t i = 1; i < dest.size(); ++i) {
            sum.erase(sum.begin());
            partial.pop_back();
            add_active_control(src1.at(i));
            ok = ok && increase(sum, partial);
            remove_active_control(src1.at(i));
        }

        return ok;
    }

    bool standardSyrecSynthesizerAdditionalLines::not_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if (!equals(dest, src1, src2)) return false;
        (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_not(dest);
        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::swap(const std::vector<unsigned>& dest1, const std::vector<unsigned>& dest2) {
        for (unsigned i = 0u; i < dest1.size(); ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_fredkin(dest1.at(i), dest2.at(i));
        }
        return true;
    }

    //**********************************************************************
    //*****                      Shift Operations                      *****
    //**********************************************************************

    bool standardSyrecSynthesizerAdditionalLines::left_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2) {
        for (std::size_t i = 0u; (i + src2) < dest.size(); ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src1.at(i), dest.at(i + src2));
        }
        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::right_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2) {
        for (std::size_t i = src2; i < dest.size(); ++i) {
            (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ)).append_cnot(src1.at(i), dest.at(i - src2));
        }
        return true;
    }

    //**********************************************************************
    //*****                     Efficient Controls                     *****
    //**********************************************************************

    bool standardSyrecSynthesizerAdditionalLines::add_active_control(unsigned control) {
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

        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::remove_active_control([[maybe_unused]] unsigned control) {
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
        return true;
    }

    bool standardSyrecSynthesizerAdditionalLines::assemble_circuit(const cct_node& current) {
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

    unsigned min(unsigned a, unsigned b, unsigned c) {
        unsigned tmp = (a < b) ? a : b;
        return ((tmp < c) ? tmp : c);
    }

    void standardSyrecSynthesizerAdditionalLines::get_variables(const variable_access::ptr& var, std::vector<unsigned>& lines) {
        unsigned offset = _var_lines[var->get_var()];
        if (!var->indexes.empty()) {
            // check if it is all numeric_expressions
            std::size_t n = var->get_var()->dimensions.size(); // dimensions
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
                for (auto i = (int)first; i >= (int)second; --i) {
                    lines.emplace_back(offset + i);
                }
            }
        } else {
            for (unsigned i = 0u; i < var->get_var()->bitwidth; ++i) {
                lines.emplace_back(offset + i);
            }
        }
    }

    unsigned standardSyrecSynthesizerAdditionalLines::get_constant_line(bool value) {
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

    void standardSyrecSynthesizerAdditionalLines::get_constant_lines(unsigned bitwidth, unsigned value, std::vector<unsigned>& lines) {
        boost::dynamic_bitset<> number(bitwidth, value);

        for (unsigned i = 0u; i < bitwidth; ++i) {
            lines.emplace_back(get_constant_line(number.test(i)));
        }
    }
    // helper function
    void standardSyrecSynthesizerAdditionalLines::add_variable(circuit& circ, const std::vector<unsigned>& dimensions, const variable::ptr& var,
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

    void standardSyrecSynthesizerAdditionalLines::add_variables(circuit& circ, const variable::vec& variables) {
        for (const auto& var: variables) {
            // entry in var lines map
            _var_lines.try_emplace(var, circ.get_lines());

            // types of constant and garbage
            constant _constant = (var->type == variable::out || var->type == variable::wire) ? constant(false) : constant();
            bool     _garbage  = (var->type == variable::in || var->type == variable::wire);

            add_variable(circ, var->dimensions, var, _constant, _garbage, std::string());
        }
    }

    bool synthesisAdditionalLines(circuit& circ, const program& program, const properties::ptr& settings, const properties::ptr& statistics) {
        // Settings parsing
        auto variable_name_format  = get<std::string>(settings, "variable_name_format", "%1$s%3$s.%2$d");
        auto main_module           = get<std::string>(settings, "main_module", std::string());
        auto statement_synthesizer = get<standardSyrecSynthesizerAdditionalLines>(settings, "statement_synthesizer", standardSyrecSynthesizerAdditionalLines(circ, program));

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

} // namespace syrec
