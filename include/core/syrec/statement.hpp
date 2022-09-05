#pragma once

#include "core/syrec/expression.hpp"
#include "core/syrec/module.hpp"
#include "core/syrec/variable.hpp"

#include <iostream>
#include <memory>
#include <utility>
#include <vector>

namespace syrec {

    class module;

    /**
     * @brief Abstract base class for all SyReC statements
     *
     * All statement classes are derived from this abstract class.
     * Each class has to implement the print() method. Otherwise,
     * the different classes are solely used for distinction.
     */
    struct statement {
        /**
       * @brief Smart pointer
       */
        typedef std::shared_ptr<statement> ptr;

        /**
       * @brief Vector of smart pointers
       */
        typedef std::vector<ptr> vec;

        /**
       * @brief Standard constructor
       *
       * Initializes default values
       */
        statement() = default;

        /**
       * @brief Deconstructor
       */
        virtual ~statement() = default;

        unsigned line_number{};

        virtual statement::ptr reverse() {
            return std::make_shared<statement>(*this);
        };
    };
    typedef statement skip_statement;

    /**
     * @brief SWAP Statement
     *
     * This class represents the SyReC SWAP Statement (<=>)
     * between two variables lhs() and rhs().
     */
    struct swap_statement: public statement {
        /**
       * @brief Constructor
       *
       * @param lhs Variable access on left hand side
       * @param rhs Variable access on right hand side
       */
        swap_statement(variable_access::ptr lhs,
                       variable_access::ptr rhs):
            lhs(std::move(lhs)),
            rhs(std::move(rhs)) {}

        variable_access::ptr lhs;
        variable_access::ptr rhs;
    };

    /**
     * @brief Unary Statement
     *
     * This class represents the SyReC Unary statements (++, --, ~)
     * on the variable access var().
     */
    struct unary_statement: public statement {
        /**
       * @brief Type of the statement
       */
        enum {
            /**
         * @brief Inversion of the variable
         */
            invert,

            /**
         * @brief Increment of the variable by 1
         */
            increment,

            /**
         * @brief Decrement of the variable by 1
         */
            decrement
        };

        /**
       * @brief Constructor
       *
       * @param op Operation
       * @param var Variable access to be transformed
       */
        unary_statement(unsigned             op,
                        variable_access::ptr var):
            op(op),
            var(std::move(var)) {}

        statement::ptr reverse() override {
            switch (op) {
                case unary_statement::increment:
                    return std::make_shared<unary_statement>(decrement, var);

                case unary_statement::decrement:
                    return std::make_shared<unary_statement>(increment, var);

                case unary_statement::invert:
                default:
                    return std::make_shared<unary_statement>(*this);
            }
        }

        unsigned             op{};
        variable_access::ptr var{};
    };

    /**
     * @brief Assignment Statement
     *
     * This class represents the SyReC assignment statements (+=, -=, ^=)
     * of the expression rhs() to the variable access lhs().
     */
    struct assign_statement: public statement {
        /**
       * @brief Type of assignment
       */
        enum {
            /**
         * @brief Addition to itself
         */
            add,

            /**
         * @brief Subtraction from itself
         */
            subtract,

            /**
         * @brief Reflexive EXOR operation
         */
            exor
        };

        /**
       * @brief Constructor
       *
       * @param lhs Variable access to which the operation is applied
       * @param op Operation to be applied
       * @param rhs Expression to be evaluated
       */
        assign_statement(variable_access::ptr lhs,
                         unsigned             op,
                         expression::ptr      rhs):
            lhs(std::move(lhs)),
            op(op), rhs(std::move(rhs)) {}

        statement::ptr reverse() override {
            switch (op) {
                case assign_statement::add:
                    return std::make_shared<assign_statement>(lhs, subtract, rhs);

                case assign_statement::subtract:
                    return std::make_shared<assign_statement>(lhs, add, rhs);

                case assign_statement::exor:
                default:
                    return std::make_shared<assign_statement>(*this);
            }
        }

        variable_access::ptr lhs{};
        unsigned             op{};
        expression::ptr      rhs{};
    };

    /**
     * @brief IF Statement
     *
     * This class represents the SyReC \b if statement
     */
    struct if_statement: public statement {
        /**
       * @brief Standard constructor
       *
       * Initializes default values
       */
        if_statement() = default;

