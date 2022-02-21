#include "core/syrec/reverse_statements.hpp"

#include <boost/foreach.hpp>

#define foreach_ BOOST_FOREACH
#define reverse_foreach_ BOOST_REVERSE_FOREACH

namespace revkit {
    namespace syrec {

        statement::ptr reverse_statements::operator()(statement::ptr _statement) const {
            if (dynamic_cast<swap_statement*>(_statement.get())) {
                return _statement;
            } else if (unary_statement* stat = dynamic_cast<unary_statement*>(_statement.get())) {
                switch (stat->op()) {
                    case unary_statement::invert:
                        return _statement;

                    case unary_statement::increment:
                        return statement::ptr(new unary_statement(unary_statement::decrement, stat->var()));

                    case unary_statement::decrement:
                        return statement::ptr(new unary_statement(unary_statement::increment, stat->var()));
                }
            } else if (assign_statement* stat = dynamic_cast<assign_statement*>(_statement.get())) {
                switch (stat->op()) {
                    case assign_statement::add:
                        return statement::ptr(new assign_statement(stat->lhs(), assign_statement::subtract, stat->rhs()));

                    case assign_statement::subtract:
                        return statement::ptr(new assign_statement(stat->lhs(), assign_statement::add, stat->rhs()));

                    case assign_statement::exor:
                        return _statement;
                }
            } else if (if_statement* stat = dynamic_cast<if_statement*>(_statement.get())) {
                if_statement* if_stat = new if_statement();

                if_stat->set_condition(stat->fi_condition());

                if_stat->set_fi_condition(stat->condition());

                // TODO: ohne Schleife (neue add-Funktionen notwendig)
                reverse_foreach_(statement::ptr s, stat->then_statements()) {
                    if_stat->add_then_statement(s);
                }

                reverse_foreach_(statement::ptr s, stat->else_statements()) {
                    if_stat->add_else_statement(s);
                }

                return statement::ptr(if_stat); // TODO: ist das korrekt (auch s.u. beim FOR).
            } else if (for_statement* stat = dynamic_cast<for_statement*>(_statement.get())) {
                for_statement* for_stat = new for_statement();

                for_stat->set_loop_variable(stat->loop_variable());

                for_stat->set_range(std::make_pair(stat->range().second, stat->range().first)); // TODO: das geht doch sicher auch einfacher

                for_stat->set_step(stat->step());

                for_stat->set_negative_step(!stat->is_negative_step());

                // TODO: einfacher
                reverse_foreach_(statement::ptr s, stat->statements()) {
                    for_stat->add_statement(s);
                }

                return statement::ptr(for_stat); // TODO: hier ist FOR (v.o.)
            } else if (call_statement* stat = dynamic_cast<call_statement*>(_statement.get())) {
                return statement::ptr(new uncall_statement(stat->target(), stat->parameters()));
            } else if (uncall_statement* stat = dynamic_cast<uncall_statement*>(_statement.get())) {
                return statement::ptr(new call_statement(stat->target(), stat->parameters()));
            } else if (dynamic_cast<skip_statement*>(_statement.get())) {
                return _statement;
            }

            std::cout << "reverse_statement" << std::endl;

            return _statement;
        }

    } // namespace syrec
} // namespace revkit
