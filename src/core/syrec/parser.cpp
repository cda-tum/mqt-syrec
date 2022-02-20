/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2010  The RevKit Developers <revkit@informatik.uni-bremen.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "core/syrec/parser.hpp"

#include "core/syrec/expression.hpp"
#include "core/syrec/grammar.hpp"
#include "core/syrec/module.hpp"
#include "core/syrec/number.hpp"
#include "core/syrec/program.hpp"
#include "core/syrec/statement.hpp"
#include "core/syrec/variable.hpp"

#include <boost/assign/std/set.hpp>
#include <boost/assign/std/vector.hpp>
//#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <fstream>

//#define foreach_ BOOST_FOREACH

using namespace boost::assign;

namespace revkit {

    using namespace syrec;
    namespace bf = boost::fusion;

    struct parser_context {
        parser_context(const read_program_settings& settings):
            current_line_number(0u),
            settings(settings) {
        }

        ast_iterator                 begin;
        unsigned                     current_line_number;
        const read_program_settings& settings;
        std::string                  error_message;
        std::vector<std::string>     loop_variables;
    };

    number::ptr parse_number(const ast_number& ast_num, const module& proc, parser_context& context);

    struct parse_number_visitor: public boost::static_visitor<number::ptr> {
        explicit parse_number_visitor(const module& proc, parser_context& context):
            proc(proc),
            context(context) {}

        number::ptr operator()(unsigned value) const {
            return number::ptr(new number(value));
        }

        number::ptr operator()(const ast_variable& ast_var) const {
            variable::ptr var = proc.find_parameter_or_variable(ast_var.name);
            if (!var) {
                context.error_message = boost::str(boost::format("Unknown variable %s") % ast_var.name);
                return number::ptr();
            }

            return number::ptr(new number(var->bitwidth()));
        }

        number::ptr operator()(const std::string& loop_variable) const {
            if (boost::find(context.loop_variables, loop_variable) != context.loop_variables.end()) {
                return number::ptr(new number(loop_variable));
            } else {
                context.error_message = boost::str(boost::format("Unkown loop variable $%s") % loop_variable);
                return number::ptr();
            }
        }

        number::ptr operator()(const ast_number_expression& ast_ne) const {
            ast_number  ast_no1 = ast_ne.operand1;
            std::string ast_op  = ast_ne.op;
            ast_number  ast_no2 = ast_ne.operand2;

            unsigned op = 0u;
            if (ast_op == "+") {
                op = numeric_expression::add;
            } else if (ast_op == "-") {
                op = numeric_expression::subtract;
            } else if (ast_op == "*") {
                op = numeric_expression::multiply;
            } else if (ast_op == "/") {
                op = numeric_expression::divide;
            }

            number::ptr lhs = parse_number(ast_no1, proc, context);
            if (!lhs) return number::ptr();

            number::ptr rhs = parse_number(ast_no2, proc, context);
            if (!rhs) return number::ptr();

            if (lhs->is_constant() && rhs->is_constant()) {
                unsigned lhs_value = lhs->evaluate(number::loop_variable_mapping());
                unsigned rhs_value = rhs->evaluate(number::loop_variable_mapping());
                unsigned num_value;

                switch (op) {
                    case numeric_expression::add: // +
                    {
                        num_value = lhs_value + rhs_value;
                    } break;

                    case numeric_expression::subtract: // -
                    {
                        num_value = lhs_value - rhs_value;
                    } break;

                    case numeric_expression::multiply: // *
                    {
                        num_value = lhs_value * rhs_value;
                    } break;

                    case numeric_expression::divide: // /
                    {
                        num_value = lhs_value / rhs_value;
                    } break;

                    default:
                        return number::ptr();
                }

                return number::ptr(new number(num_value));
            }
            return number::ptr(new number(lhs, op, rhs));
        }

    private:
        const module&   proc;
        parser_context& context;
    };

    number::ptr parse_number(const ast_number& ast_num, const module& proc, parser_context& context) {
        return boost::apply_visitor(parse_number_visitor(proc, context), ast_num);
    }

