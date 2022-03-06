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

#include "core/syrec/statement.hpp"

#include <iterator>
#include <utility>

namespace syrec::applications {

    class statement::priv {
    public:
        priv() = default;

        unsigned line_number{};
    };

    statement::statement():
        d(new priv()) {
    }

    statement::~statement() {
        delete d;
    }

    /*std::ostream& statement::print(std::ostream& os) const {
        return os;
    }*/

    void statement::set_line_number(unsigned line_number) {
        d->line_number = line_number;
    }

    unsigned statement::line_number() const {
        return d->line_number;
    }

    class swap_statement::priv {
    public:
        priv() = default;

        variable_access::ptr lhs;
        variable_access::ptr rhs;
    };

    /*  swap_statement::swap_statement():
        d(new priv()) {
    }*/

    swap_statement::swap_statement(variable_access::ptr lhs,
                                   variable_access::ptr rhs):
        d(new priv()) {
        d->lhs = std::move(lhs);
        d->rhs = std::move(rhs);
    }

    swap_statement::~swap_statement() {
        delete d;
    }

    /*[[maybe_unused]] void swap_statement::set_lhs(variable_access::ptr lhs) {
        d->lhs = std::move(lhs);
    }*/

    variable_access::ptr swap_statement::lhs() const {
        return d->lhs;
    }

    /*[[maybe_unused]] void swap_statement::set_rhs(variable_access::ptr rhs) {
        d->rhs = std::move(rhs);
    }*/

    variable_access::ptr swap_statement::rhs() const {
        return d->rhs;
    }

    /* std::ostream& swap_statement::print(std::ostream& os) const {
        return os << std::string(os.precision(), ' ') << *d->lhs << " <=> " << *d->rhs << ";" << std::endl;
    }*/

    class unary_statement::priv {
    public:
        priv() = default;

        unsigned             op{};
        variable_access::ptr var;
    };

    /*unary_statement::unary_statement():
        d(new priv()) {
    }*/

    unary_statement::unary_statement(unsigned             op,
                                     variable_access::ptr var):
        d(new priv()) {
        d->op  = op;
        d->var = std::move(var);
    }

    unary_statement::~unary_statement() {
        delete d;
    }

    /*void unary_statement::set_op(unsigned op) {
        d->op = op;
    }*/

    unsigned unary_statement::op() const {
        return d->op;
    }

    /* void unary_statement::set_var(variable_access::ptr var) {
        d->var = std::move(var);
    }*/

    variable_access::ptr unary_statement::var() const {
        return d->var;
    }

    /*std::ostream& unary_statement::print(std::ostream& os) const {
        os << std::string(os.precision(), ' ');
        switch (d->op) {
            case invert:
                os << '~';
                break;

            case increment:
                os << "++";
                break;

            case decrement:
                os << "--";
                break;
        }
        os << "= " << *d->var << ";" << std::endl;
        return os;
    }*/

    class assign_statement::priv {
    public:
        priv() = default;

        variable_access::ptr lhs;
        expression::ptr      rhs;
        unsigned             op{};
    };

    /*assign_statement::assign_statement():
        d(new priv()) {
    }*/

    assign_statement::assign_statement(variable_access::ptr lhs,
                                       unsigned             op,
                                       expression::ptr      rhs):
        d(new priv()) {
        d->lhs = std::move(lhs);
        d->op  = op;
        d->rhs = std::move(rhs);
    }

    assign_statement::~assign_statement() {
        delete d;
    }

    /* [[maybe_unused]] void assign_statement::set_lhs(variable_access::ptr lhs) {
        d->lhs = std::move(lhs);
    }*/

    variable_access::ptr assign_statement::lhs() const {
        return d->lhs;
    }

    /*[[maybe_unused]] void assign_statement::set_rhs(expression::ptr rhs) {
        d->rhs = std::move(rhs);
    }*/

    expression::ptr assign_statement::rhs() const {
        return d->rhs;
    }

    /*void assign_statement::set_op(unsigned op) {
        d->op = op;
    }*/

    unsigned assign_statement::op() const {
        return d->op;
    }

    /*std::ostream& assign_statement::print(std::ostream& os) const {
        os << std::string(os.precision(), ' ') << *d->lhs << " ";
        switch (d->op) {
            case add:
                os << '+';
                break;

            case subtract:
                os << '-';
                break;

            case exor:
                os << '^';
                break;
        }
        os << "= " << *d->rhs << ";" << std::endl;
        return os;
    }*/

    class if_statement::priv {
    public:
        priv() = default;

        expression::ptr condition;
        statement::vec  then_statements;
        statement::vec  else_statements;
        expression::ptr fi_condition;
    };

    if_statement::if_statement():
        d(new priv()) {
    }

    if_statement::~if_statement() {
        delete d;
    }

    void if_statement::set_condition(expression::ptr condition) {
        d->condition = std::move(condition);
    }

    expression::ptr if_statement::condition() const {
        return d->condition;
    }

    void if_statement::add_then_statement(const statement::ptr& then_statement) {
        d->then_statements.emplace_back(then_statement);
    }

    const statement::vec& if_statement::then_statements() const {
        return d->then_statements;
    }

    void if_statement::add_else_statement(const statement::ptr& else_statement) {
        d->else_statements.emplace_back(else_statement);
    }

    const statement::vec& if_statement::else_statements() const {
        return d->else_statements;
    }

    void if_statement::set_fi_condition(expression::ptr fi_condition) {
        d->fi_condition = std::move(fi_condition);
    }

    expression::ptr if_statement::fi_condition() const {
        return d->fi_condition;
    }

