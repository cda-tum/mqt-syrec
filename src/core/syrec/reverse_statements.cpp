#include "core/syrec/reverse_statements.hpp"

#include <boost/foreach.hpp>

#define foreach_ BOOST_FOREACH
#define reverse_foreach_ BOOST_REVERSE_FOREACH

namespace revkit::syrec {

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
                        return statement::ptr(new assign_statement(stat_1->lhs(), assign_statement::subtract, stat_1->rhs()));

                    case assign_statement::subtract:
                        return statement::ptr(new assign_statement(stat_1->lhs(), assign_statement::add, stat_1->rhs()));

                    case assign_statement::exor:
                        return _statement;
                }
            } else if (auto* stat_2 = dynamic_cast<if_statement*>(_statement.get())) {
                auto* if_stat = new if_statement();

                if_stat->set_condition(stat_2->fi_condition());

                if_stat->set_fi_condition(stat_2->condition());

                // TODO: ohne Schleife (neue add-Funktionen notwendig)
                reverse_foreach_(statement::ptr s, stat_2->then_statements()) {
                    if_stat->add_then_statement(s);
                }

                reverse_foreach_(statement::ptr s, stat_2->else_statements()) {
                    if_stat->add_else_statement(s);
                }

                return statement::ptr(if_stat); // TODO: ist das korrekt (auch s.u. beim FOR).
            } else if (auto* stat_3 = dynamic_cast<for_statement*>(_statement.get())) {
                auto* for_stat = new for_statement();

                for_stat->set_loop_variable(stat_3->loop_variable());

                for_stat->set_range(std::make_pair(stat_3->range().second, stat_3->range().first)); // TODO: das geht doch sicher auch einfacher

                for_stat->set_step(stat_3->step());

                for_stat->set_negative_step(!stat_3->is_negative_step());

                // TODO: einfacher
                reverse_foreach_(statement::ptr s, stat_3->statements()) {
                    for_stat->add_statement(s);
                }

                return statement::ptr(for_stat); // TODO: hier ist FOR (v.o.)
            } else if (auto* stat_4 = dynamic_cast<call_statement*>(_statement.get())) {
                return statement::ptr(new uncall_statement(stat_4->target(), stat_4->parameters()));
            } else if (auto* stat_5 = dynamic_cast<uncall_statement*>(_statement.get())) {
                return statement::ptr(new call_statement(stat_5->target(), stat_5->parameters()));
            } else if (dynamic_cast<skip_statement*>(_statement.get())) {
                return _statement;
            }

            std::cout << "reverse_statement" << std::endl;

            return _statement;
        }

    } // namespace revkit