    expression::ptr parse_expression(const ast_expression& ast_exp, const module& proc, unsigned bitwidth, parser_context& context);

    variable_access::ptr parse_variable_access(const ast_variable& ast_var, const module& proc, parser_context& context) {
        variable::ptr var = proc.find_parameter_or_variable(ast_var.name);

        if (!var) {
            context.error_message = boost::str(boost::format("Unknown variable %s") % ast_var.name);
            return variable_access::ptr();
        }

        variable_access::ptr va(new variable_access());
        va->set_var(var);

        boost::optional<std::pair<number::ptr, number::ptr>> var_range;

        ast_range range = ast_var.range;
        if (range) {
            number::ptr first = parse_number(bf::at_c<0>(*range), proc, context);
            if (!first) return variable_access::ptr();

            // is in range?
            if (!first->is_loop_variable()) {
                unsigned bound = first->evaluate(number::loop_variable_mapping());
                if (bound >= var->bitwidth()) {
                    context.error_message = boost::str(boost::format("Bound %d out of range in variable %s(%d)") % bound % var->name() % var->bitwidth());
                    return variable_access::ptr();
                }
            }

            number::ptr second = first;

            if (bf::at_c<1>(*range)) {
                second = parse_number(*bf::at_c<1>(*range), proc, context);
                if (!second) return variable_access::ptr();

                // is in range?
                if (!second->is_loop_variable()) {
                    unsigned bound = second->evaluate(number::loop_variable_mapping());
                    if (bound >= var->bitwidth()) {
                        context.error_message = boost::str(boost::format("Bound %d out of range in variable %s(%d)") % bound % var->name() % var->bitwidth());
                        return variable_access::ptr();
                    }
                }
            }

            var_range = std::make_pair(first, second);
        }

        va->set_range(var_range);

        // indexes
        if (var->dimensions().size() != ast_var.indexes.size()) {
            context.error_message = boost::str(boost::format("Invalid number of array indexes in variable %s. Expected %d, got %d") % var->name() % var->dimensions().size() % ast_var.indexes.size());
            return variable_access::ptr();
        }

        expression::vec indexes;
        for (const ast_expression& ast_exp: ast_var.indexes) {
            expression::ptr index = parse_expression(ast_exp, proc, var->bitwidth(), context);
            if (!index) return variable_access::ptr();
            indexes += index;
        }
        va->set_indexes(indexes);

        return va;
    }

    struct expression_visitor: public boost::static_visitor<expression*> {
        expression_visitor(const module& proc, unsigned bitwidth, parser_context& context):
            proc(proc),
            bitwidth(bitwidth),
            context(context) {}

        expression* operator()(const ast_number& ast_num) const {
            number::ptr num = parse_number(ast_num, proc, context);
            if (!num) return 0;
            return new numeric_expression(num, bitwidth);
        }

        expression* operator()(const ast_variable& ast_var) const {
            variable_access::ptr access = parse_variable_access(ast_var, proc, context);
            if (!access) return 0;
            return new variable_expression(access);
        }

