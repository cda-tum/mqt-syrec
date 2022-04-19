#include "core/syrec/parser.hpp"

#include <fstream>
#include <memory>
#include <optional>

namespace syrec {

    number::ptr parse_number(const ast_number& ast_num, const module& proc, parser_context& context);

    struct parse_number_visitor {
        explicit parse_number_visitor(const module& proc, parser_context& context):
            proc(proc),
            context(context) {}

        number::ptr operator()(unsigned value) const {
            return std::make_shared<number>(value);
        }

        number::ptr operator()(const boost::recursive_wrapper<ast_variable>& ast_var) const {
            variable::ptr var = proc.find_parameter_or_variable(ast_var.get().name);
            if (!var) {
                context.error_message = "Unknown variable " + ast_var.get().name;
                return {};
            }

            return std::make_shared<number>(var->bitwidth);
        }

        number::ptr operator()(const std::string& loop_variable) const {
            if (std::find(context.loop_variables.begin(), context.loop_variables.end(), loop_variable) != context.loop_variables.end()) {
                return std::make_shared<number>(loop_variable);
            } else {
                context.error_message = "Unkown loop variable $" + loop_variable;
                return {};
            }
        }

        number::ptr operator()(const boost::recursive_wrapper<ast_number_expression>& ast_ne) const {
            ast_number  ast_no1 = ast_ne.get().operand1;
            std::string ast_op  = ast_ne.get().op;
            ast_number  ast_no2 = ast_ne.get().operand2;

            unsigned op = 0u;
            if (ast_op == "+") {
                op = numeric_expression::add;
            } else if (ast_op == "-") {
                op = numeric_expression::subtract;
            } else if (ast_op == "*") {
                op = numeric_expression::multiply;
            } else if (ast_op == "/") {
                op = numeric_expression::divide;
            } else if (ast_op == "%") {
                op = numeric_expression::modulo;
            } else if (ast_op == "&&") {
                op = numeric_expression::logical_and;
            }

            else if (ast_op == "||") {
                op = numeric_expression::logical_or;
            } else if (ast_op == "&") {
                op = numeric_expression::bitwise_and;
            } else if (ast_op == "|") {
                op = numeric_expression::bitwise_or;
            } else if (ast_op == ">") {
                op = numeric_expression::greater_than;
            } else if (ast_op == "<") {
                op = numeric_expression::less_than;
            } else if (ast_op == ">=") {
                op = numeric_expression::greater_equals;
            } else if (ast_op == "<=") {
                op = numeric_expression::less_equals;
            } else if (ast_op == "==") {
                op = numeric_expression::equals;
            } else if (ast_op == "!=") {
                op = numeric_expression::not_equals;
            }

            number::ptr lhs = parse_number(ast_no1, proc, context);
            if (!lhs) return {};

            number::ptr rhs = parse_number(ast_no2, proc, context);
            if (!rhs) return {};
            unsigned num = 0;
            if (lhs->is_constant() && rhs->is_constant()) {
                unsigned lhs_value = lhs->evaluate(number::loop_variable_mapping());
                unsigned rhs_value = rhs->evaluate(number::loop_variable_mapping());
                unsigned num_value = 0;

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

                    case numeric_expression::modulo: // /
                    {
                        num_value = lhs_value % rhs_value;
                    } break;

                    case numeric_expression::logical_and: // /
                    {
                        num_value = lhs_value && rhs_value;
                    } break;

                    case numeric_expression::logical_or: // /
                    {
                        num_value = lhs_value || rhs_value;
                    } break;

                    case numeric_expression::bitwise_and: // /
                    {
                        num_value = lhs_value & rhs_value;
                    } break;

                    case numeric_expression::bitwise_or: // /
                    {
                        num_value = lhs_value | rhs_value;
                    } break;

                    case numeric_expression::less_than: // /
                    {
                        num_value = lhs_value < rhs_value;
                    } break;

                    case numeric_expression::greater_than: // /
                    {
                        num_value = lhs_value > rhs_value;
                    } break;

                    case numeric_expression::greater_equals: // /
                    {
                        num_value = lhs_value >= rhs_value;
                    } break;

                    case numeric_expression::less_equals: // /
                    {
                        num_value = lhs_value <= rhs_value;
                    } break;

                    case numeric_expression::equals: // /
                    {
                        num_value = lhs_value == rhs_value;
                    } break;

                    case numeric_expression::not_equals: // /
                    {
                        num_value = lhs_value != rhs_value;
                    } break;

                    default:
                        return {};
                }

                return std::make_shared<number>(num_value);
            }
            return std::make_shared<number>(num); //return std::make_shared<number>(lhs, op, rhs);
        }

