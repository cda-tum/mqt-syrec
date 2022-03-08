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

    swap_statement::swap_statement(variable_access::ptr lhs,
                                   variable_access::ptr rhs):
        d(new priv()) {
        d->lhs = std::move(lhs);
        d->rhs = std::move(rhs);
    }

    swap_statement::~swap_statement() {
        delete d;
    }

    variable_access::ptr swap_statement::lhs() const {
        return d->lhs;
    }

    variable_access::ptr swap_statement::rhs() const {
        return d->rhs;
    }

    class unary_statement::priv {
    public:
        priv() = default;

        unsigned             op{};
        variable_access::ptr var;
    };

    unary_statement::unary_statement(unsigned             op,
                                     variable_access::ptr var):
        d(new priv()) {
        d->op  = op;
        d->var = std::move(var);
    }

    unary_statement::~unary_statement() {
        delete d;
    }

    unsigned unary_statement::op() const {
        return d->op;
    }

    variable_access::ptr unary_statement::var() const {
        return d->var;
    }

    class assign_statement::priv {
    public:
        priv() = default;

        variable_access::ptr lhs;
        expression::ptr      rhs;
        unsigned             op{};
    };

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

    variable_access::ptr assign_statement::lhs() const {
        return d->lhs;
    }

    expression::ptr assign_statement::rhs() const {
        return d->rhs;
    }

    unsigned assign_statement::op() const {
        return d->op;
    }

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

    class for_statement::priv {
    public:
        priv() = default;

        std::string                         loop_variable;
        std::pair<number::ptr, number::ptr> range;
        number::ptr                         step;
        //[[maybe_unused]] bool                                negative_step = false;
        statement::vec statements;
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

    class call_statement::priv {
    public:
        priv() = default;

        module::ptr              target;
        std::vector<std::string> parameters;
    };

    call_statement::call_statement(module::ptr target, const std::vector<std::string>& parameters):
        d(new priv()) {
        d->target     = std::move(target);
        d->parameters = parameters;
    }

    call_statement::~call_statement() {
        delete d;
    }

    module::ptr call_statement::target() const {
        return d->target;
    }

    const std::vector<std::string>& call_statement::parameters() const {
        return d->parameters;
    }

    class uncall_statement::priv {
    public:
        priv() = default;

        module::ptr              target;
        std::vector<std::string> parameters;
    };

    uncall_statement::uncall_statement(module::ptr target, const std::vector<std::string>& parameters):
        d(new priv()) {
        d->target     = std::move(target);
        d->parameters = parameters;
    }

    uncall_statement::~uncall_statement() {
        delete d;
    }

    module::ptr uncall_statement::target() const {
        return d->target;
    }

    const std::vector<std::string>& uncall_statement::parameters() const {
        return d->parameters;
    }

    skip_statement::~skip_statement() = default;

    statement::ptr reverse_statements::operator()(statement::ptr _statement) const {
        if (dynamic_cast<swap_statement*>(_statement.get())) {
            return _statement;
        } else if (auto* stat = dynamic_cast<unary_statement*>(_statement.get())) {
            switch (stat->op()) {
                case unary_statement::invert:
                    return _statement;

                case unary_statement::increment:
                    return statement::ptr(new unary_statement(unary_statement::decrement, stat->var()));

                case unary_statement::decrement:
                    return statement::ptr(new unary_statement(unary_statement::increment, stat->var()));
            }
        } else if (auto* stat_1 = dynamic_cast<assign_statement*>(_statement.get())) {
            switch (stat_1->op()) {
                case assign_statement::add:
                    std::cout << "add uncall" << std::endl;
                    return statement::ptr(new assign_statement(stat_1->lhs(), assign_statement::subtract, stat_1->rhs()));

                case assign_statement::subtract:
                    std::cout << "sub uncall" << std::endl;
                    return statement::ptr(new assign_statement(stat_1->lhs(), assign_statement::add, stat_1->rhs()));

                case assign_statement::exor:
                    std::cout << "exor uncall" << std::endl;
                    return _statement;
            }
        } else if (auto* stat_2 = dynamic_cast<if_statement*>(_statement.get())) {
            auto* if_stat = new if_statement();

            if_stat->set_condition(stat_2->fi_condition());

            if_stat->set_fi_condition(stat_2->condition());

            for (auto it = stat_2->then_statements().rbegin(); it != stat_2->then_statements().rend(); ++it) {
                if_stat->add_then_statement(*it);
            }
            for (auto it = stat_2->else_statements().rbegin(); it != stat_2->else_statements().rend(); ++it) {
                if_stat->add_else_statement(*it);
            }

            return statement::ptr(if_stat);
        } else if (auto* stat_3 = dynamic_cast<for_statement*>(_statement.get())) {
            auto* for_stat = new for_statement();
            std::cout << "for1" << std::endl;
            for_stat->set_loop_variable(stat_3->loop_variable());
            std::cout << "for2" << std::endl;
            for_stat->set_range(std::make_pair(stat_3->range().second, stat_3->range().first));

            /*for_stat->set_step(stat_3->step());
            for_stat->set_negative_step(!stat_3->is_negative_step());*/

            std::cout << "for3" << std::endl;
            for (auto it = stat_3->statements().rbegin(); it != stat_3->statements().rend(); ++it) {
                std::cout << "for5" << std::endl;
                for_stat->add_statement(*it);
            }
            std::cout << "for4" << std::endl;
            return statement::ptr(for_stat);
        }
        //else if (auto* stat_4 = dynamic_cast<call_statement*>(_statement.get())) {
        //  return statement::ptr(new uncall_statement(stat_4->target(), stat_4->parameters()));
        //} else if (auto* stat_5 = dynamic_cast<uncall_statement*>(_statement.get())) {
        //    return statement::ptr(new call_statement(stat_5->target(), stat_5->parameters()));
        //}
        else if (dynamic_cast<skip_statement*>(_statement.get())) {
            return _statement;
        }

        std::cout << "reverse_statement" << std::endl;

        return _statement;
    }

} // namespace syrec::applications