        expression* operator()(const ast_binary_expression& ast_exp) const {
            ast_expression ast_exp1 = ast_exp.operand1;
            std::string    ast_op   = ast_exp.op;
            ast_expression ast_exp2 = ast_exp.operand2;

            unsigned op = 0u;
            if (ast_op == "+") {
                op = binary_expression::add;
            } else if (ast_op == "-") {
                op = binary_expression::subtract;
            } else if (ast_op == "^") {
                op = binary_expression::exor;
            } else if (ast_op == "*") {
                op = binary_expression::multiply;
            } else if (ast_op == "/") {
                op = binary_expression::divide;
            } else if (ast_op == "%") {
                op = binary_expression::modulo;
            } else if (ast_op == "*>") {
                op = binary_expression::frac_divide;
            } else if (ast_op == "&") {
                op = binary_expression::bitwise_and;
            } else if (ast_op == "|") {
                op = binary_expression::bitwise_or;
            } else if (ast_op == "&&") {
                op = binary_expression::logical_and;
            } else if (ast_op == "||") {
                op = binary_expression::logical_or;
            } else if (ast_op == "<") {
                op = binary_expression::less_than;
            } else if (ast_op == ">") {
                op = binary_expression::greater_than;
            } else if (ast_op == "=") {
                op = binary_expression::equals;
            } else if (ast_op == "!=") {
                op = binary_expression::not_equals;
            } else if (ast_op == "<=") {
                op = binary_expression::less_equals;
            } else if (ast_op == ">=") {
                op = binary_expression::greater_equals;
            }

            expression::ptr lhs = parse_expression(ast_exp1, proc, 0u, context);
            if (!lhs) return 0;

            expression::ptr rhs = parse_expression(ast_exp2, proc, lhs->bitwidth(), context);
            if (!rhs) return 0;

            if (numeric_expression* lhs_exp = dynamic_cast<numeric_expression*>(lhs.get())) {
                if (numeric_expression* rhs_exp = dynamic_cast<numeric_expression*>(rhs.get())) {
                    if (lhs_exp->value()->is_constant() && rhs_exp->value()->is_constant()) {
                        unsigned lhs_value = lhs_exp->value()->evaluate(number::loop_variable_mapping());
                        unsigned rhs_value = rhs_exp->value()->evaluate(number::loop_variable_mapping());
                        unsigned num_value;

                        switch (op) {
                            case binary_expression::add: // +
                            {
                                num_value = lhs_value + rhs_value;
                            } break;

                            case binary_expression::subtract: // -
                            {
                                num_value = lhs_value - rhs_value;
                            } break;

                            case binary_expression::exor: // ^
                            {
                                num_value = lhs_value ^ rhs_value;
                            } break;

                            case binary_expression::multiply: // *
                            {
                                num_value = lhs_value * rhs_value;
                            } break;

                            case binary_expression::divide: // /
                            {
                                num_value = lhs_value / rhs_value;
                            } break;

                            case binary_expression::modulo: // %
                            {
                                num_value = lhs_value % rhs_value;
                            } break;

                            case binary_expression::frac_divide: // *>
                            {
                                std::cerr << boost::format("Operator *> is undefined for numbers w/o specified bit width: ( %d *> %d )") % lhs_value % rhs_value << std::endl;
                                assert(false);
                                return 0;
                            } break;

                            case binary_expression::logical_and: // &&
                            {
                                num_value = lhs_value && rhs_value;
                            } break;

                            case binary_expression::logical_or: // ||
                            {
                                num_value = lhs_value || rhs_value;
                            } break;

                            case binary_expression::bitwise_and: // &
                            {
                                num_value = lhs_value & rhs_value;
                            } break;

                            case binary_expression::bitwise_or: // |
                            {
                                num_value = lhs_value | rhs_value;
                            } break;

                            case binary_expression::less_than: // <
                            {
                                num_value = lhs_value < rhs_value;
                            } break;

                            case binary_expression::greater_than: // >
                            {
                                num_value = lhs_value > rhs_value;
                            } break;

                            case binary_expression::equals: // =
                            {
                                num_value = lhs_value == rhs_value;
                            } break;

                            case binary_expression::not_equals: // !=
                            {
                                num_value = lhs_value != rhs_value;
                            } break;

                            case binary_expression::less_equals: // <=
                            {
                                num_value = lhs_value <= rhs_value;
                            } break;

                            case binary_expression::greater_equals: // >=
                            {
                                num_value = lhs_value >= rhs_value;
                            } break;

                            default:
                                std::cerr << "Invalid operator in binary expression" << std::endl;
                                assert(false);
                        }

                        return new numeric_expression(number::ptr(new number(num_value)), lhs->bitwidth());
                    }
                } else {
                    lhs_exp = new numeric_expression(lhs_exp->value(), rhs->bitwidth());
                    lhs     = expression::ptr(lhs_exp);
                }
            }
            return new binary_expression(lhs, op, rhs);
        }