    private:
        const module&   proc;
        parser_context& context;
    };

    number::ptr parse_number(const ast_number& ast_num, const module& proc, parser_context& context) {
        return std::visit(parse_number_visitor(proc, context), ast_num);
    }

    expression::ptr parse_expression(const ast_expression& ast_exp, const module& proc, unsigned bitwidth, parser_context& context);

    variable_access::ptr parse_variable_access(const ast_variable& ast_var, const module& proc, parser_context& context) {
        variable::ptr var = proc.find_parameter_or_variable(ast_var.name);

        if (!var) {
            context.error_message = "Unknown variable %s" + ast_var.name;
            return {};
        }

        variable_access::ptr va(new variable_access());
        va->set_var(var);

        std::optional<std::pair<number::ptr, number::ptr>> var_range;

        ast_range range = ast_var.range;
        if (range) {
            number::ptr first = parse_number(bf::at_c<0>(*range), proc, context);
            if (!first) return {};

            // is in range?
            if (!first->is_loop_variable()) {
                unsigned bound = first->evaluate(number::loop_variable_mapping());
                if (bound >= var->bitwidth) {
                    context.error_message = "Bound " + std::to_string(bound) + " out of range in variable " + var->name + "(" + std::to_string(var->bitwidth) + ")";
                    return {};
                }
            }

            number::ptr second = first;

            if (bf::at_c<1>(*range)) {
                second = parse_number(*bf::at_c<1>(*range), proc, context);
                if (!second) return {};

                // is in range?
                if (!second->is_loop_variable()) {
                    unsigned bound = second->evaluate(number::loop_variable_mapping());
                    if (bound >= var->bitwidth) {
                        context.error_message = "Bound " + std::to_string(bound) + " out of range in variable " + var->name + "(" + std::to_string(var->bitwidth) + ")";
                        return {};
                    }
                }
            }

            var_range = std::make_pair(first, second);
        }

        va->range = var_range;

        // indexes
        if (var->dimensions.size() != ast_var.indexes.size()) {
            context.error_message = "Invalid number of array indexes in variable " + var->name + ". Expected " + std::to_string(var->dimensions.size()) + ", got " + std::to_string(ast_var.indexes.size());
            return {};
        }

        expression::vec indexes;
        for (const ast_expression& ast_exp: ast_var.indexes) {
            expression::ptr index = parse_expression(ast_exp, proc, var->bitwidth, context);
            if (!index) return {};
            indexes.emplace_back(index);
        }
        va->indexes = indexes;

        return va;
    }

    struct expression_visitor {
        expression_visitor(const module& proc, unsigned bitwidth, parser_context& context):
            proc(proc),
            bitwidth(bitwidth),
            context(context) {}

        expression* operator()(const ast_number& ast_num) const {
            number::ptr num = parse_number(ast_num, proc, context);
            if (!num) return nullptr;
            return new numeric_expression(num, bitwidth);
        }

        expression* operator()(const ast_variable& ast_var) const {
            variable_access::ptr access = parse_variable_access(ast_var, proc, context);
            if (!access) return nullptr;
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
            if (!lhs) return nullptr;

            expression::ptr rhs = parse_expression(ast_exp2, proc, lhs->bitwidth(), context);
            if (!rhs) return nullptr;

            /*if (auto* lhs_exp = dynamic_cast<numeric_expression*>(lhs.get())) {
                if (auto* rhs_exp = dynamic_cast<numeric_expression*>(rhs.get())) {
                    if (lhs_exp->value()->is_constant() && rhs_exp->value()->is_constant()) {
                        unsigned lhs_value = lhs_exp->value()->evaluate(number::loop_variable_mapping());
                        unsigned rhs_value = rhs_exp->value()->evaluate(number::loop_variable_mapping());
                        unsigned num_value = 0;

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
                                std::cerr << "Operator *> is undefined for numbers w/o specified bit width: ( " + std::to_string(lhs_value) + " *> " + std::to_string(rhs_value) + " )" << std::endl;
                                assert(false);
                                return nullptr;
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

                        return new numeric_expression(std::make_shared<number>(num_value), lhs->bitwidth());
                    }
                } else {
                    lhs_exp = new numeric_expression(lhs_exp->value(), rhs->bitwidth());
                    lhs     = expression::ptr(lhs_exp);
                }
            }*/
            return new binary_expression(lhs, op, rhs);
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
            if (!lhs) return nullptr;

            number::ptr rhs = parse_number(ast_num, proc, context);
            if (!rhs) return nullptr;

            if (auto* lhs_no = dynamic_cast<numeric_expression*>(lhs.get())) {
                if (lhs_no->value->is_constant() && rhs->is_constant()) {
                    unsigned value    = lhs_no->value->evaluate(number::loop_variable_mapping());
                    unsigned shft_amt = rhs->evaluate(number::loop_variable_mapping());
                    unsigned result   = 0;

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
                    return new numeric_expression(std::make_shared<number>(result), lhs->bitwidth());
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

    struct statement_visitor {
        statement_visitor(const program& prog, const module& proc, parser_context& context):
            prog(prog),
            proc(proc),
            context(context) {}

        statement* operator()(const ast_swap_statement& ast_swap_stat) const {
            const ast_variable& ast_var1 = bf::at_c<0>(ast_swap_stat);
            const ast_variable& ast_var2 = bf::at_c<1>(ast_swap_stat);

            variable_access::ptr va1 = parse_variable_access(ast_var1, proc, context);
            if (!va1) return nullptr;
            variable_access::ptr va2 = parse_variable_access(ast_var2, proc, context);
            if (!va2) return nullptr;

            if (va1->bitwidth() != va2->bitwidth()) {
                std::cerr << "Different bit-widths in <=> statement: " + va1->get_var()->name + " (" + std::to_string(va1->bitwidth()) + "), " + va2->get_var()->name + " (" + std::to_string(va2->bitwidth()) + ")" << std::endl;
                assert(false);
                return nullptr;
            }

            return new swap_statement(va1, va2);
        }

        statement* operator()(const ast_unary_statement& ast_unary_stat) const {
            const std::string&  ast_op  = bf::at_c<0>(ast_unary_stat);
            const ast_variable& ast_var = bf::at_c<1>(ast_unary_stat);

            variable_access::ptr var = parse_variable_access(ast_var, proc, context);
            if (!var) return nullptr;

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
            if (!lhs) return nullptr;

            unsigned op = ast_op == '+' ? assign_statement::add :
                                          (ast_op == '-' ? assign_statement::subtract : assign_statement::exor);

            expression::ptr rhs = parse_expression(ast_exp, proc, lhs->bitwidth(), context);
            if (!rhs) return nullptr;

            if (lhs->bitwidth() != rhs->bitwidth()) {
                context.error_message = "Wrong bit-width in assignment to " + lhs->get_var()->name;
                return nullptr;
            }

            return new assign_statement(lhs, op, rhs);
        }

        statement* operator()(const ast_if_statement& ast_if_stat) const {
            auto* if_stat = new if_statement();

            expression::ptr condition = parse_expression(ast_if_stat.condition, proc, 1u, context);
            if (!condition) return nullptr;
            if_stat->set_condition(condition);

            expression::ptr fi_condition = parse_expression(ast_if_stat.fi_condition, proc, 1u, context);
            if (!fi_condition) return nullptr;
            if_stat->set_fi_condition(fi_condition);

            for (const ast_statement& ast_stat: ast_if_stat.if_statement) {
                statement::ptr stat = parse_statement(ast_stat, prog, proc, context);
                if (!stat) return nullptr;
                if_stat->add_then_statement(stat);
            }

            for (const ast_statement& ast_stat: ast_if_stat.else_statement) {
                statement::ptr stat = parse_statement(ast_stat, prog, proc, context);
                if (!stat) return nullptr;
                if_stat->add_else_statement(stat);
            }

            return if_stat;
        }

        statement* operator()(const ast_for_statement& ast_for_stat) const {
            auto* for_stat = new for_statement();

            number::ptr from;
            number::ptr to = parse_number(ast_for_stat.to, proc, context);
            if (!to) return nullptr;

            std::string loop_variable;
            if (ast_for_stat.from) {
                from = parse_number(bf::at_c<1>(*ast_for_stat.from), proc, context);
                if (!from) return nullptr;

                // is there a loop variable?
                if (bf::at_c<0>(*ast_for_stat.from)) {
                    loop_variable = *bf::at_c<0>(*ast_for_stat.from);

                    // does the loop variable exist already?
                    if (std::find(context.loop_variables.begin(), context.loop_variables.end(), loop_variable) != context.loop_variables.end()) {
                        context.error_message = "Redefinition of loop variable $" + loop_variable;
                        return nullptr;
                    }

                    for_stat->loop_variable = loop_variable;

                    context.loop_variables.emplace_back(loop_variable);
                }
            }

            for_stat->range = std::make_pair(from, to);

            // step
            /*if (ast_for_stat.step) {
                number::ptr step = parse_number(bf::at_c<1>(*ast_for_stat.step), proc, context);
                if (!step) return nullptr;

                for_stat->set_step(step);

                if (bf::at_c<0>(*ast_for_stat.step)) {
                    for_stat->set_negative_step(true);
                }
            }*/

            for (const ast_statement& ast_stat: ast_for_stat.do_statement) {
                statement::ptr stat = parse_statement(ast_stat, prog, proc, context);
                if (!stat) return nullptr;
                for_stat->add_statement(stat);
            }

            if (!loop_variable.empty()) {
                // release loop variable
                context.loop_variables.erase(std::remove_if(context.loop_variables.begin(), context.loop_variables.end(), [&](const auto& s) { return s == loop_variable; }), context.loop_variables.end());
            }

            return for_stat;
        }

        statement* operator()(const ast_call_statement& ast_call_stat) const {
            std::string proc_name  = bf::at_c<1>(ast_call_stat);
            module::ptr other_proc = prog.find_module(proc_name);

            // found no module
            if (!other_proc.get()) {
                context.error_message = "Unknown module " + proc_name;
                return nullptr;
            }

            const std::vector<std::string>& parameters = bf::at_c<2>(ast_call_stat);

            // wrong number of parameters
            if (parameters.size() != other_proc->parameters.size()) {
                context.error_message = "Wrong number of arguments in (un)call of " + other_proc->name + ". Expected " + std::to_string(other_proc->parameters.size()) + ", got " + std::to_string(parameters.size());
                return nullptr;
            }

            // unknown variable name in parameters
            for (const std::string& parameter: parameters) {
                if (!proc.find_parameter_or_variable(parameter)) {
                    context.error_message = "Unknown variable " + parameter + " in (un)call of " + other_proc->name;
                    return nullptr;
                }
            }

            // check whether bit-width fits
            for (unsigned i = 0; i < parameters.size(); ++i) {
                variable::ptr vOther    = other_proc->parameters.at(i);
                variable::ptr parameter = proc.find_parameter_or_variable(parameters.at(i)); // must exist (see above)

                if (vOther->bitwidth != parameter->bitwidth) {
                    context.error_message = std::to_string(i + 1) + ". parameter (" + parameters.at(i) + ") in (un)call of " + other_proc->name + " has bit-width of " + std::to_string(parameter->bitwidth) + ", but " + std::to_string(vOther->bitwidth) + " is required";
                    return nullptr;
                }
            }

            if (bf::at_c<0>(ast_call_stat) == "call") {
                return new call_statement(other_proc, parameters);
            } else {
                return new uncall_statement(other_proc, parameters);
            }
        }

        statement* operator()(const std::string& ast_skip_stat [[maybe_unused]]) const {
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
            stat->line_number           = context.current_line_number;
            return statement::ptr(stat);
        } else {
            return {};
        }
    }

    unsigned parse_variable_type(const std::string& name) {
        if (name == "in") {
            return variable::in;
        } else if (name == "out") {
            return variable::out;
        } else if (name == "inout") {
            return variable::inout;
        } else if (name == "state") {
            return variable::state;
        } else if (name == "wire") {
            return variable::wire;
        }

        assert(false);
        return 0u;
    }

    bool parse_module(module& proc, const ast_module& ast_proc, const program& prog, parser_context& context) {
        std::set<std::string> variable_names;

        for (const ast_parameter& ast_param: bf::at_c<1>(ast_proc)) {
            const std::string& variable_name = bf::at_c<0>(bf::at_c<1>(ast_param));

            if (variable_names.find(variable_name) != variable_names.end()) {
                context.error_message = "Redefinition of variable " + variable_name;
                return false;
            } else {
                variable_names.emplace(variable_name);
            }

            unsigned type = parse_variable_type(bf::at_c<0>(ast_param));
            proc.add_parameter(std::make_shared<variable>(
                    type,
                    variable_name,
                    bf::at_c<1>(bf::at_c<1>(ast_param)),
                    bf::at_c<2>(bf::at_c<1>(ast_param)).get_value_or(context.settings.default_bitwidth)));
        }

        for (const ast_statement& ast_stat: bf::at_c<3>(ast_proc)) {
            statement::ptr stat = parse_statement(ast_stat, prog, proc, context);
            if (!stat) return false;
            proc.add_statement(stat);
        }

        return true;
    }

    bool program::read_program_from_string(const std::string& content, const read_program_settings& settings, std::string* error) {
        ast_program ast_prog;
        if (!parse_string(ast_prog, content)) {
            *error = "PARSE_STRING_FAILED";
            return false;
        }

        parser_context context(settings);
        context.begin = content.begin();

        // Modules
        for (const ast_module& ast_proc: ast_prog) {
            module::ptr proc(new module(bf::at_c<0>(ast_proc)));
            if (!parse_module(*proc, ast_proc, *this, context)) {
                if (error != nullptr) {
                    *error = "In line " + std::to_string(context.current_line_number) + ": " + context.error_message;
                }
                return false;
            }
            add_module(proc);
        }

        return true;
    }
} // namespace syrec