    /*std::ostream& if_statement::print(std::ostream& os) const {
        unsigned indent = os.precision();
        os.precision(indent + 2u);

        os << std::string(indent, ' ') << "if " << *d->condition << " then" << std::endl;
        for (const auto& statement: d->then_statements) {
            os << *statement;
        }
        os << std::string(indent, ' ') << "else" << std::endl;
        for (const auto& statement: d->else_statements) {
            os << *statement;
        }
        os << std::string(indent, ' ') << "fi " << *d->fi_condition << ";" << std::endl;

        os.precision(indent);
        return os;
    }*/

    class for_statement::priv {
    public:
        priv() = default;

        std::string                         loop_variable;
        std::pair<number::ptr, number::ptr> range;
        number::ptr                         step;
        bool                                negative_step = false;
        statement::vec                      statements;
    };

    for_statement::for_statement():
        d(new priv()) {
    }

    for_statement::~for_statement() {
        delete d;
    }

    void for_statement::set_loop_variable(const std::string& loop_variable) {
        d->loop_variable = loop_variable;
    }

    const std::string& for_statement::loop_variable() const {
        return d->loop_variable;
    }

    void for_statement::set_range(const std::pair<number::ptr, number::ptr>& range) {
        d->range = range;
    }

    const std::pair<number::ptr, number::ptr>& for_statement::range() const {
        return d->range;
    }

    /*void for_statement::set_step(const number::ptr& step) {
        d->step = step;
    }*/

    const number::ptr& for_statement::step() const {
        return d->step;
    }

    void for_statement::add_statement(const statement::ptr& statement) {
        d->statements.emplace_back(statement);
    }

    /*void for_statement::set_negative_step(bool negative_step) {
        d->negative_step = negative_step;
    }*/

    /*bool for_statement::is_negative_step() const {
        return d->negative_step;
    }*/

    const statement::vec& for_statement::statements() const {
        return d->statements;
    }

    /*std::ostream& for_statement::print(std::ostream& os) const {
        unsigned indent = os.precision();
        os.precision(indent + 2u);

        os << std::string(indent, ' ') << "for ";

        if (!d->loop_variable.empty()) {
            os << "$" << d->loop_variable << " = ";
        }

        if (d->range.first) {
            os << *d->range.first << " to ";
        }

        os << *d->range.second;

        if (d->step) {
            os << " step ";
            if (d->negative_step) {
                os << '-';
            }
            os << *d->step;
        }

        os << " do" << std::endl;
        for (const auto& statement: d->statements) {
            os << *statement;
        }

        os << std::string(indent, ' ') << "rof"
           << ";" << std::endl;

        os.precision(indent);
        return os;
    }*/

    class call_statement::priv {
    public:
        priv() = default;

        module::ptr              target;
        std::vector<std::string> parameters;
    };

    /* call_statement::call_statement():
        d(new priv()) {
    }*/

    /*[[maybe_unused]] call_statement::call_statement(module::ptr target):
        d(new priv()) {
        d->target = std::move(target);
    }*/

    call_statement::call_statement(module::ptr target, const std::vector<std::string>& parameters):
        d(new priv()) {
        d->target     = std::move(target);
        d->parameters = parameters;
    }

    call_statement::~call_statement() {
        delete d;
    }

    /*void call_statement::set_target(module::ptr target) {
        d->target = std::move(target);
    }*/

    module::ptr call_statement::target() const {
        return d->target;
    }

    /*[[maybe_unused]] void call_statement::set_parameters(const std::vector<std::string>& parameters) {
        d->parameters = parameters;
    }*/

    const std::vector<std::string>& call_statement::parameters() const {
        return d->parameters;
    }

    /*std::ostream& call_statement::print(std::ostream& os) const {
        os << std::string(os.precision(), ' ') << "call " << d->target->name() << "(";
        for (const auto& parameter: d->parameters) {
            if (parameter != d->parameters.front()) {
                os << ", ";
            }
            os << parameter;
        }
        os << ");" << std::endl;
        return os;
    }*/

    class uncall_statement::priv {
    public:
        priv() = default;

        module::ptr              target;
        std::vector<std::string> parameters;
    };

    /*uncall_statement::uncall_statement():
        d(new priv()) {
    }*/

    /*[[maybe_unused]] uncall_statement::uncall_statement(module::ptr target):
        d(new priv()) {
        d->target = std::move(target);
    }*/

    uncall_statement::uncall_statement(module::ptr target, const std::vector<std::string>& parameters):
        d(new priv()) {
        d->target     = std::move(target);
        d->parameters = parameters;
    }

    uncall_statement::~uncall_statement() {
        delete d;
    }

    /* void uncall_statement::set_target(module::ptr target) {
        d->target = std::move(target);
    }*/

    module::ptr uncall_statement::target() const {
        return d->target;
    }

    /*[[maybe_unused]] void uncall_statement::set_parameters(const std::vector<std::string>& parameters) {
        d->parameters = parameters;
    }*/

    const std::vector<std::string>& uncall_statement::parameters() const {
        return d->parameters;
    }

    /* std::ostream& uncall_statement::print(std::ostream& os) const {
        os << std::string(os.precision(), ' ') << "uncall " << d->target->name() << "(";
        for (const auto& parameter: d->parameters) {
            if (parameter != d->parameters.front()) {
                os << ", ";
            }
            os << parameter;
        }
        os << ");" << std::endl;
        return os;
    }*/

    skip_statement::~skip_statement() = default;

    /*std::ostream& skip_statement::print(std::ostream& os) const {
        return os << std::string(os.precision(), ' ') << "skip;" << std::endl;
    }*/

    /*std::ostream& operator<<(std::ostream& os, const statement& s) {
        return s.print(os);
    }*/

} // namespace syrec::applications