        expression* operator()(const ast_unary_expression& ast_exp) const {
            std::string    ast_op   = ast_exp.op;
            ast_expression ast_expr = ast_exp.operand;

            unsigned op = 0u;
            if (ast_op == "!") {
                op = unary_expression::logical_not;
            } else if (ast_op == "~") {
                op = unary_expression::bitwise_not;
            }

            expression::ptr expr = parse_expression(ast_expr, proc, bitwidth, context);
            if (!expr) return 0;

            // double negative elimination
            if (unary_expression* sub_expr = dynamic_cast<unary_expression*>(expr.get())) {
                if (((op == unary_expression::bitwise_not) && (sub_expr->op() == unary_expression::bitwise_not)) || ((sub_expr->expr()->bitwidth() == 1u) && ((op == unary_expression::bitwise_not) || (op == unary_expression::logical_not)) && ((sub_expr->op() == unary_expression::bitwise_not) || (sub_expr->op() == unary_expression::logical_not)))) {
                    //TODO geht das auch einfacher?
                    if (numeric_expression* ep = dynamic_cast<numeric_expression*>(sub_expr->expr().get())) {
                        return new numeric_expression(ep->value(), ep->bitwidth());
                    }
                    if (variable_expression* ep = dynamic_cast<variable_expression*>(sub_expr->expr().get())) {
                        return new variable_expression(ep->var());
                    }
                    if (binary_expression* ep = dynamic_cast<binary_expression*>(sub_expr->expr().get())) {
                        return new binary_expression(ep->lhs(), ep->op(), ep->rhs());
                    }
                    if (unary_expression* ep = dynamic_cast<unary_expression*>(sub_expr->expr().get())) {
                        return new unary_expression(ep->op(), ep->expr());
                    }
                    if (shift_expression* ep = dynamic_cast<shift_expression*>(sub_expr->expr().get())) {
                        return new shift_expression(ep->lhs(), ep->op(), ep->rhs());
                    }
                }
            } else if (numeric_expression* sub_expr = dynamic_cast<numeric_expression*>(expr.get())) {
                if (op == unary_expression::logical_not) {
                    if (sub_expr->value()->is_constant()) {
                        unsigned value = (sub_expr->value()->evaluate(number::loop_variable_mapping()) == 0) ? 1 : 0;
                        return new numeric_expression(number::ptr(new number(value)), 1);
                    }
                } else if (op == unary_expression::bitwise_not) {
                    std::cerr << boost::format("Bitwise NOT is undefined for numbers w/o specified bit width: ~%s") % *expr << std::endl;
                    assert(false);
                    return 0;
                }
            }
            return new unary_expression(op, expr);
        }

        expression* operator()(const ast_shift_expression& ast_exp) const {
            ast_expression ast_exp1 = ast_exp.operand1;
            std::string    ast_op   = ast_exp.op;
            ast_number     ast_num  = ast_exp.operand2;

            unsigned op = 0u;
            if (ast_op == "<<") {
                op = shift_expression::left;
            } else if (ast_op == ">>") {
                op = shift_expression::right;
            }

            expression::ptr lhs = parse_expression(ast_exp1, proc, bitwidth, context);
            if (!lhs) return 0;

            number::ptr rhs = parse_number(ast_num, proc, context);
            if (!rhs) return 0;

            if (numeric_expression* lhs_no = dynamic_cast<numeric_expression*>(lhs.get())) {
                if (lhs_no->value()->is_constant() && rhs->is_constant()) {
                    unsigned value    = lhs_no->value()->evaluate(number::loop_variable_mapping());
                    unsigned shft_amt = rhs->evaluate(number::loop_variable_mapping());
                    unsigned result;

                    switch (op) {
                        case shift_expression::left: // <<
                        {
                            result = value << shft_amt;
                        } break;

                        case shift_expression::right: // >>
                        {
                            result = value >> shft_amt;
                        } break;

                        default:
                            std::cerr << "Invalid operator in shift expression" << std::endl;
                            assert(false);
                    }
                    return new numeric_expression(number::ptr(new number(result)), lhs->bitwidth());
                }
            }

            return new shift_expression(lhs, op, rhs);
        }