        /**
       * @brief Sets the condition for the execution of the then_statements()
       *
       * The expression \p condition is assumed to have a bit-width of 1 bit.
       *
       * @param condition Expression
       */
        void set_condition(expression::ptr cond) {
            condition = std::move(cond);
        }

        /**
       * @brief Adds a statement to the then branch
       *
       * @param then_statement Statement to be executed in the if branch
       */
        void add_then_statement(const statement::ptr& then_statement) {
            then_statements.emplace_back(then_statement);
        }

        /**
       * @brief Adds a statement to the else branch
       *
       * @param else_statement Statement to be executed in the else branch
       */
        void add_else_statement(const statement::ptr& else_statement) {
            else_statements.emplace_back(else_statement);
        }

        /**
       * @brief Sets the reverse condition for the execution of the if_statements()
       *
       * The expression \p fi_condition is assumed to have a bit-width of 1 bit.
       * The reverse condition is checked in order the if statement is uncalled,
       * i.e. executed reversed. Usually it is the same has the condition(), unless
       * the evaluation of the condition does not change in one of the branches.
       *
       * @param fi_condition Expression
       */
        void set_fi_condition(expression::ptr fi_cond) {
            fi_condition = std::move(fi_cond);
        }

        statement::ptr reverse() override {
            auto fi = std::make_shared<if_statement>();
            fi->set_fi_condition(condition);
            fi->set_condition(fi_condition);
            for (auto it = then_statements.rbegin(); it != then_statements.rend(); ++it) {
                fi->add_then_statement(*it);
            }
            for (auto it = else_statements.rbegin(); it != else_statements.rend(); ++it) {
                fi->add_else_statement(*it);
            }
            return fi;
        }

        expression::ptr condition{};
        statement::vec  then_statements{};
        statement::vec  else_statements{};
        expression::ptr fi_condition{};
    };

    /**
     * @brief FOR Statement
     *
     * This class represents the SyReC \b for statement
     */
    struct for_statement: public statement {
        /**
       * @brief Standard constructor
       *
       * Initializes default values
       */
        for_statement() = default;

        /**
       * @brief Adds a statement to be executed in the loop
       *
       * @param statement Statement
       */
        void add_statement(const statement::ptr& statement) {
            statements.emplace_back(statement);
        }

        statement::ptr reverse() override {
            auto for_stat           = std::make_shared<for_statement>();
            for_stat->loop_variable = loop_variable;
            for_stat->range         = std::make_pair(range.second, range.first);
            for (auto it = statements.rbegin(); it != statements.rend(); ++it) {
                for_stat->add_statement(*it);
            }
            return for_stat;
        }

        std::string                         loop_variable{};
        std::pair<number::ptr, number::ptr> range{};
        number::ptr                         step{};
        statement::vec                      statements{};
    };

    struct uncall_statement;

    /**
     * @brief CALL Statement
     *
     * This class represents the SyReC \b call statement to call a module.
     */
    struct call_statement: public statement {
        /**
       * @brief Constructor with module and parameters
       *
       * @param target Module to call
       * @param parameters Parameters to assign
       */
        call_statement(std::shared_ptr<module> target, std::vector<std::string> parameters):
            target(std::move(target)), parameters(std::move(parameters)) {}

        statement::ptr reverse() override;

        std::shared_ptr<module>  target{};
        std::vector<std::string> parameters{};
    };

    /**
     * @brief UNCALL Statement
     *
     * This class represents the SyReC \b uncall statement to uncall a module.
     */
    struct uncall_statement: public statement {
        /**
       * @brief Constructor with module and parameters
       *
       * @param target Module to uncall
       * @param parameters Parameters to assign
       */
        uncall_statement(std::shared_ptr<module> target, std::vector<std::string> parameters):
            target(std::move(target)), parameters(std::move(parameters)) {}

        statement::ptr reverse() override {
            return std::make_shared<call_statement>(target, parameters);
        }

        std::shared_ptr<module>  target{};
        std::vector<std::string> parameters{};
    };

    inline statement::ptr call_statement::reverse() {
        return std::make_shared<uncall_statement>(target, parameters);
    }

} // namespace syrec
