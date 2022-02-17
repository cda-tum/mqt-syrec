/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2011  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include "core/syrec/write_systemc.hpp"

#include <boost/algorithm/string/erase.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <core/pattern.hpp>
#include <core/syrec/program.hpp>
#include <list>
#include <string>

#define foreach_ BOOST_FOREACH

static const std::string TAB = "  ";

namespace revkit {

    class systemc_writer {
    public:
        systemc_writer(syrec::module::ptr mmod, const pattern& p);
        void program_to_systemc(const syrec::program& prog, std::ostream& os);

    private:
        std::string make_array_loop(const std::string& body, const std::vector<unsigned>& dimensions, unsigned level);
        std::string number_to_systemc(const syrec::number& number);
        std::string variable_access_to_systemc(const syrec::variable_access& va);
        std::string expression_to_systemc(const syrec::variable_expression& expr);
        std::string expression_to_systemc(const syrec::binary_expression& expr);
        std::string expression_to_systemc(const syrec::shift_expression& expr);
        std::string expression_to_systemc(syrec::expression::ptr expression);
        std::string statement_to_systemc(const syrec::swap_statement& statement, unsigned level);
        std::string statement_to_systemc(const syrec::unary_statement& statement, unsigned level);
        std::string statement_to_systemc(const syrec::assign_statement& statement, unsigned level);
        std::string statement_to_systemc(const syrec::if_statement& statement, unsigned level);
        std::string statement_to_systemc(const syrec::for_statement& statement, unsigned level);
        std::string statement_to_systemc(const syrec::call_statement& statement, unsigned level);
        std::string statement_to_systemc(const syrec::uncall_statement& statement, unsigned level);
        std::string statement_to_systemc(const syrec::skip_statement& statement);
        std::string statement_to_systemc(syrec::statement::ptr statement, unsigned level);
        std::string module_to_systemc(syrec::module::ptr mod);

        int                                tempcount;
        std::list<syrec::call_statement>   calls;
        std::list<syrec::uncall_statement> uncalls;
        syrec::module::ptr                 main_mod;
        syrec::module::ptr                 mod;
        const pattern&                     p;
    };

    systemc_writer::systemc_writer(syrec::module::ptr mmod, const pattern& pat):
        tempcount(0),
        main_mod(mmod),
        p(pat) {
    }

    std::string systemc_writer::make_array_loop(const std::string& body, const std::vector<unsigned>& dimensions, unsigned level) {
        std::string loop_str = "";
        for (unsigned i = 0; i < dimensions.size(); i++) {
            for (unsigned j = 0; j < i + level; j++) {
                loop_str += TAB;
            }
            loop_str += "for(unsigned i";
            loop_str += boost::lexical_cast<std::string>(i);
            loop_str += " = 0; i";
            loop_str += boost::lexical_cast<std::string>(i);
            loop_str += " < ";
            loop_str += boost::lexical_cast<std::string>(dimensions.at(i));
            loop_str += "; i";
            loop_str += boost::lexical_cast<std::string>(i);
            loop_str += "++)\n";
            for (unsigned j = 0; j < i + level; j++) {
                loop_str += TAB;
            }
            loop_str += "{\n";
        }
        for (unsigned i = 0; i < dimensions.size() + level; i++) {
            loop_str += TAB;
        }
        loop_str += body;
        for (unsigned i = dimensions.size(); i > 0; i--) {
            for (unsigned j = 0; j < level + i - 1; j++) {
                loop_str += TAB;
            }
            loop_str += "}\n";
        }
        return loop_str;
    }

    std::string systemc_writer::number_to_systemc(const syrec::number& number) {
        std::string number_str = "";
        if (number.is_loop_variable()) {
            // wird mit variable_access auf Schleifenvariable zugegriffen?
            number_str += "$";
            number_str += number.variable_name();
        } else {
            number_str += boost::lexical_cast<std::string>(number.evaluate(std::map<std::string, unsigned>()));
        }
        return number_str;
    }

    std::string systemc_writer::variable_access_to_systemc(const syrec::variable_access& va) {
        std::string va_str = "_";
        va_str += va.var()->name();
        foreach_(std::shared_ptr<syrec::expression> index_expr, va.indexes()) {
            va_str += "[";
            va_str += expression_to_systemc(index_expr);
            va_str += "]";
        }
        const boost::optional<std::pair<syrec::number::ptr, syrec::number::ptr>>& range = va.range();
        if (range) {
            std::string r_first  = number_to_systemc(*range->first);
            std::string r_second = number_to_systemc(*range->second);
            va_str += ".range(";
            va_str += r_second;
            va_str += ",";
            va_str += r_first;
            va_str += ")";
        }
        return va_str;
    }

    std::string systemc_writer::expression_to_systemc(const syrec::variable_expression& expr) {
        return variable_access_to_systemc(*expr.var());
    }

    std::string systemc_writer::expression_to_systemc(const syrec::binary_expression& expr) {
        std::string expr_str = "( ";
        expr_str += expression_to_systemc(expr.lhs());

        if (expr.op() == syrec::binary_expression::add) {
            expr_str += " + ";
        } else if (expr.op() == syrec::binary_expression::subtract) {
            expr_str += " - ";
        } else if (expr.op() == syrec::binary_expression::exor) {
            expr_str += " ^ ";
        } else if (expr.op() == syrec::binary_expression::multiply) {
            expr_str += " * ";
        } else if (expr.op() == syrec::binary_expression::divide) {
            expr_str += " / ";
        } else if (expr.op() == syrec::binary_expression::modulo) {
            expr_str += " % ";
        } else if (expr.op() == syrec::binary_expression::frac_divide) {
            expr_str = "( ( ";
            expr_str += expression_to_systemc(expr.lhs());
            expr_str += " * ";
            expr_str += expression_to_systemc(expr.rhs());
            expr_str += " ) >> ";
            expr_str += expr.lhs()->bitwidth();
            expr_str += " )";
            return expr_str;
        } else if (expr.op() == syrec::binary_expression::logical_and) {
            expr_str += " && ";
        } else if (expr.op() == syrec::binary_expression::logical_or) {
            expr_str += " || ";
        } else if (expr.op() == syrec::binary_expression::bitwise_and) {
            expr_str += " & ";
        } else if (expr.op() == syrec::binary_expression::bitwise_or) {
            expr_str += " | ";
        } else if (expr.op() == syrec::binary_expression::less_than) {
            expr_str += " < ";
        } else if (expr.op() == syrec::binary_expression::greater_than) {
            expr_str += " > ";
        } else if (expr.op() == syrec::binary_expression::equals) {
            expr_str += " == ";
        } else if (expr.op() == syrec::binary_expression::not_equals) {
            expr_str += " != ";
        } else if (expr.op() == syrec::binary_expression::less_equals) {
            expr_str += " <= ";
        } else if (expr.op() == syrec::binary_expression::greater_equals) {
            expr_str += " >= ";
        }

        expr_str += expression_to_systemc(expr.rhs());
        expr_str += " )";

        return expr_str;
    }

    std::string systemc_writer::expression_to_systemc(const syrec::shift_expression& expr) {
        std::string expr_str = "( ";
        expr_str += expression_to_systemc(expr.lhs());

        if (expr.op() == syrec::shift_expression::left) {
            expr_str += " << ";
        } else if (expr.op() == syrec::shift_expression::right) {
            expr_str += " >> ";
        }

        expr_str += number_to_systemc(*expr.rhs());
        expr_str += " )";

        return expr_str;
    }

    std::string systemc_writer::expression_to_systemc(syrec::expression::ptr expression) {
        if (syrec::numeric_expression* expr = dynamic_cast<syrec::numeric_expression*>(expression.get())) {
            return number_to_systemc(*expr->value());
        } else if (syrec::variable_expression* expr = dynamic_cast<syrec::variable_expression*>(expression.get())) {
            return expression_to_systemc(*expr);
        } else if (syrec::binary_expression* expr = dynamic_cast<syrec::binary_expression*>(expression.get())) {
            return expression_to_systemc(*expr);
        } else if (syrec::shift_expression* expr = dynamic_cast<syrec::shift_expression*>(expression.get())) {
            return expression_to_systemc(*expr);
        } else {
            return "";
        }
    }

    std::string systemc_writer::statement_to_systemc(const syrec::swap_statement& statement, unsigned level) {
        std::string swapstmt_str;
        for (unsigned i = 0; i < level; ++i) {
            swapstmt_str += TAB;
        }
        if (statement.lhs()->bitwidth() <= 64) {
            swapstmt_str += "sc_uint<";
        } else {
            swapstmt_str += "sc_biguint<";
        }
        swapstmt_str += boost::lexical_cast<std::string>(statement.lhs()->bitwidth()) + "> temp" + boost::lexical_cast<std::string>(tempcount) + " = ";
        swapstmt_str += variable_access_to_systemc(*statement.lhs()) + ";\n";
        for (unsigned i = 0; i < level; ++i) {
            swapstmt_str += TAB;
        }
        swapstmt_str += variable_access_to_systemc(*statement.lhs()) + " = " + variable_access_to_systemc(*statement.rhs()) + ";\n";
        for (unsigned i = 0; i < level; ++i) {
            swapstmt_str += TAB;
        }
        swapstmt_str += variable_access_to_systemc(*statement.rhs()) + " = temp" + boost::lexical_cast<std::string>(tempcount) + ";\n";
        tempcount++;
        return swapstmt_str;
    }