    private:
        const module&   proc;
        unsigned        bitwidth;
        parser_context& context;
    };

    // explain: bitwidth is the bitwidth to set in case ast_exp is a numeric expression
    expression::ptr parse_expression(const ast_expression& ast_exp, const module& proc, unsigned bitwidth, parser_context& context) {
        return expression::ptr(boost::apply_visitor(expression_visitor(proc, bitwidth, context), ast_exp));
    }

    statement::ptr parse_statement(const ast_statement& ast_stat, const program& prog, const module& proc, parser_context& context);

    struct statement_visitor: public boost::static_visitor<statement*> {
        statement_visitor(const program& prog, const module& proc, parser_context& context):
            prog(prog),
            proc(proc),
            context(context) {}

        statement* operator()(const ast_swap_statement& ast_swap_stat) const {
            const ast_variable& ast_var1 = bf::at_c<0>(ast_swap_stat);
            const ast_variable& ast_var2 = bf::at_c<1>(ast_swap_stat);

            variable_access::ptr va1 = parse_variable_access(ast_var1, proc, context);
            if (!va1) return 0;
            variable_access::ptr va2 = parse_variable_access(ast_var2, proc, context);
            if (!va2) return 0;

            if (va1->bitwidth() != va2->bitwidth()) {
                std::cerr << boost::format("Different bit-widths in <=> statement: %s (%d), %s (%d)") % va1->var()->name() % va1->bitwidth() % va2->var()->name() % va2->bitwidth() << std::endl;
                assert(false);
                return 0;
            }

            return new swap_statement(va1, va2);
        }

        statement* operator()(const ast_unary_statement& ast_unary_stat) const {
            const std::string&  ast_op  = bf::at_c<0>(ast_unary_stat);
            const ast_variable& ast_var = bf::at_c<1>(ast_unary_stat);

            variable_access::ptr var = parse_variable_access(ast_var, proc, context);
            if (!var) return 0;

            unsigned op = 0u;

            if (ast_op == "~") {
                op = unary_statement::invert;
            } else if (ast_op == "++") {
                op = unary_statement::increment;
            } else if (ast_op == "--") {
                op = unary_statement::decrement;
            }

            return new unary_statement(op, var);
        }

        statement* operator()(const ast_assign_statement& ast_assign_stat) const {
            const ast_variable&   ast_var = bf::at_c<0>(ast_assign_stat);
            char                  ast_op  = bf::at_c<1>(ast_assign_stat);
            const ast_expression& ast_exp = bf::at_c<2>(ast_assign_stat);

            variable_access::ptr lhs = parse_variable_access(ast_var, proc, context);
            if (!lhs) return 0;

            unsigned op = ast_op == '+' ? assign_statement::add :
                                          (ast_op == '-' ? assign_statement::subtract : assign_statement::exor);

            expression::ptr rhs = parse_expression(ast_exp, proc, lhs->bitwidth(), context);
            if (!rhs) return 0;

            if (lhs->bitwidth() != rhs->bitwidth()) {
                context.error_message = boost::str(boost::format("Wrong bit-width in assignment to %s") % lhs->var()->name());
                return 0;
            }

            return new assign_statement(lhs, op, rhs);
        }

        statement* operator()(const ast_if_statement& ast_if_stat) const {
            if_statement* if_stat = new if_statement();

            expression::ptr condition = parse_expression(ast_if_stat.condition, proc, 1u, context);
            if (!condition) return 0;
            if_stat->set_condition(condition);

            expression::ptr fi_condition = parse_expression(ast_if_stat.fi_condition, proc, 1u, context);
            if (!fi_condition) return 0;
            if_stat->set_fi_condition(fi_condition);

            for (const ast_statement& ast_stat: ast_if_stat.if_statement) {
                statement::ptr stat = parse_statement(ast_stat, prog, proc, context);
                if (!stat) return 0;
                if_stat->add_then_statement(stat);
            }

            for (const ast_statement& ast_stat: ast_if_stat.else_statement) {
                statement::ptr stat = parse_statement(ast_stat, prog, proc, context);
                if (!stat) return 0;
                if_stat->add_else_statement(stat);
            }

            return if_stat;
        }

