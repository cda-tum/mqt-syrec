#include "core/syrec/reverse_statements.hpp"

namespace syrec::applications {

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

            for_stat->set_loop_variable(stat_3->loop_variable());

            for_stat->set_range(std::make_pair(stat_3->range().second, stat_3->range().first));

            for_stat->set_step(stat_3->step());

            for_stat->set_negative_step(!stat_3->is_negative_step());

            for (auto it = stat_3->statements().rbegin(); it != stat_3->statements().rend(); ++it) {
                for_stat->add_statement(*it);
            }

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