    std::string systemc_writer::statement_to_systemc(const syrec::unary_statement& statement, unsigned level) {
        std::string unarystmt_str = "";
        for (unsigned i = 0; i < level; ++i) {
            unarystmt_str += TAB;
        }
        if (statement.op() == 0) {
            unarystmt_str += "~" + variable_access_to_systemc(*statement.var());
        } else if (statement.op() == 1) {
            unarystmt_str += variable_access_to_systemc(*statement.var()) + "++";
        } else if (statement.op() == 2) {
            unarystmt_str += variable_access_to_systemc(*statement.var()) + "--";
        }
        unarystmt_str += ";\n";
        return unarystmt_str;
    }

    std::string systemc_writer::statement_to_systemc(const syrec::assign_statement& statement, unsigned level) {
        std::string assignstmt_str = "";
        for (unsigned i = 0; i < level; ++i) {
            assignstmt_str += TAB;
        }
        const boost::optional<std::pair<syrec::number::ptr, syrec::number::ptr>>& range = statement.lhs()->range();
        if (range) {
            assignstmt_str += "sc_uint<";
            std::string r_first  = number_to_systemc(*range->first);
            std::string r_second = number_to_systemc(*range->second);
            int         rangeint = abs((atoi(r_second.c_str())) - (atoi(r_first.c_str()))) + 1;
            if (rangeint != 0) {
                assignstmt_str += boost::lexical_cast<std::string>(rangeint);
                assignstmt_str += "> temp" + boost::lexical_cast<std::string>(tempcount);
                assignstmt_str += " = ";
                assignstmt_str += variable_access_to_systemc(*statement.lhs());
                assignstmt_str += ";\n";
                for (unsigned i = 0; i < level; ++i) {
                    assignstmt_str += TAB;
                }
                assignstmt_str += "temp" + boost::lexical_cast<std::string>(tempcount);
                if (statement.op() == 0) {
                    assignstmt_str += " += ";
                } else if (statement.op() == 1) {
                    assignstmt_str += " -= ";
                } else if (statement.op() == 2) {
                    assignstmt_str += " ^= ";
                }
                assignstmt_str += expression_to_systemc(statement.rhs());
                assignstmt_str += ";\n";
                for (unsigned i = 0; i < level; ++i) {
                    assignstmt_str += TAB;
                }
                assignstmt_str += variable_access_to_systemc(*statement.lhs());
                assignstmt_str += " = temp" + boost::lexical_cast<std::string>(tempcount);
                assignstmt_str += ";\n";

                tempcount++;
            }
        } else {
            assignstmt_str += variable_access_to_systemc(*statement.lhs());

            if (statement.op() == 0) {
                assignstmt_str += " += ";
            } else if (statement.op() == 1) {
                assignstmt_str += " -= ";
            } else if (statement.op() == 2) {
                assignstmt_str += " ^= ";
            }
            assignstmt_str += expression_to_systemc(statement.rhs());
            assignstmt_str += ";\n";
        }
        return assignstmt_str;
    }

    std::string systemc_writer::statement_to_systemc(const syrec::if_statement& statement, unsigned level) {
        std::string ifstmt_str = "";
        for (unsigned i = 0; i < level; ++i) {
            ifstmt_str += TAB;
        }
        ifstmt_str += "if ";

        if (dynamic_cast<syrec::numeric_expression*>(statement.condition().get())) {
            ifstmt_str += "(";
        } else if (dynamic_cast<syrec::variable_expression*>(statement.condition().get())) {
            ifstmt_str += "(";
        }
        ifstmt_str += expression_to_systemc(statement.condition());
        if (dynamic_cast<syrec::numeric_expression*>(statement.condition().get())) {
            ifstmt_str += ")";
        } else if (dynamic_cast<syrec::variable_expression*>(statement.condition().get())) {
            ifstmt_str += ")";
        }
        ifstmt_str += "\n";
        for (unsigned i = 0; i < level; ++i) {
            ifstmt_str += TAB;
        }
        ifstmt_str += "{\n";

        foreach_(syrec::statement::ptr stat, statement.then_statements()) {
            ifstmt_str += statement_to_systemc(stat, level + 1);
        }
        for (unsigned i = 0; i < level; ++i) {
            ifstmt_str += TAB;
        }
        ifstmt_str += "}\n";
        for (unsigned i = 0; i < level; ++i) {
            ifstmt_str += TAB;
        }
        ifstmt_str += "else\n";
        for (unsigned i = 0; i < level; ++i) {
            ifstmt_str += TAB;
        }
        ifstmt_str += "{\n";
        foreach_(syrec::statement::ptr stat, statement.else_statements()) {
            ifstmt_str += statement_to_systemc(stat, level + 1);
        }
        for (unsigned i = 0; i < level; ++i) {
            ifstmt_str += TAB;
        }
        ifstmt_str += "}\n";

        return ifstmt_str;
    }

    std::string systemc_writer::statement_to_systemc(const syrec::for_statement& statement, unsigned level) {
        std::string forstmt_str = "";
        for (unsigned i = 0; i < level; ++i) {
            forstmt_str += TAB;
        }
        forstmt_str += "for (int ";
        if (!statement.loop_variable().empty()) {
            forstmt_str += "$" + statement.loop_variable() + " = " + boost::lexical_cast<std::string>(*statement.range().first) + "; ";
            if (statement.is_negative_step()) {
                forstmt_str += "$" + statement.loop_variable() + " > " + boost::lexical_cast<std::string>(*statement.range().second) + "; ";
                if (!statement.step()) {
                    forstmt_str += "$i++)\n";
                    for (unsigned i = 0; i < level; ++i) {
                        forstmt_str += TAB;
                    }
                    forstmt_str += "{\n";
                } else {
                    forstmt_str += "$i -= " + boost::lexical_cast<std::string>(*statement.step()) + ")\n";
                    for (unsigned i = 0; i < level; ++i) {
                        forstmt_str += TAB;
                    }
                    forstmt_str += "{\n";
                }
            } else {
                forstmt_str += "$" + statement.loop_variable() + " < " + boost::lexical_cast<std::string>(*statement.range().second) + "; ";
                if (!statement.step()) {
                    forstmt_str += "$i++)\n";
                    for (unsigned i = 0; i < level; ++i) {
                        forstmt_str += TAB;
                    }
                    forstmt_str += "{\n";
                } else {
                    forstmt_str += "$i += " + boost::lexical_cast<std::string>(*statement.step()) + ")\n";
                    for (unsigned i = 0; i < level; ++i) {
                        forstmt_str += TAB;
                    }
                    forstmt_str += "{\n";
                }
            }
        } else if (statement.range().first) {
            forstmt_str += "$i = " + boost::lexical_cast<std::string>(*statement.range().first) + "; ";
            if (statement.is_negative_step()) {
                forstmt_str += "$i > " + boost::lexical_cast<std::string>(*statement.range().second) + "; ";
                if (!statement.step()) {
                    forstmt_str += "$i++)\n";
                    for (unsigned i = 0; i < level; ++i) {
                        forstmt_str += TAB;
                    }
                    forstmt_str += "{\n";
                } else {
                    forstmt_str += "$i -= " + boost::lexical_cast<std::string>(*statement.step()) + ")\n";
                    for (unsigned i = 0; i < level; ++i) {
                        forstmt_str += TAB;
                    }
                    forstmt_str += "{\n";
                }
            } else {
                forstmt_str += "$i < " + boost::lexical_cast<std::string>(*statement.range().second) + "; ";
                if (!statement.step()) {
                    forstmt_str += "$i++)\n";
                    for (unsigned i = 0; i < level; ++i) {
                        forstmt_str += TAB;
                    }
                    forstmt_str += "{\n";
                } else {
                    forstmt_str += "$i += " + boost::lexical_cast<std::string>(*statement.step()) + ")\n";
                    for (unsigned i = 0; i < level; ++i) {
                        forstmt_str += TAB;
                    }
                    forstmt_str += "{\n";
                }
            }
        } else {
            forstmt_str += "$i = 0; ";
            if (statement.is_negative_step()) {
                forstmt_str += "$i > " + boost::lexical_cast<std::string>(*statement.range().second) + "; ";
                if (!statement.step()) {
                    forstmt_str += "$i++)\n";
                    for (unsigned i = 0; i < level; ++i) {
                        forstmt_str += TAB;
                    }
                    forstmt_str += "{\n";
                } else {
                    forstmt_str += "$i -= " + boost::lexical_cast<std::string>(*statement.step()) + ")\n";
                    for (unsigned i = 0; i < level; ++i) {
                        forstmt_str += TAB;
                    }
                    forstmt_str += "{\n";
                }
            } else {
                forstmt_str += "$i < " + boost::lexical_cast<std::string>(*statement.range().second) + "; ";
                if (!statement.step()) {
                    forstmt_str += "$i++)\n";
                    for (unsigned i = 0; i < level; ++i) {
                        forstmt_str += TAB;
                    }
                    forstmt_str += "{\n";
                } else {
                    forstmt_str += "$i += " + boost::lexical_cast<std::string>(*statement.step()) + ")\n";
                    for (unsigned i = 0; i < level; ++i) {
                        forstmt_str += TAB;
                    }
                    forstmt_str += "{\n";
                }
            }
        }
        foreach_(syrec::statement::ptr stat, statement.statements()) {
            forstmt_str += statement_to_systemc(stat, level + 1);
        }
        for (unsigned i = 0; i < level; ++i) {
            forstmt_str += TAB;
        }
        forstmt_str += "}\n";

        return forstmt_str;
    }