        statement* operator()(const ast_for_statement& ast_for_stat) const {
            for_statement* for_stat = new for_statement();

            number::ptr from;
            number::ptr to = parse_number(ast_for_stat.to, proc, context);
            if (!to) return 0;

            std::string loop_variable;
            if (ast_for_stat.from) {
                from = parse_number(bf::at_c<1>(*ast_for_stat.from), proc, context);
                if (!from) return 0;

                // is there a loop variable?
                if (bf::at_c<0>(*ast_for_stat.from)) {
                    loop_variable = *bf::at_c<0>(*ast_for_stat.from);

                    // does the loop variable exist already?
                    if (boost::find(context.loop_variables, loop_variable) != context.loop_variables.end()) {
                        context.error_message = boost::str(boost::format("Redefinition of loop variable $%s") % loop_variable);
                        return 0;
                    }

                    for_stat->set_loop_variable(loop_variable);

                    context.loop_variables += loop_variable;
                }
            }

            for_stat->set_range(std::make_pair(from, to));

            // step
            if (ast_for_stat.step) {
                number::ptr step = parse_number(bf::at_c<1>(*ast_for_stat.step), proc, context);
                if (!step) return 0;

                for_stat->set_step(step);

                if (bf::at_c<0>(*ast_for_stat.step)) {
                    for_stat->set_negative_step(true);
                }
            }

            for (const ast_statement& ast_stat: ast_for_stat.do_statement) {
                statement::ptr stat = parse_statement(ast_stat, prog, proc, context);
                if (!stat) return 0;
                for_stat->add_statement(stat);
            }

            if (!loop_variable.empty()) {
                // release loop variable
                boost::remove_erase(context.loop_variables, loop_variable);
            }

            return for_stat;
        }

        statement* operator()(const ast_call_statement& ast_call_stat) const {
            std::string proc_name  = bf::at_c<1>(ast_call_stat);
            module::ptr other_proc = prog.find_module(proc_name);

            // found no module
            if (!other_proc.get()) {
                context.error_message = boost::str(boost::format("Unknown module %s") % proc_name);
                return 0;
            }

            const std::vector<std::string>& parameters = bf::at_c<2>(ast_call_stat);

            // wrong number of parameters
            if (parameters.size() != other_proc->parameters().size()) {
                context.error_message = boost::str(boost::format("Wrong number of arguments in (un)call of %s. Expected %d, got %d") % other_proc->name() % other_proc->parameters().size() % parameters.size());
                return 0;
            }

            // unknown variable name in parameters
            for (const std::string& parameter: parameters) {
                if (!proc.find_parameter_or_variable(parameter)) {
                    context.error_message = boost::str(boost::format("Unknown variable %s in (un)call of %s") % parameter % other_proc->name());
                    return 0;
                }
            }

            // check whether bit-width fits
            for (unsigned i = 0; i < parameters.size(); ++i) {
                variable::ptr vOther    = other_proc->parameters().at(i);
                variable::ptr parameter = proc.find_parameter_or_variable(parameters.at(i)); // must exist (see above)

                if (vOther->bitwidth() != parameter->bitwidth()) {
                    context.error_message = boost::str(boost::format("%d. parameter (%s) in (un)call of %s has bit-width of %d, but %d is required") % (i + 1) % parameters.at(i) % other_proc->name() % parameter->bitwidth() % vOther->bitwidth());
                    return 0;
                }
            }

            if (bf::at_c<0>(ast_call_stat) == "call") {
                return new call_statement(other_proc, parameters);
            } else {
                return new uncall_statement(other_proc, parameters);
            }
        }

        statement* operator()(const std::string& ast_skip_stat) const {
            return new skip_statement();
        }

    private:
        const program&  prog;
        const module&   proc;
        parser_context& context;
    };