    std::string systemc_writer::statement_to_systemc(const syrec::call_statement& statement, unsigned level) {
        calls.push_back(statement);
        std::string callstmt_str = "";
        unsigned    param_no     = 0;
        foreach_(std::string param_str, statement.parameters()) {
            syrec::variable::ptr mod_param = mod->find_parameter_or_variable(param_str);
            syrec::variable::ptr param     = statement.target()->parameters().at(param_no);
            if (param->type() != syrec::variable::out) {
                std::string body = "signal_";
                body += mod_param->name();
                body += boost::lexical_cast<std::string>(calls.size());
                for (unsigned i = 0; i < mod_param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += ".write(_";
                body += mod_param->name();
                for (unsigned i = 0; i < mod_param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += ");\n";
                callstmt_str += make_array_loop(body, mod_param->dimensions(), level);
            }
            param_no++;
        }
        for (unsigned i = 0; i < level; ++i) {
            callstmt_str += TAB;
        }
        callstmt_str += "m";
        callstmt_str += boost::lexical_cast<std::string>(calls.size());
        callstmt_str += "_activate.write(true);\n";
        for (unsigned i = 0; i < level; ++i) {
            callstmt_str += TAB;
        }
        callstmt_str += "wait();\n";
        for (unsigned i = 0; i < level; ++i) {
            callstmt_str += TAB;
        }
        callstmt_str += "call_";
        callstmt_str += statement.target()->name();
        callstmt_str += ".notify();\n";
        for (unsigned i = 0; i < level; ++i) {
            callstmt_str += TAB;
        }
        callstmt_str += "wait(";
        callstmt_str += statement.target()->name();
        callstmt_str += "_done);\n";
        for (unsigned i = 0; i < level; ++i) {
            callstmt_str += TAB;
        }
        callstmt_str += "m";
        callstmt_str += boost::lexical_cast<std::string>(calls.size());
        callstmt_str += "_activate.write(false);\n";
        param_no = 0;
        foreach_(std::string param_str, statement.parameters()) {
            syrec::variable::ptr mod_param = mod->find_parameter_or_variable(param_str);
            syrec::variable::ptr param     = statement.target()->parameters().at(param_no);
            if (param->type() == syrec::variable::out) {
                std::string body = "_";
                body += mod_param->name();
                for (unsigned i = 0; i < mod_param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += " = signal_";
                body += mod_param->name();
                body += boost::lexical_cast<std::string>(calls.size());
                for (unsigned i = 0; i < mod_param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += ".read();\n";
                callstmt_str += make_array_loop(body, mod_param->dimensions(), level);
            } else if (param->type() == syrec::variable::inout) {
                std::string body = "_";
                body += mod_param->name();
                for (unsigned i = 0; i < mod_param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += " = signal_";
                body += mod_param->name();
                body += boost::lexical_cast<std::string>(calls.size());
                body += "_out";
                for (unsigned i = 0; i < mod_param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += ".read();\n";
                callstmt_str += make_array_loop(body, mod_param->dimensions(), level);
            }
            param_no++;
        }
        return callstmt_str;
    }

    std::string systemc_writer::statement_to_systemc(const syrec::uncall_statement& statement, unsigned level) {
        uncalls.push_back(statement);
        std::string uncallstmt_str = "";
        unsigned    param_no       = 0;
        foreach_(std::string param_str, statement.parameters()) {
            syrec::variable::ptr mod_param = mod->find_parameter_or_variable(param_str);
            syrec::variable::ptr param     = statement.target()->parameters().at(param_no);
            if (param->type() != syrec::variable::out) {
                std::string body = "signal_";
                body += mod_param->name();
                body += boost::lexical_cast<std::string>(uncalls.size());
                body += "r";
                for (unsigned i = 0; i < mod_param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += ".write(_";
                body += mod_param->name();
                for (unsigned i = 0; i < mod_param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += ");\n";
                uncallstmt_str += make_array_loop(body, mod_param->dimensions(), level);
            }
            param_no++;
        }
        for (unsigned i = 0; i < level; ++i) {
            uncallstmt_str += TAB;
        }
        uncallstmt_str += "rev_m";
        uncallstmt_str += boost::lexical_cast<std::string>(uncalls.size());
        uncallstmt_str += "_activate.write(true);\n";
        for (unsigned i = 0; i < level; ++i) {
            uncallstmt_str += TAB;
        }
        uncallstmt_str += "wait();\n";
        for (unsigned i = 0; i < level; ++i) {
            uncallstmt_str += TAB;
        }
        uncallstmt_str += "call_rev_";
        uncallstmt_str += statement.target()->name();
        uncallstmt_str += ".notify();\n";
        for (unsigned i = 0; i < level; ++i) {
            uncallstmt_str += TAB;
        }
        uncallstmt_str += "wait(rev_";
        uncallstmt_str += statement.target()->name();
        uncallstmt_str += "_done);\n";
        for (unsigned i = 0; i < level; ++i) {
            uncallstmt_str += TAB;
        }
        uncallstmt_str += "rev_m";
        uncallstmt_str += boost::lexical_cast<std::string>(uncalls.size());
        uncallstmt_str += "_activate.write(false);\n";
        param_no = 0;
        foreach_(std::string param_str, statement.parameters()) {
            syrec::variable::ptr mod_param = mod->find_parameter_or_variable(param_str);
            syrec::variable::ptr param     = statement.target()->parameters().at(param_no);
            if (param->type() == syrec::variable::out) {
                std::string body = "_";
                body += mod_param->name();
                for (unsigned i = 0; i < mod_param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += " = signal_";
                body += mod_param->name();
                body += boost::lexical_cast<std::string>(uncalls.size());
                body += "r";
                for (unsigned i = 0; i < mod_param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += ".read();\n";
                uncallstmt_str += make_array_loop(body, mod_param->dimensions(), level);
            } else if (param->type() == syrec::variable::inout) {
                std::string body = "_";
                body += mod_param->name();
                for (unsigned i = 0; i < mod_param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += " = signal_";
                body += mod_param->name();
                body += boost::lexical_cast<std::string>(uncalls.size());
                body += "r_out";
                for (unsigned i = 0; i < mod_param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += ".read();\n";
                uncallstmt_str += make_array_loop(body, mod_param->dimensions(), level);
            }
            param_no++;
        }

        return uncallstmt_str;
    }

    std::string systemc_writer::statement_to_systemc(const syrec::skip_statement& statement) {
        return "";
    }

    std::string systemc_writer::statement_to_systemc(syrec::statement::ptr statement, unsigned level) {
        if (syrec::swap_statement* stat = dynamic_cast<syrec::swap_statement*>(statement.get())) {
            return statement_to_systemc(*stat, level);
        } else if (syrec::unary_statement* stat = dynamic_cast<syrec::unary_statement*>(statement.get())) {
            return statement_to_systemc(*stat, level);
        } else if (syrec::assign_statement* stat = dynamic_cast<syrec::assign_statement*>(statement.get())) {
            return statement_to_systemc(*stat, level);
        } else if (syrec::if_statement* stat = dynamic_cast<syrec::if_statement*>(statement.get())) {
            return statement_to_systemc(*stat, level);
        } else if (syrec::for_statement* stat = dynamic_cast<syrec::for_statement*>(statement.get())) {
            return statement_to_systemc(*stat, level);
        } else if (syrec::call_statement* stat = dynamic_cast<syrec::call_statement*>(statement.get())) {
            return statement_to_systemc(*stat, level);
        } else if (syrec::uncall_statement* stat = dynamic_cast<syrec::uncall_statement*>(statement.get())) {
            return statement_to_systemc(*stat, level);
        } else if (syrec::skip_statement* stat = dynamic_cast<syrec::skip_statement*>(statement.get())) {
            return statement_to_systemc(*stat);
        } else {
            return "";
        }
    }

    std::string systemc_writer::module_to_systemc(syrec::module::ptr module) {
        // TODO: segfault nach der Ausführung
        calls.clear();
        uncalls.clear();
        mod                                            = module;
        bool                            mod_is_mainmod = (mod->name() == main_mod->name());
        std::list<syrec::variable::ptr> io_ports;
        std::string                     mod_systemc = "SC_MODULE(_";
        mod_systemc += mod->name();
        mod_systemc += ")\n{\n";
        // clock und extra-Input zur Aktivierung
        mod_systemc += TAB;
        mod_systemc += "sc_in<bool> clk;\n";
        mod_systemc += TAB;
        mod_systemc += "sc_in<bool> activate;\n";
        // ports
        if (!mod->parameters().empty()) {
            foreach_(syrec::variable::ptr param, mod->parameters()) {
                mod_systemc += TAB;
                mod_systemc += "sc_";
                if (param->type() == syrec::variable::in) {
                    mod_systemc += "in";
                } else if (param->type() == syrec::variable::out) {
                    mod_systemc += "out";
                } else if (param->type() == syrec::variable::inout) {
                    io_ports.push_back(param);
                    mod_systemc += "in";
                }
                if (param->bitwidth() <= 64) {
                    mod_systemc += "< sc_uint<";
                } else {
                    mod_systemc += "< sc_biguint<";
                }
                mod_systemc += boost::lexical_cast<std::string>(param->bitwidth());
                mod_systemc += "> > port_";
                mod_systemc += param->name();
                if (param->type() == syrec::variable::inout) {
                    mod_systemc += "_in";
                }
                foreach_(unsigned i, param->dimensions()) {
                    mod_systemc += "[";
                    mod_systemc += boost::lexical_cast<std::string>(i);
                    mod_systemc += "]";
                }
                mod_systemc += ";\n";
            }
            foreach_(syrec::variable::ptr param, io_ports) {
                mod_systemc += TAB;
                mod_systemc += "sc_out";
                if (param->bitwidth() <= 64) {
                    mod_systemc += "< sc_uint<";
                } else {
                    mod_systemc += "< sc_biguint<";
                }
                mod_systemc += boost::lexical_cast<std::string>(param->bitwidth());
                mod_systemc += "> > port_";
                mod_systemc += param->name();
                mod_systemc += "_out";
                foreach_(unsigned i, param->dimensions()) {
                    mod_systemc += "[";
                    mod_systemc += boost::lexical_cast<std::string>(i);
                    mod_systemc += "]";
                }
                mod_systemc += ";\n";
            }
            mod_systemc += "\n";
        }
        // state-Variablen
        foreach_(syrec::variable::ptr var, mod->variables()) {
            if (var->type() == syrec::variable::state) {
                mod_systemc += TAB;
                if (var->bitwidth() <= 64) {
                    mod_systemc += "sc_uint<";
                } else {
                    mod_systemc += "sc_biguint<";
                }
                mod_systemc += boost::lexical_cast<std::string>(var->bitwidth());
                mod_systemc += "> _";
                mod_systemc += var->name();
                foreach_(unsigned i, var->dimensions()) {
                    mod_systemc += "[";
                    mod_systemc += boost::lexical_cast<std::string>(i);
                    mod_systemc += "]";
                }
                mod_systemc += ";\n";
            }
        }

        // doit()
        std::string doit_str = TAB;
        doit_str += "void doit()\n";
        doit_str += TAB;
        doit_str += "{\n";
        doit_str += TAB;
        doit_str += TAB;
        doit_str += "while(true)\n";
        doit_str += TAB;
        doit_str += TAB;
        doit_str += "{\n";
        doit_str += TAB;
        doit_str += TAB;
        doit_str += TAB;
        doit_str += "wait(call_";
        doit_str += mod->name();
        doit_str += ");\n";
        doit_str += TAB;
        doit_str += TAB;
        doit_str += TAB;
        doit_str += "if(activate)\n";
        doit_str += TAB;
        doit_str += TAB;
        doit_str += TAB;
        doit_str += "{\n";
        // lokale Variablen (wires)
        foreach_(syrec::variable::ptr var, mod->variables()) {
            if (var->type() == syrec::variable::wire) {
                doit_str += TAB;
                doit_str += TAB;
                doit_str += TAB;
                doit_str += TAB;
                if (var->bitwidth() <= 64) {
                    doit_str += "sc_uint<";
                } else {
                    doit_str += "sc_biguint<";
                }
                doit_str += boost::lexical_cast<std::string>(var->bitwidth());
                doit_str += "> _";
                doit_str += var->name();
                foreach_(unsigned i, var->dimensions()) {
                    doit_str += "[";
                    doit_str += boost::lexical_cast<std::string>(i);
                    doit_str += "]";
                }
                if (var->dimensions().empty()) {
                    doit_str += " = 0";
                }
                doit_str += ";\n";
            }
        }
        foreach_(syrec::variable::ptr param, mod->parameters()) {
            doit_str += TAB;
            doit_str += TAB;
            doit_str += TAB;
            doit_str += TAB;
            if (param->bitwidth() <= 64) {
                doit_str += "sc_uint<";
            } else {
                doit_str += "sc_biguint<";
            }
            doit_str += boost::lexical_cast<std::string>(param->bitwidth());
            doit_str += "> _";
            doit_str += param->name();
            foreach_(unsigned i, param->dimensions()) {
                doit_str += "[";
                doit_str += boost::lexical_cast<std::string>(i);
                doit_str += "]";
            }
            if (param->type() != syrec::variable::out) {
                doit_str += ";\n";
                std::string body = "_";
                body += param->name();
                for (unsigned i = 0; i < param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += " = port_";
                body += param->name();
                if (param->type() == syrec::variable::inout) {
                    body += "_in";
                }
                for (unsigned i = 0; i < param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += ".read();\n";
                doit_str += make_array_loop(body, param->dimensions(), 4);
            } else {
                if (param->dimensions().empty()) {
                    doit_str += " = 0";
                }
                doit_str += ";\n";
            }
        }
        doit_str += "\n";
        doit_str += TAB;
        doit_str += TAB;
        doit_str += TAB;
        doit_str += TAB;
        doit_str += "// eigentliche Funktionalität - Anfang\n";
        foreach_(syrec::statement::ptr stat, mod->statements()) {
            doit_str += statement_to_systemc(stat, 4);
        }
        doit_str += TAB;
        doit_str += TAB;
        doit_str += TAB;
        doit_str += TAB;
        doit_str += "// eigentliche Funktionalität - Ende\n\n";
        foreach_(syrec::variable::ptr param, mod->parameters()) {
            if (param->type() != syrec::variable::in) {
                std::string body = "port_";
                body += param->name();
                if (param->type() == syrec::variable::inout) {
                    body += "_out";
                }
                for (unsigned i = 0; i < param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += ".write(_";
                body += param->name();
                for (unsigned i = 0; i < param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += ");\n";
                doit_str += make_array_loop(body, param->dimensions(), 4);
            }
        }
        doit_str += TAB;
        doit_str += TAB;
        doit_str += TAB;
        doit_str += TAB;
        doit_str += "wait();\n";
        doit_str += TAB;
        doit_str += TAB;
        doit_str += TAB;
        doit_str += TAB;
        doit_str += mod->name();
        doit_str += "_done.notify();\n";
        doit_str += TAB;
        doit_str += TAB;
        doit_str += TAB;
        doit_str += "}\n";
        doit_str += TAB;
        doit_str += TAB;
        doit_str += "}\n";
        doit_str += TAB;
        doit_str += "}\n\n";
        // Signale für eingebundene Module
        unsigned mno = 1;
        for (std::list<syrec::call_statement>::const_iterator it = calls.begin(); it != calls.end(); ++it, ++mno) {
            mod_systemc += TAB;
            mod_systemc += "sc_signal<bool> m";
            mod_systemc += boost::lexical_cast<std::string>(mno);
            mod_systemc += "_activate;\n";
        }
        mno = 1;
        for (std::list<syrec::uncall_statement>::const_iterator it = uncalls.begin(); it != uncalls.end(); ++it, ++mno) {
            mod_systemc += TAB;
            mod_systemc += "sc_signal<bool> rev_m";
            mod_systemc += boost::lexical_cast<std::string>(mno);
            mod_systemc += "_activate;\n";
        }
        // eingebundene Module deklarieren
        mno = 1;
        for (std::list<syrec::call_statement>::const_iterator it = calls.begin(); it != calls.end(); ++it, ++mno) {
            mod_systemc += TAB;
            mod_systemc += "_";
            mod_systemc += it->target()->name();
            mod_systemc += " m";
            mod_systemc += boost::lexical_cast<std::string>(mno);
            //mod_systemc += "_";
            //mod_systemc += it->target()->name();
            mod_systemc += ";\n";
        }
        mno = 1;
        for (std::list<syrec::uncall_statement>::const_iterator it = uncalls.begin(); it != uncalls.end(); ++it, ++mno) {
            mod_systemc += TAB;
            mod_systemc += "rev_";
            mod_systemc += it->target()->name();
            mod_systemc += " rev_m";
            mod_systemc += boost::lexical_cast<std::string>(mno);
            //mod_systemc += "_";
            //mod_systemc += it->target()->name();
            mod_systemc += ";\n";
        }
        mod_systemc += "\n";
        // Signale für calls und uncalls
        /*mno = 1;
    for ( std::list<syrec::call_statement>::const_iterator it = calls.begin(); it != calls.end(); ++it, ++mno)
    {
      mod_systemc += TAB;
      mod_systemc += "sc_event call_m";
      mod_systemc += boost::lexical_cast<std::string>(mno);
      mod_systemc += "_";
      mod_systemc += it->target()->name();
      mod_systemc += ";\n";
      mod_systemc += TAB;
      mod_systemc += "sc_event m";
      mod_systemc += boost::lexical_cast<std::string>(mno);
      mod_systemc += "_";
      mod_systemc += it->target()->name();
      mod_systemc += "_done;\n";
    }
    mno = 1;
    for ( std::list<syrec::uncall_statement>::const_iterator it = uncalls.begin(); it != uncalls.end(); ++it, ++mno)
    {
      mod_systemc += TAB;
      mod_systemc += "sc_event call_revm";
      mod_systemc += boost::lexical_cast<std::string>(mno);
      mod_systemc += "_";
      mod_systemc += it->target()->name();
      mod_systemc += ";\n";
      mod_systemc += TAB;
      mod_systemc += "sc_event revm";
      mod_systemc += boost::lexical_cast<std::string>(mno);
      mod_systemc += "_";
      mod_systemc += it->target()->name();
      mod_systemc += "_done;\n";
    }
    mod_systemc += "\n";*/
        std::string ctor_str = TAB;
        ctor_str += "SC_CTOR(_";
        ctor_str += mod->name();
        ctor_str += ")";
        mno           = 1;
        bool firstmod = true;
        for (std::list<syrec::call_statement>::const_iterator it = calls.begin(); it != calls.end(); ++it, ++mno) {
            if (firstmod) {
                ctor_str += " : ";
                firstmod = false;
            } else {
                ctor_str += ", ";
            }
            ctor_str += "m";
            ctor_str += boost::lexical_cast<std::string>(mno);
            ctor_str += "(\"";
            ctor_str += "m";
            ctor_str += boost::lexical_cast<std::string>(mno);
            ctor_str += "_";
            ctor_str += it->target()->name();
            ctor_str += "\")";
        }
        mno = 1;
        for (std::list<syrec::uncall_statement>::const_iterator it = uncalls.begin(); it != uncalls.end(); ++it, ++mno) {
            if (firstmod) {
                ctor_str += " : ";
                firstmod = false;
            } else {
                ctor_str += ", ";
            }
            ctor_str += "revm";
            ctor_str += boost::lexical_cast<std::string>(mno);
            ctor_str += "(\"";
            ctor_str += "revm";
            ctor_str += boost::lexical_cast<std::string>(mno);
            ctor_str += "_";
            ctor_str += it->target()->name();
            ctor_str += "\")";
        }
        ctor_str += "\n";
        ctor_str += TAB;
        ctor_str += "{\n";
        // constructor - eingebundene Module verdrahten
        std::vector<std::list<std::string>> signals;
        std::vector<std::list<std::string>> io_signals;
        std::vector<std::list<std::string>> rev_signals;
        std::vector<std::list<std::string>> rev_io_signals;
        mno = 1;
        for (std::list<syrec::call_statement>::const_iterator it = calls.begin(); it != calls.end(); ++it, ++mno) {
            std::list<std::string> m_signals;
            std::list<std::string> m_io_signals;
            ctor_str += TAB;
            ctor_str += TAB;
            ctor_str += "m";
            ctor_str += boost::lexical_cast<std::string>(mno);
            ctor_str += ".clk(clk);\n";
            ctor_str += TAB;
            ctor_str += TAB;
            ctor_str += "m";
            ctor_str += boost::lexical_cast<std::string>(mno);
            ctor_str += ".activate(m";
            ctor_str += boost::lexical_cast<std::string>(mno);
            ctor_str += "_activate);\n";
            unsigned param_no = 0;
            foreach_(std::string param_str, it->parameters()) {
                syrec::variable::ptr mod_param = mod->find_parameter_or_variable(param_str);
                syrec::variable::ptr param     = it->target()->parameters().at(param_no);
                std::string          body      = "m";
                body += boost::lexical_cast<std::string>(mno);
                body += ".port_";
                body += param->name();
                if (param->type() == syrec::variable::inout) {
                    body += "_in";
                    for (unsigned i = 0; i < param->dimensions().size(); i++) {
                        body += "[i";
                        body += boost::lexical_cast<std::string>(i);
                        body += "]";
                    }
                    body += "(signal_";
                    body += mod_param->name();
                    body += boost::lexical_cast<std::string>(mno);
                    for (unsigned i = 0; i < param->dimensions().size(); i++) {
                        body += "[i";
                        body += boost::lexical_cast<std::string>(i);
                        body += "]";
                    }
                    body += ");\n";
                    body += TAB;
                    body += TAB;
                    if (!param->dimensions().empty()) {
                        body += TAB;
                    }
                    body += "m";
                    body += boost::lexical_cast<std::string>(mno);
                    body += ".port_";
                    body += param->name();
                    body += "_out";
                    for (unsigned i = 0; i < param->dimensions().size(); i++) {
                        body += "[i";
                        body += boost::lexical_cast<std::string>(i);
                        body += "]";
                    }
                    body += "(signal_";
                    body += mod_param->name();
                    body += boost::lexical_cast<std::string>(mno);
                    body += "_out";
                    for (unsigned i = 0; i < param->dimensions().size(); i++) {
                        body += "[i";
                        body += boost::lexical_cast<std::string>(i);
                        body += "]";
                    }
                    body += ");\n";
                    m_io_signals.push_back(mod_param->name());
                } else {
                    for (unsigned i = 0; i < param->dimensions().size(); i++) {
                        body += "[i";
                        body += boost::lexical_cast<std::string>(i);
                        body += "]";
                    }
                    body += "(signal_";
                    body += mod_param->name();
                    body += boost::lexical_cast<std::string>(mno);
                    for (unsigned i = 0; i < param->dimensions().size(); i++) {
                        body += "[i";
                        body += boost::lexical_cast<std::string>(i);
                        body += "]";
                    }
                    body += ");\n";
                }
                m_signals.push_back(mod_param->name());
                ctor_str += make_array_loop(body, param->dimensions(), 2);
                param_no++;
            }
            ctor_str += "\n";
            signals.push_back(m_signals);
            io_signals.push_back(m_io_signals);
        }
        mno = 1;
        for (std::list<syrec::uncall_statement>::const_iterator it = uncalls.begin(); it != uncalls.end(); ++it, ++mno) {
            std::list<std::string> m_signals;
            std::list<std::string> m_io_signals;
            ctor_str += TAB;
            ctor_str += TAB;
            ctor_str += "rev_m";
            ctor_str += boost::lexical_cast<std::string>(mno);
            ctor_str += ".clk(clk);\n";
            ctor_str += TAB;
            ctor_str += TAB;
            ctor_str += "rev_m";
            ctor_str += boost::lexical_cast<std::string>(mno);
            ctor_str += ".activate(rev_m";
            ctor_str += boost::lexical_cast<std::string>(mno);
            ctor_str += "_activate);\n";
            unsigned param_no = 0;
            foreach_(std::string param_str, it->parameters()) {
                syrec::variable::ptr mod_param = mod->find_parameter_or_variable(param_str);
                syrec::variable::ptr param     = it->target()->parameters().at(param_no);
                std::string          body      = "rev_m";
                body += boost::lexical_cast<std::string>(mno);
                body += ".port_";
                body += param->name();
                if (param->type() == syrec::variable::inout) {
                    body += "_in";
                    for (unsigned i = 0; i < param->dimensions().size(); i++) {
                        body += "[i";
                        body += boost::lexical_cast<std::string>(i);
                        body += "]";
                    }
                    body += "(signal_";
                    body += mod_param->name();
                    body += boost::lexical_cast<std::string>(mno);
                    body += "r";
                    for (unsigned i = 0; i < param->dimensions().size(); i++) {
                        body += "[i";
                        body += boost::lexical_cast<std::string>(i);
                        body += "]";
                    }
                    body += ");\n";
                    body += TAB;
                    body += TAB;
                    if (!param->dimensions().empty()) {
                        body += TAB;
                    }
                    body += "rev_m";
                    body += boost::lexical_cast<std::string>(mno);
                    body += ".port_";
                    body += param->name();
                    body += "_out";
                    for (unsigned i = 0; i < param->dimensions().size(); i++) {
                        body += "[i";
                        body += boost::lexical_cast<std::string>(i);
                        body += "]";
                    }
                    body += "(signal_";
                    body += mod_param->name();
                    body += boost::lexical_cast<std::string>(mno);
                    body += "r_out";
                    for (unsigned i = 0; i < param->dimensions().size(); i++) {
                        body += "[i";
                        body += boost::lexical_cast<std::string>(i);
                        body += "]";
                    }
                    body += ");\n";
                    m_io_signals.push_back(mod_param->name());
                } else {
                    for (unsigned i = 0; i < param->dimensions().size(); i++) {
                        body += "[i";
                        body += boost::lexical_cast<std::string>(i);
                        body += "]";
                    }
                    body += "(signal_";
                    body += mod_param->name();
                    body += boost::lexical_cast<std::string>(mno);
                    body += "r";
                    for (unsigned i = 0; i < param->dimensions().size(); i++) {
                        body += "[i";
                        body += boost::lexical_cast<std::string>(i);
                        body += "]";
                    }
                    body += ");\n";
                }
                m_signals.push_back(mod_param->name());
                ctor_str += make_array_loop(body, param->dimensions(), 2);
                param_no++;
            }
            ctor_str += "\n";
            rev_signals.push_back(m_signals);
            rev_io_signals.push_back(m_io_signals);
        }
        // TODO: hier die state Variablen (sofern vorhanden) mit 0 initialisieren? ist das noetig?
        foreach_(syrec::variable::ptr var, mod->variables()) {
            if (((var->type() == syrec::variable::state) && var->dimensions().empty())) {
                ctor_str += TAB;
                ctor_str += TAB;
                ctor_str += "_";
                ctor_str += var->name();
                ctor_str += " = ";
                if (mod_is_mainmod) {
                    std::map<std::string, unsigned int>::const_iterator it = p.initializers().find(var->name());
                    if (it != p.initializers().end()) {
                        ctor_str += boost::lexical_cast<std::string>(it->second);
                    } else {
                        ctor_str += "0";
                    }
                } else {
                    ctor_str += "0";
                }
                ctor_str += ";\n";
            }
        }
        // Methode beim Simulationskernel anmelden
        ctor_str += TAB;
        ctor_str += TAB;
        ctor_str += "SC_THREAD(doit);\n";
        ctor_str += TAB;
        ctor_str += TAB;
        ctor_str += "sensitive << clk.pos();\n";
        /*ctor_str += "SC_METHOD(doit);\n";
    // sensitivity list - da rein kombinatorisch, alle inputs?
    foreach_( syrec::variable::ptr param, mod->pacallsrameters() )
    {
      if(param->type() != syrec::variable::out)
      {
        std::string body = "sensitive << port_";
        body += param->name();
        for ( unsigned i = 0; i < param->dimensions().size(); i++)
        {
          body += "[i";
          body += boost::lexical_cast<std::string>(i);
          body += "]";
        }
        body += ";\n";
        ctor_str += make_array_loop( body, param->dimensions() );
      }
    }*/
        ctor_str += TAB;
        ctor_str += "}\n";
        ctor_str += "};\n";

        // Signale
        if ((signals.size() > 0) || (rev_signals.size() > 0)) {
            for (unsigned i = 0; i < signals.size(); ++i) {
                foreach_(std::string sig, signals[i]) {
                    syrec::variable::ptr var = mod->find_parameter_or_variable(sig);
                    mod_systemc += TAB;
                    mod_systemc += "sc_signal< ";
                    if (var->bitwidth() <= 64) {
                        mod_systemc += "sc_uint<";
                    } else {
                        mod_systemc += "sc_biguint<";
                    }
                    mod_systemc += boost::lexical_cast<std::string>(var->bitwidth());
                    mod_systemc += "> > signal_";
                    mod_systemc += var->name();
                    mod_systemc += boost::lexical_cast<std::string>(i + 1);
                    foreach_(unsigned j, var->dimensions()) {
                        mod_systemc += "[";
                        mod_systemc += boost::lexical_cast<std::string>(j);
                        mod_systemc += "]";
                    }
                    mod_systemc += ";\n";
                }
            }
            // zusätzliche Signale für inout -> in und out
            for (unsigned i = 0; i < io_signals.size(); ++i) {
                foreach_(std::string sig, io_signals[i]) {
                    syrec::variable::ptr var = mod->find_parameter_or_variable(sig);
                    mod_systemc += TAB;
                    mod_systemc += "sc_signal< ";
                    if (var->bitwidth() <= 64) {
                        mod_systemc += "sc_uint<";
                    } else {
                        mod_systemc += "sc_biguint<";
                    }
                    mod_systemc += boost::lexical_cast<std::string>(var->bitwidth());
                    mod_systemc += "> > signal_";
                    mod_systemc += var->name();
                    mod_systemc += boost::lexical_cast<std::string>(i + 1);
                    mod_systemc += "_out";
                    foreach_(unsigned j, var->dimensions()) {
                        mod_systemc += "[";
                        mod_systemc += boost::lexical_cast<std::string>(j);
                        mod_systemc += "]";
                    }
                    mod_systemc += ";\n";
                }
            }

            for (unsigned i = 0; i < rev_signals.size(); ++i) {
                foreach_(std::string sig, rev_signals[i]) {
                    syrec::variable::ptr var = mod->find_parameter_or_variable(sig);
                    mod_systemc += TAB;
                    mod_systemc += "sc_signal< ";
                    if (var->bitwidth() <= 64) {
                        mod_systemc += "sc_uint<";
                    } else {
                        mod_systemc += "sc_biguint<";
                    }
                    mod_systemc += boost::lexical_cast<std::string>(var->bitwidth());
                    mod_systemc += "> > signal_";
                    mod_systemc += var->name();
                    mod_systemc += boost::lexical_cast<std::string>(i + 1);
                    mod_systemc += "r";
                    foreach_(unsigned j, var->dimensions()) {
                        mod_systemc += "[";
                        mod_systemc += boost::lexical_cast<std::string>(j);
                        mod_systemc += "]";
                    }
                    mod_systemc += ";\n";
                }
            }
            // zusätzliche Signale für inout -> in und out
            for (unsigned i = 0; i < rev_io_signals.size(); ++i) {
                foreach_(std::string sig, rev_io_signals[i]) {
                    syrec::variable::ptr var = mod->find_parameter_or_variable(sig);
                    mod_systemc += TAB;
                    mod_systemc += "sc_signal< ";
                    if (var->bitwidth() <= 64) {
                        mod_systemc += "sc_uint<";
                    } else {
                        mod_systemc += "sc_biguint<";
                    }
                    mod_systemc += boost::lexical_cast<std::string>(var->bitwidth());
                    mod_systemc += "> > signal_";
                    mod_systemc += var->name();
                    mod_systemc += boost::lexical_cast<std::string>(i + 1);
                    mod_systemc += "r_out";
                    foreach_(unsigned j, var->dimensions()) {
                        mod_systemc += "[";
                        mod_systemc += boost::lexical_cast<std::string>(j);
                        mod_systemc += "]";
                    }
                    mod_systemc += ";\n";
                }
            }

            mod_systemc += "\n";
        }

        mod_systemc += doit_str;
        mod_systemc += ctor_str;
        return mod_systemc;
    }

    void systemc_writer::program_to_systemc(const syrec::program& prog, std::ostream& os) {
        os << "#include \"systemc.h\"" << std::endl;

        // Events zum Aufrufen von Modulen, auch für uncall, pauschal für alle angelegt
        os << std::endl;
        os << "sc_event start_display;" << std::endl;
        os << "sc_event display_ended;" << std::endl;
        foreach_(syrec::module::ptr module, prog.modules()) {
            os << "sc_event call_" << module->name() << ";" << std::endl;
            os << "sc_event " << module->name() << "_done;" << std::endl;
            os << "sc_event call_rev_" << module->name() << ";" << std::endl;
            os << "sc_event rev_" << module->name() << "_done;" << std::endl;
        }

        foreach_(syrec::module::ptr module, prog.modules()) {
            os << std::endl
               << module_to_systemc(module) << std::endl;
        }

        /*
    //veraltet
    //signale sammeln
    //jedes nur einmal benötigt: "mitschreiben" welche man schon hat
    std::list<syrec::variable::ptr> siglist;
    std::set<std::string> sigset; //nur namen
    foreach_( syrec::module::ptr module, prog.modules() )
    {
      foreach_( syrec::variable::ptr param, module->parameters() )
      {
        if ( !sigset.empty() )
        {
          if ( sigset.find( param->name() ) == sigset.end() )
          {
            sigset.insert( param->name() );
            siglist.push_back(param);
          }
        }
        else
        {
          sigset.insert( param->name() );
          siglist.push_back(param);
        }
      }
    }
    */
        std::string stimgen;
        stimgen += "SC_MODULE(stimgen)\n{\n";
        //ausgangssignale sammeln
        std::list<syrec::variable::ptr> stimgen_out;
        for (std::vector<std::string>::const_iterator i = p.inputs().begin(); i != p.inputs().end(); ++i) {
            foreach_(syrec::variable::ptr param, main_mod->parameters()) {
                if (*i == param->name()) {
                    stimgen_out.push_back(param);
                }
            }
        }
        //inouts sammeln für initialize
        std::list<syrec::variable::ptr> stimgen_inout;
        std::list<std::string>          stimgen_init;
        for (std::map<std::string, unsigned int>::const_iterator i = p.initializers().begin(); i != p.initializers().end(); ++i) {
            foreach_(syrec::variable::ptr param, main_mod->parameters()) {
                if (i->first == param->name()) {
                    stimgen_inout.push_back(param);
                    stimgen_init.push_back(boost::lexical_cast<std::string>(i->second));
                }
            }
        }
        //stimgen:ports
        std::list<syrec::variable::ptr> main_io_ports;
        foreach_(syrec::variable::ptr param, main_mod->parameters()) {
            if (param->type() == syrec::variable::inout) {
                main_io_ports.push_back(param);
            }
        }
        //foreach_( syrec::variable::ptr param, stimgen_out )
        foreach_(syrec::variable::ptr param, main_mod->parameters()) {
            if (param->type() == syrec::variable::inout || param->type() == syrec::variable::in) {
                stimgen += TAB;
                stimgen += "sc_out";
                if (param->bitwidth() <= 64) {
                    stimgen += "< sc_uint<";
                } else {
                    stimgen += "< sc_biguint<";
                }
                stimgen += boost::lexical_cast<std::string>(param->bitwidth());
                stimgen += "> > port_";
                stimgen += param->name();
                if (param->type() == syrec::variable::inout) {
                    stimgen += "_in";
                }
                foreach_(unsigned i, param->dimensions()) {
                    stimgen += "[";
                    stimgen += boost::lexical_cast<std::string>(i);
                    stimgen += "]";
                }
                stimgen += ";\n";
            }
        }

        stimgen += TAB;
        stimgen += "sc_in<bool> clk;\n";
        stimgen += TAB;
        stimgen += "sc_out<bool> main_activate;\n\n";
        stimgen += TAB;
        //stimgen:doit
        stimgen += "void doit()\n";
        stimgen += TAB;
        stimgen += "{\n";
        stimgen += TAB;
        stimgen += TAB;
        stimgen += "main_activate.write(true);\n";
        for (std::vector<std::vector<unsigned int>>::const_iterator it = p.patterns().begin(); it != p.patterns().end(); ++it) {
            std::vector<unsigned int>::const_iterator it2 = it->begin();
            foreach_(syrec::variable::ptr param, stimgen_out) {
                stimgen += TAB;
                stimgen += TAB;
                stimgen += "port_";
                stimgen += param->name();
                if (param->type() == syrec::variable::inout) {
                    stimgen += "_in";
                }
                stimgen += ".write(";
                stimgen += boost::lexical_cast<std::string>(*it2);
                it2++;
                stimgen += ");\n";
            }
            stimgen += TAB;
            stimgen += TAB;
            stimgen += "wait();\n";
            stimgen += TAB;
            stimgen += TAB;
            stimgen += "call_" + main_mod->name() + ".notify();\n";
            stimgen += TAB;
            stimgen += TAB;
            stimgen += "wait(" + main_mod->name() + "_done);\n";
            stimgen += TAB;
            stimgen += TAB;
            stimgen += "start_display.notify();\n";
            stimgen += TAB;
            stimgen += TAB;
            stimgen += "wait(display_ended);\n\n";
        }
        stimgen += TAB;
        stimgen += TAB;
        stimgen += "sc_stop();\n";
        stimgen += TAB;
        stimgen += "}\n\n";
        stimgen += TAB;
        //stimgen:CTOR
        stimgen += "SC_CTOR(stimgen)\n";
        stimgen += TAB;
        stimgen += "{\n";
        stimgen += TAB;
        stimgen += TAB;
        foreach_(syrec::variable::ptr param, stimgen_inout) {
            stimgen += "port_";
            stimgen += param->name();
            if (param->type() == syrec::variable::inout) {
                stimgen += "_in";
            }
            stimgen += ".initialize(";
            stimgen += stimgen_init.back();
            stimgen_init.pop_back();
            stimgen += ");\n";
            stimgen += TAB;
            stimgen += TAB;
        }
        /*for ( std::map<std::string,unsigned int>::const_iterator i = p.init.begin(); i != p.init.end(); ++i)
    {
      stimgen += "port_";
      stimgen += i->first;

      stimgen += ".initialize(";
      stimgen += boost::lexical_cast<std::string>(i->second);
      stimgen += ");\n";
      stimgen += TAB;
      stimgen += TAB;
    }*/
        stimgen += "SC_THREAD(doit);\n";
        stimgen += TAB;
        stimgen += TAB;
        stimgen += "sensitive << clk.pos();\n";
        stimgen += TAB;
        stimgen += "}\n};\n\n";

        //monitor
        std::string monitor;
        monitor += "SC_MODULE(display)\n{\n";
        //monitor:ports
        foreach_(syrec::variable::ptr param, main_mod->parameters()) {
            monitor += TAB;
            monitor += "sc_in";
            if (param->bitwidth() <= 64) {
                monitor += "< sc_uint<";
            } else {
                monitor += "< sc_biguint<";
            }
            monitor += boost::lexical_cast<std::string>(param->bitwidth());
            monitor += "> > port_";
            monitor += param->name();
            if (param->type() == syrec::variable::inout) {
                monitor += "_in";
            }
            foreach_(unsigned i, param->dimensions()) {
                monitor += "[";
                monitor += boost::lexical_cast<std::string>(i);
                monitor += "]";
            }
            monitor += ";\n";
        }
        foreach_(syrec::variable::ptr param, main_io_ports) {
            monitor += TAB;
            monitor += "sc_in";
            if (param->bitwidth() <= 64) {
                monitor += "< sc_uint<";
            } else {
                monitor += "< sc_biguint<";
            }
            monitor += boost::lexical_cast<std::string>(param->bitwidth());
            monitor += "> > port_";
            monitor += param->name();
            monitor += "_out";
            foreach_(unsigned i, param->dimensions()) {
                monitor += "[";
                monitor += boost::lexical_cast<std::string>(i);
                monitor += "]";
            }
            monitor += ";\n";
        }
        monitor += TAB;
        monitor += "unsigned int step;\n";

        monitor += "\n";
        //monitor:doit
        monitor += TAB;
        monitor += "void doit()\n";
        monitor += TAB;
        monitor += "{\n";
        //einmal startzustand ausgeben
        /*
    foreach_( syrec::variable::ptr param, main_mod->parameters() )
    {
      if(param->dimensions().empty())
      {
        monitor += TAB;
        monitor += TAB;
        monitor += "cout << \"port_" + param->name();
        if(param->type() == syrec::variable::inout)
        {
          monitor += "_in";
        }
        monitor += ": \" << port_" + param->name();
        if(param->type() == syrec::variable::inout)
        {
          monitor += "_in";
        }
        monitor += " << \" at \" << sc_time_stamp() << \"\\n\";";
        monitor += "\n";
      }
      else
      {
        std::string body = "cout << \"port_" + param->name();
        if(param->type() == syrec::variable::inout)
        {
          monitor += "_in";
        }
        for ( unsigned i = 0; i < param->dimensions().size(); i++)
        {
          body += "[\" << i";
          body += boost::lexical_cast<std::string>(i);
          body += " << \"]";
        }
        body += ": \" << port_" + param->name();
        if(param->type() == syrec::variable::inout)
        {
          monitor += "_in";
        }
        for ( unsigned i = 0; i < param->dimensions().size(); i++)
        {
          body += "[i";
          body += boost::lexical_cast<std::string>(i);
          body += "]";
        }
        body += " << \" at \" << sc_time_stamp() << \"\\n\";";
        body += "\n";
        monitor += make_array_loop( body, param->dimensions(), 3 );
      }
    }
    foreach_( syrec::variable::ptr param, main_io_ports )
    {
      if(param->dimensions().empty())
      {
        monitor += TAB;
        monitor += TAB;
        monitor += "cout << \"port_" + param->name();
        monitor += "_out";
        monitor += ": \" << port_" + param->name();
        monitor += "_out";
        monitor += " << \" at \" << sc_time_stamp() << \"\\n\";";
        monitor += "\n";
      }
      else
      {
        std::string body = "cout << \"port_" + param->name();
        monitor += "_out";
        for ( unsigned i = 0; i < param->dimensions().size(); i++)
        {
          body += "[\" << i";
          body += boost::lexical_cast<std::string>(i);
          body += " << \"]";
        }
        body += ": \" << port_" + param->name();
        monitor += "_out";
        for ( unsigned i = 0; i < param->dimensions().size(); i++)
        {
          body += "[i";
          body += boost::lexical_cast<std::string>(i);
          body += "]";
        }
        body += " << \" at \" << sc_time_stamp() << \"\\n\";";
        body += "\n";
        monitor += make_array_loop( body, param->dimensions(), 3 );
      }
    }*/
        //
        monitor += TAB;
        monitor += TAB;
        monitor += "while(true)\n";
        monitor += TAB;
        monitor += TAB;
        monitor += "{\n";
        monitor += TAB;
        monitor += TAB;
        monitor += TAB;
        monitor += "wait(start_display);\n";
        monitor += TAB;
        monitor += TAB;
        monitor += TAB;
        monitor += "cout << \"step \" << step << \":\" << endl;\n";
        foreach_(syrec::variable::ptr param, main_mod->parameters()) {
            /*if(param->dimensions().empty())
      {
        monitor += TAB;
        monitor += TAB;
        monitor += TAB;
        monitor += "cout << \"port_" + param->name();
        if(param->type() == syrec::variable::inout)
        {
          monitor += "_in";
        }
        monitor += ": \" << port_" + param->name();
        if(param->type() == syrec::variable::inout)
        {
          monitor += "_in";
        }
        monitor += " << \" at \" << sc_time_stamp() << \"\\n\";";
        monitor += "\n";
      }
      else
      {*/
            std::string body = "cout << \"" + param->name();
            if (param->type() == syrec::variable::inout) {
                body += "_in";
            }
            for (unsigned i = 0; i < param->dimensions().size(); i++) {
                body += "[\" << i";
                body += boost::lexical_cast<std::string>(i);
                body += " << \"]";
            }
            body += ": \" << port_" + param->name();
            if (param->type() == syrec::variable::inout) {
                body += "_in";
            }
            for (unsigned i = 0; i < param->dimensions().size(); i++) {
                body += "[i";
                body += boost::lexical_cast<std::string>(i);
                body += "]";
            }
            body += " << endl;";
            body += "\n";
            monitor += make_array_loop(body, param->dimensions(), 3);
            //}
        }
        foreach_(syrec::variable::ptr param, main_io_ports) {
            /*if(param->dimensions().empty())
      {
        monitor += TAB;
        monitor += TAB;
        monitor += TAB;
        monitor += "cout << \"port_" + param->name();
        monitor += "_out";
        monitor += ": \" << port_" + param->name();
        monitor += "_out";
        monitor += " << \" at \" << sc_time_stamp() << \"\\n\";";
        monitor += "\n";
      }
      else
      {*/
            std::string body = "cout << \"" + param->name();
            body += "_out";
            for (unsigned i = 0; i < param->dimensions().size(); i++) {
                body += "[\" << i";
                body += boost::lexical_cast<std::string>(i);
                body += " << \"]";
            }
            body += ": \" << port_" + param->name();
            body += "_out";
            for (unsigned i = 0; i < param->dimensions().size(); i++) {
                body += "[i";
                body += boost::lexical_cast<std::string>(i);
                body += "]";
            }
            body += " << endl;";
            body += "\n";
            monitor += make_array_loop(body, param->dimensions(), 3);
            //}
        }
        monitor += TAB;
        monitor += TAB;
        monitor += TAB;
        monitor += "cout << endl;\n";
        monitor += TAB;
        monitor += TAB;
        monitor += TAB;
        monitor += "step++;\n";
        monitor += TAB;
        monitor += TAB;
        monitor += TAB;
        monitor += "display_ended.notify();\n";
        monitor += TAB;
        monitor += TAB;
        monitor += "}\n";
        monitor += TAB;
        monitor += "}\n\n";
        monitor += TAB;
        //monitor:CTOR
        monitor += "SC_CTOR(display)\n";
        monitor += TAB;
        monitor += "{\n";
        monitor += TAB;
        monitor += TAB;
        monitor += "SC_THREAD(doit);\n";
        monitor += TAB;
        monitor += TAB;
        monitor += "step = 1;\n";
        /*monitor += TAB;
    monitor += TAB;
    foreach_( syrec::variable::ptr param, siglist )
    {
      if(param->dimensions().empty())
      {
        monitor += "sensitive << port_" + param->name() + ";\n";
        monitor += TAB;
        monitor += TAB;
      }
    }
    monitor += "\n";
    foreach_( syrec::variable::ptr param, siglist )
    {
      if(!param->dimensions().empty())
      {
        std::string body = "sensitive << port_" + param->name();
        for ( unsigned i = 0; i < param->dimensions().size(); i++)
        {
          body += "[i";
          body += boost::lexical_cast<std::string>(i);
          body += "]";
        }
        body += ";\n";
        monitor += make_array_loop( body, param->dimensions(), 2 );
      }
    }*/
        monitor += TAB;
        monitor += "}\n";
        monitor += "};\n";

        //main methode
        std::string main;
        main += "int sc_main(int argc, char* argv[]) \n";
        main += "{\n";
        main += TAB;
        main += TAB;
        main += "//Module instanzieren\n";
        main += TAB;
        main += TAB;
        main += "_" + main_mod->name() + " module_" + main_mod->name() + "(\"" + "module_" + main_mod->name() + "\");";
        main += "\n";
        main += TAB;

        main += TAB;
        main += "display module_display(\"module_display\");\n";
        main += TAB;
        main += TAB;
        main += "stimgen module_stimgen(\"module_stimgen\");\n";
        main += TAB;
        main += TAB;
        //clock
        main += "sc_clock CLOCK(\"clock\",20);\n";
        //signale deklarieren in main methode
        main += "\n";
        main += TAB;
        main += TAB;
        main += "//Signaldeklaration\n";
        main += TAB;
        main += TAB;
        foreach_(syrec::variable::ptr param, main_mod->parameters()) {
            main += "sc_signal";
            if (param->bitwidth() <= 64) {
                main += "< sc_uint<";
            } else {
                main += "< sc_biguint<";
            }
            main += boost::lexical_cast<std::string>(param->bitwidth());
            main += "> > signal_";
            main += param->name();
            if (param->type() == syrec::variable::inout) {
                main += "_in";
            }
            foreach_(unsigned i, param->dimensions()) {
                main += "[";
                main += boost::lexical_cast<std::string>(i);
                main += "]";
            }
            main += ";\n";
            main += TAB;
            main += TAB;
        }
        foreach_(syrec::variable::ptr param, main_io_ports) {
            main += "sc_signal";
            if (param->bitwidth() <= 64) {
                main += "< sc_uint<";
            } else {
                main += "< sc_biguint<";
            }
            main += boost::lexical_cast<std::string>(param->bitwidth());
            main += "> > signal_";
            main += param->name();
            main += "_out";
            foreach_(unsigned i, param->dimensions()) {
                main += "[";
                main += boost::lexical_cast<std::string>(i);
                main += "]";
            }
            main += ";\n";
            main += TAB;
            main += TAB;
        }

        main += "sc_signal<bool> main_activate;\n";
        //signale verbinden
        main += "\n";
        main += TAB;
        main += TAB;
        main += "//Bindung der Signale\n";
        main += TAB;
        main += TAB;
        main += "module_" + main_mod->name() + ".clk(CLOCK);\n";
        foreach_(syrec::variable::ptr param, main_mod->parameters()) {
            if (param->dimensions().empty()) {
                main += TAB;
                main += TAB;
                main += "module_" + main_mod->name() + ".port_" + param->name();
                if (param->type() == syrec::variable::inout) {
                    main += "_in";
                }
                main += "(signal_" + param->name();
                if (param->type() == syrec::variable::inout) {
                    main += "_in";
                }
                main += ");\n";
            } else {
                std::string body = "module_" + main_mod->name() + ".port_" + param->name();
                if (param->type() == syrec::variable::inout) {
                    body += "_in";
                }
                for (unsigned i = 0; i < param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += "(signal_" + param->name();
                if (param->type() == syrec::variable::inout) {
                    body += "_in";
                }
                for (unsigned i = 0; i < param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += ");\n";
                main += make_array_loop(body, param->dimensions(), 2);
            }
        }
        foreach_(syrec::variable::ptr param, main_io_ports) {
            if (param->dimensions().empty()) {
                main += TAB;
                main += TAB;
                main += "module_" + main_mod->name() + ".port_" + param->name();
                main += "_out(signal_" + param->name();
                main += "_out);\n";
            } else {
                std::string body = "module_" + main_mod->name() + ".port_" + param->name();
                body += "_out";
                for (unsigned i = 0; i < param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += "(signal_" + param->name();
                body += "_out";
                for (unsigned i = 0; i < param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += ");\n";
                main += make_array_loop(body, param->dimensions(), 2);
            }
        }
        main += TAB;
        main += TAB;
        main += "module_" + main_mod->name() + ".activate(main_activate);\n\n";

        //signale für display verbinden
        foreach_(syrec::variable::ptr param, main_mod->parameters()) {
            if (param->dimensions().empty()) {
                main += TAB;
                main += TAB;
                main += "module_display.port_" + param->name();
                if (param->type() == syrec::variable::inout) {
                    main += "_in";
                }
                main += "(signal_" + param->name();
                if (param->type() == syrec::variable::inout) {
                    main += "_in";
                }
                main += ");\n";
            } else {
                std::string body = "module_display.port_" + param->name();
                if (param->type() == syrec::variable::inout) {
                    body += "_in";
                }
                for (unsigned i = 0; i < param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += "(signal_" + param->name();
                if (param->type() == syrec::variable::inout) {
                    body += "_in";
                }
                for (unsigned i = 0; i < param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += ");\n";
                main += make_array_loop(body, param->dimensions(), 2);
            }
        }
        foreach_(syrec::variable::ptr param, main_io_ports) {
            if (param->dimensions().empty()) {
                main += TAB;
                main += TAB;
                main += "module_display.port_" + param->name();
                main += "_out(signal_" + param->name();
                main += "_out);\n";
            } else {
                std::string body = "module_display.port_" + param->name();
                body += "_out";
                for (unsigned i = 0; i < param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += "(signal_" + param->name();
                body += "_out";
                for (unsigned i = 0; i < param->dimensions().size(); i++) {
                    body += "[i";
                    body += boost::lexical_cast<std::string>(i);
                    body += "]";
                }
                body += ");\n";
                main += make_array_loop(body, param->dimensions(), 2);
            }
        }
        //signale für stimgen verbinden
        main += "\n";
        main += TAB;
        main += TAB;
        main += "module_stimgen.clk(CLOCK);\n";
        main += TAB;
        main += TAB;
        main += "module_stimgen.main_activate(main_activate);\n";
        // foreach_( syrec::variable::ptr param, stimgen_out )
        foreach_(syrec::variable::ptr param, main_mod->parameters()) {
            if (param->type() == syrec::variable::inout || param->type() == syrec::variable::in) {
                if (param->dimensions().empty()) {
                    main += TAB;
                    main += TAB;
                    main += "module_stimgen.port_" + param->name();
                    if (param->type() == syrec::variable::inout) {
                        main += "_in";
                    }
                    main += "(signal_" + param->name();
                    if (param->type() == syrec::variable::inout) {
                        main += "_in";
                    }
                    main += ");\n";
                } else {
                    std::string body = "module_stimgen.port_" + param->name();
                    if (param->type() == syrec::variable::inout) {
                        body += "_in";
                    }
                    for (unsigned i = 0; i < param->dimensions().size(); i++) {
                        body += "[i";
                        body += boost::lexical_cast<std::string>(i);
                        body += "]";
                    }
                    body += "(signal_" + param->name();
                    if (param->type() == syrec::variable::inout) {
                        body += "_in";
                    }
                    for (unsigned i = 0; i < param->dimensions().size(); i++) {
                        body += "[i";
                        body += boost::lexical_cast<std::string>(i);
                        body += "]";
                    }
                    body += ");\n";
                    main += make_array_loop(body, param->dimensions(), 2);
                }
            }
        }

        //main methode
        main += "\n";
        main += TAB;
        main += TAB;
        main += "sc_start();\n";
        main += TAB;
        main += TAB;
        main += "return 0;\n";
        main += "}\n";
        os << stimgen << std::endl;
        os << monitor << std::endl;
        os << main << std::endl;
    }

    void write_systemc(const syrec::program& prog, std::ostream& os, properties::ptr settings, const pattern& p) {
        // main-Modul ermitteln (aus Synthesizer) und übersetzen
        std::string        main_module = get<std::string>(settings, "main_module", std::string());
        syrec::module::ptr main_mod    = prog.find_module(main_module);
        if (!main_mod) {
            main_mod = prog.find_module("main");
            if (!main_mod) {
                main_mod = prog.modules().back();
            }
        }
        systemc_writer sc_writer = systemc_writer(main_mod, p);
        sc_writer.program_to_systemc(prog, os);
    }

    void write_systemc(const syrec::program& prog, const std::string& filename, properties::ptr settings, const pattern& p) {
        std::ofstream os(filename.c_str());
        write_systemc(prog, os, settings, p);
    }

} // namespace revkit