    statement::ptr parse_statement(const ast_statement& ast_stat, const program& prog, const module& proc, parser_context& context) {
        if (statement* stat = boost::apply_visitor(statement_visitor(prog, proc, context), bf::at_c<1>(ast_stat))) {
            context.current_line_number = std::count(context.begin, bf::at_c<0>(ast_stat), '\n') + 1u;
            stat->set_line_number(context.current_line_number);
            return statement::ptr(stat);
        } else {
            return statement::ptr();
        }
    }

    unsigned parse_variable_type(const std::string& name) {
        if (name == "in") {
            return syrec::variable::in;
        } else if (name == "out") {
            return syrec::variable::out;
        } else if (name == "inout") {
            return syrec::variable::inout;
        } else if (name == "state") {
            return syrec::variable::state;
        } else if (name == "wire") {
            return syrec::variable::wire;
        }

        assert(false);
    }

    bool parse_module(syrec::module& proc, const ast_module& ast_proc, const syrec::program& prog, parser_context& context) {
        std::set<std::string> variable_names;

        for (const ast_parameter& ast_param: bf::at_c<1>(ast_proc)) {
            const std::string& variable_name = bf::at_c<0>(bf::at_c<1>(ast_param));

            if (boost::find(variable_names, variable_name) != variable_names.end()) {
                context.error_message = boost::str(boost::format("Redefinition of variable %s") % variable_name);
                return false;
            } else {
                variable_names += variable_name;
            }

            unsigned type = parse_variable_type(bf::at_c<0>(ast_param));
            proc.add_parameter(variable::ptr(
                    new variable(type,
                                 variable_name,
                                 bf::at_c<1>(bf::at_c<1>(ast_param)),
                                 bf::at_c<2>(bf::at_c<1>(ast_param)).get_value_or(context.settings.default_bitwidth))));
        }

        for (const ast_variable_declarations& ast_decls: bf::at_c<2>(ast_proc)) {
            unsigned type = parse_variable_type(bf::at_c<0>(ast_decls));

            for (const ast_variable_declaration& ast_decl: bf::at_c<1>(ast_decls)) {
                const std::string& variable_name = bf::at_c<0>(ast_decl);

                if (boost::find(variable_names, variable_name) != variable_names.end()) {
                    context.error_message = boost::str(boost::format("Redefinition of variable %s") % variable_name);
                    return false;
                } else {
                    variable_names += variable_name;
                }

                proc.add_variable(variable::ptr(
                        new variable(type,
                                     variable_name,
                                     bf::at_c<1>(ast_decl),
                                     bf::at_c<2>(ast_decl).get_value_or(context.settings.default_bitwidth))));
            }
        }

        for (const ast_statement& ast_stat: bf::at_c<3>(ast_proc)) {
            statement::ptr stat = parse_statement(ast_stat, prog, proc, context);
            if (!stat) return false;
            proc.add_statement(stat);
        }

        return true;
    }

    bool read_program_from_string(syrec::program& prog, const std::string& content, const read_program_settings& settings, std::string* error) {
        ast_program ast_prog;
        if (!parse_string(ast_prog, content)) {
            return false;
        }

        parser_context context(settings);
        context.begin = content.begin();

        // Modules
        for (const ast_module& ast_proc: ast_prog) {
            module::ptr proc(new module(bf::at_c<0>(ast_proc)));
            if (!parse_module(*proc, ast_proc, prog, context)) {
                if (error) {
                    *error = boost::str(boost::format("In line %d: %s") % context.current_line_number % context.error_message);
                }
                return false;
            }
            prog.add_module(proc);
        }

        return true;
    }

    read_program_settings::read_program_settings():
        default_bitwidth(32u) {
    }

    bool read_program(syrec::program& prog, const std::string& filename, const read_program_settings& settings, std::string* error) {
        std::string content, line;

        std::ifstream is;
        is.open(filename.c_str(), std::ios::in);

        while (getline(is, line)) {
            content += line + '\n';
        }

        return read_program_from_string(prog, content, settings, error);
    }

} // namespace revkit
