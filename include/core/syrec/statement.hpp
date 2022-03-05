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

/**
 * @file statement.hpp
 *
 * @brief SyReC statement data types
 */
#ifndef STATEMENT_HPP
#define STATEMENT_HPP

#include "core/syrec/expression.hpp"
#include "core/syrec/module.hpp"
#include "core/syrec/variable.hpp"

#include <iostream>
#include <memory>
#include <vector>

namespace syrec::applications {

    class module;

    /**
     * @brief Abstract base class for all SyReC statements
     *
     * All statement classes are derived from this abstract class.
     * Each class has to implement the print() method. Otherwise,
     * the different classes are solely used for distinction.
     *
     * @author RevKit
     * @since  1.1
     */
    class statement {
    public:
        /**
       * @brief Smart pointer
       *
       * @author RevKit
       * @since  1.1
       */
        typedef std::shared_ptr<statement> ptr;

        /**
       * @brief Vector of smart pointers
       *
       * @author RevKit
       * @since  1.1
       */
        typedef std::vector<ptr> vec;

        /**
       * @brief Standard constructor
       *
       * Initializes default values
       *
       * @author RevKit
       * @since  1.1
       */
        statement();

        /**
       * @brief Deconstructor
       *
       * @author RevKit
       * @since  1.1
       */
        virtual ~statement();

        /**
       * @brief Prints the statement to the output stream \p os
       * 
       * @param os Output stream
       * 
       * @return Output stream
       *
       * @author RevKit
       * @since  1.1
       */
        //virtual std::ostream& print(std::ostream& os) const;

        /**
       * @brief Sets the line number of the SyReC code
       * 
       * @param line_number Line number in SyReC code
       *
       * @author RevKit
       * @since  1.1
       */
        void set_line_number(unsigned line_number);

        /**
       * @brief Returns the line number of the SyReC code
       *
       * @return Line number in SyReC code
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] unsigned line_number() const;

    private:
        class priv;
        priv* const d = nullptr;
    };

    /**
     * @brief SWAP Statement
     *
     * This class represents the SyReC SWAP Statement (<=>)
     * between two variables lhs() and rhs().
     *
     * @author RevKit
     * @since  1.1
     */
    class swap_statement: public statement {
    public:
        /**
       * @brief Standard constructor
       * 
       * Initializes default values
       *
       * @author RevKit
       * @since  1.1
       */
        //swap_statement();

        /**
       * @brief Constructor
       * 
       * @param lhs Variable access on left hand side
       * @param rhs Variable access on right hand side
       *
       * @author RevKit
       * @since  1.1
       */
        swap_statement(variable_access::ptr lhs,
                       variable_access::ptr rhs);

        /**
       * @brief Deconstructor
       * 
       * @author RevKit
       * @since  1.1
       */
        ~swap_statement() override;

        /**
       * @brief Sets variable access on left hand side
       * 
       * @param lhs Variable access
       *
       * @author RevKit
       * @since  1.1
       */
        //[[maybe_unused]] void set_lhs(variable_access::ptr lhs);

        /**
       * @brief Returns variable access on left hand side
       * 
       * @return Variable access
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] variable_access::ptr lhs() const;

        /**
       * @brief Sets variable access on right hand side
       * 
       * @param rhs Variable access
       *
       * @author RevKit
       * @since  1.1
       */
        //[[maybe_unused]] void set_rhs(variable_access::ptr rhs);

        /**
       * @brief Returns variable access on right hand side
       * 
       * @return Variable access
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] variable_access::ptr rhs() const;

        /**
       * @brief Prints the statement to the output stream \p os
       * 
       * @param os Output stream
       * 
       * @return Output stream
       *
       * @author RevKit
       * @since  1.1
       */
        //std::ostream& print(std::ostream& os) const override;

    private:
        class priv;
        priv* const d = nullptr;
    };

    /**
     * @brief Unary Statement
     *
     * This class represents the SyReC Unary statements (++, --, ~)
     * on the variable access var().
     *
     * @author RevKit
     * @since  1.1
     */
    class unary_statement: public statement {
    public:
        /**
       * @brief Type of the statement
       *
       * @author RevKit
       * @since  1.1
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
       * @brief Standard constructor
       * 
       * Initializes default values
       *
       * @author RevKit
       * @since  1.1
       */
        //unary_statement();

        /**
       * @brief Constructor
       * 
       * @param op Operation
       * @param var Variable access to be transformed
       * 
       * @author RevKit
       * @since  1.1
       */
        unary_statement(unsigned             op,
                        variable_access::ptr var);

        /**
       * @brief Deconstructor
       *
       * @author RevKit
       * @since  1.1
       */
        ~unary_statement() override;

        /**
       * @brief Sets the operation of the statement
       * 
       * @param op Operation
       *
       * @author RevKit
       * @since  1.1
       */
        //void set_op(unsigned op);

        /**
       * @brief Returns the operation of the statement
       * 
       * @return Operation
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] unsigned op() const;

        /**
       * @brief Sets the variable access of the statement
       * 
       * @param var Variable access
       *
       * @author RevKit
       * @since  1.1
       */
        //void set_var(variable_access::ptr var);

        /**
       * @brief Returns the variable access of the statement
       * 
       * @return Variable access
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] variable_access::ptr var() const;

        /**
       * @brief Prints the statement to the output stream \p os
       * 
       * @param os Output stream
       * 
       * @return Output stream
       *
       * @author RevKit
       * @since  1.1
       */
        // std::ostream& print(std::ostream& os) const override;

    private:
        class priv;
        priv* const d = nullptr;
    };

    /**
     * @brief Assignment Statement
     *
     * This class represents the SyReC assignment statements (+=, -=, ^=)
     * of the expression rhs() to the variable access lhs().
     *
     * @author RevKit
     * @since  1.1
     */
    class assign_statement: public statement {
    public:
        /**
       * @brief Type of assignment
       *
       * @author RevKit
       * @since  1.1
       */
        enum {
            /**
         * @brief Addition to itself
         */
            add,

            /**
         * @brief Substraction from itself
         */
            subtract,

            /**
         * @brief Reflexive EXOR operation
         */
            exor
        };

        /**
       * @brief Standard constructor
       *
       * Initilizes default values
       *
       * @author RevKit
       * @since  1.1
       */
        //assign_statement();

        /**
       * @brief Constructor
       * 
       * @param lhs Variable access to which the operation is applied
       * @param op Operation to be applied
       * @param rhs Expression to be evaluated
       * 
       * @author RevKit
       * @since  1.1
       */
        assign_statement(variable_access::ptr lhs,
                         unsigned             op,
                         expression::ptr      rhs);

        /**
       * @brief Deconstructor
       *
       * @author RevKit
       * @since  1.1
       */
        ~assign_statement() override;

        /**
       * @brief Sets variable access to which the operation is applied
       * 
       * @param lhs Variable access
       *
       * @author RevKit
       * @since  1.1
       */
        //[[maybe_unused]] void set_lhs(variable_access::ptr lhs);

        /**
       * @brief Returns variable access to which the operation is applied
       * 
       * @return Variable access
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] variable_access::ptr lhs() const;

        /**
       * @brief Sets the expression to be evaluated
       * 
       * @param rhs Expression
       *
       * @author RevKit
       * @since  1.1
       */
        //[[maybe_unused]] void set_rhs(expression::ptr rhs);

        /**
       * @brief Returns the expression to be evaluated
       * 
       * @return Expression
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] expression::ptr rhs() const;

        /**
       * @brief Sets the operation to be applied
       * 
       * @param op Operation
       *
       * @author RevKit
       * @since  1.1
       */
        //void set_op(unsigned op);

        /**
       * @brief Returns the operation to be applied
       * 
       * 
       * @return Operation
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] unsigned op() const;

        /**
       * @brief Prints the statement to the output stream \p os
       * 
       * @param os Output stream
       * 
       * @return Output stream
       *
       * @author RevKit
       * @since  1.1
       */
        // std::ostream& print(std::ostream& os) const override;

    private:
        class priv;
        priv* const d = nullptr;
    };

    /**
     * @brief IF Statement
     *
     * This class represents the SyReC \b if statement
     *
     * @author RevKit
     * @since  1.1
     */
    class if_statement: public statement {
    public:
        /**
       * @brief Standard constructor
       * 
       * Initializes default values
       *
       * @author RevKit
       * @since  1.1
       */
        if_statement();

        /**
       * @brief Deconstructor
       * 
       * @author RevKit
       * @since  1.1
       */
        ~if_statement() override;

        /**
       * @brief Sets the condition for the execution of the then_statements()
       *
       * The expression \p condition is assumed to have a bit-width of 1 bit.
       * 
       * @param condition Expression
       *
       * @author RevKit
       * @since  1.1
       */
        void set_condition(expression::ptr condition);

        /**
       * @brief Returns the condition for the execution of the then_statements()
       * 
       * @return Expression
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] expression::ptr condition() const;

        /**
       * @brief Adds a statement to the then branch
       * 
       * @param then_statement Statement to be executed in the if branch
       *
       * @author RevKit
       * @since  1.1
       */
        void add_then_statement(const statement::ptr& then_statement);

        /**
       * @brief Returns all statements in the if branch
       * 
       * @return List of statements
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] const statement::vec& then_statements() const;

        /**
       * @brief Adds a statement to the else branch
       * 
       * @param else_statement Statement to be executed in the else branch
       *
       * @author RevKit
       * @since  1.1
       */
        void add_else_statement(const statement::ptr& else_statement);

        /**
       * @brief Returns all statements in the else branch
       * 
       * @return List of statements
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] const statement::vec& else_statements() const;

        /**
       * @brief Sets the reverse condition for the execution of the if_statements()
       *
       * The expression \p fi_condition is assumed to have a bit-width of 1 bit.
       * The reverse condition is checked in order the if statement is uncalled, 
       * i.e. executed reversed. Usually it is the same has the condition(), unless
       * the evalation of the condition does not change in one of the branches.
       * 
       * @param fi_condition Expression
       *
       * @author RevKit
       * @since  1.1
       */
        void set_fi_condition(expression::ptr fi_condition);

        /**
       * @brief Returns the reverse condition for the execution of the if_statements()
       * 
       * @return Expression
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] expression::ptr fi_condition() const;

        /**
       * @brief Prints the statement to the output stream \p os
       * 
       * @param os Output stream
       * 
       * @return Output stream
       *
       * @author RevKit
       * @since  1.1
       */
        // std::ostream& print(std::ostream& os) const override;

    private:
        class priv;
        priv* const d = nullptr;
    };

    /**
     * @brief FOR Statement
     *
     * This class represents the SyReC \b for statement
     *
     * @author RevKit
     * @since  1.1
     */
    class for_statement: public statement {
    public:
        /**
       * @brief Standard constructor
       *
       * Initilizes default values
       *
       * @author RevKit
       * @since  1.1
       */
        for_statement();

        /**
       * @brief Deconstructor
       *
       * @author RevKit
       * @since  1.1
       */
        ~for_statement() override;

        /**
       * @brief Sets the name of the loop variable.
       *
       * Setting a loop variable is optional.
       * 
       * @param loop_variable Name of the loop variable
       *
       * @author RevKit
       * @since  1.1
       */
        void set_loop_variable(const std::string& loop_variable);

        /**
       * @brief Returns the name of the loop variable
       * 
       * 
       * @return Name of the loop variable
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] const std::string& loop_variable() const;

        /**
       * @brief Sets the range of the loop
       *
       * The range holds the \b start and \b end values.
       * Both values are treated inclusive.
       * The start value can be larger than the end value.
       * In this case, the negative step has to be activated.
       * 
       * @param range Range of the start and end value
       *
       * @author RevKit
       * @since  1.1
       */
        void set_range(const std::pair<number::ptr, number::ptr>& range);

        /**
       * @brief Returns the range of the loop
       * 
       * @return Range of the loop
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] const std::pair<number::ptr, number::ptr>& range() const;

        /**
       * @brief Sets the step of the loop
       *
       * The step of the loop is the incremental delta which is
       * added or substracted after each loop execution. This
       * value is always positive and can be negated with the
       * set_negative_step() function. The default value is 1.
       * 
       * @param step Step of the loop
       *
       * @author RevKit
       * @since  1.1
       */
        void set_step(const number::ptr& step);

        /**
       * @brief Returns the step of the loop
       * 
       * @return Step of the loop
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] const number::ptr& step() const;

        /**
       * @brief Sets whether the step is substracted
       * 
       * @param negative_step If true, the step is substracted after each loop execution from the current index
       *
       * @author RevKit
       * @since  1.1
       */
        void set_negative_step(bool negative_step);

        /**
       * @brief Returns whether the step is substracted
       * 
       * @return If true, the step is substracted after each loop execution from the current index
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] bool is_negative_step() const;

        /**
       * @brief Adds a statement to be executed in the loop
       * 
       * @param statement Statement
       *
       * @author RevKit
       * @since  1.1
       */
        void add_statement(const statement::ptr& statement);

        /**
       * @brief Returns the statements to be executed in the loop
       * 
       * @return List of statements
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] const statement::vec& statements() const;

        /**
       * @brief Prints the statement to the output stream \p os
       * 
       * @param os Output stream
       * 
       * @return Output stream
       *
       * @author RevKit
       * @since  1.1
       */
        //  std::ostream& print(std::ostream& os) const override;

    private:
        class priv;
        priv* const d = nullptr;
    };

    /**
     * @brief CALL Statement
     *
     * This class represents the SyReC \b call statement to call a module.
     *
     * @author RevKit
     * @since  1.1
     */
    class call_statement: public statement {
    public:
        /**
       * @brief Standard constructor
       *
       * Initializes default values
       *
       * @author RevKit
       * @since  1.1
       */
        //call_statement();

        /**
       * @brief Constructor with module
       * 
       * @param target Module to call
       * 
       * @author RevKit
       * @since  1.1
       */
        //[[maybe_unused]] explicit call_statement(std::shared_ptr<module> target);

        /**
       * @brief Constructor with module and parameters
       * 
       * @param target Module to call
       * @param parameters Parameters to assign
       *
       * @author RevKit
       * @since  1.1
       */
        call_statement(std::shared_ptr<module> target, const std::vector<std::string>& parameters);

        /**
       * @brief Deconstructor
       *
       * @author RevKit
       * @since  1.1
       */
        ~call_statement() override;

        /**
       * @brief Sets the target module to call
       * 
       * @param target Module
       *
       * @author RevKit
       * @since  1.1
       */
        //void set_target(std::shared_ptr<module> target);

        /**
       * @brief Returns the target module to call
       * 
       * @return Module
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] std::shared_ptr<module> target() const;

        /**
       * @brief Sets the parameters to assign with the module call
       * 
       * @param parameters List of parameter names
       *
       * @author RevKit
       * @since  1.1
       */
        //[[maybe_unused]] void set_parameters(const std::vector<std::string>& parameters);

        /**
       * @brief Returns the parameters to assign with the module call
       * 
       * @return List of parameter names
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] const std::vector<std::string>& parameters() const;

        /**
       * @brief Prints the statement to the output stream \p os
       * 
       * @param os Output stream
       * 
       * @return Output stream
       *
       * @author RevKit
       * @since  1.1
       */
        // std::ostream& print(std::ostream& os) const override;

    private:
        class priv;
        priv* const d = nullptr;
    };

    /**
     * @brief UNCALL Statement
     *
     * This class represents the SyReC \b uncall statement to uncall a module.
     *
     * @author RevKit
     * @since  1.1
     */
    class uncall_statement: public statement {
    public:
        /**
       * @brief Standard constructor
       *
       * Initializes default values
       *
       * @author RevKit
       * @since  1.1
       */
        //uncall_statement();

        /**
       * @brief Constructor with module
       * 
       * @param target Module to uncall
       * 
       * @author RevKit
       * @since  1.1
       */
        //[[maybe_unused]] explicit uncall_statement(std::shared_ptr<module> target);

        /**
       * @brief Constructor with module and parameters
       * 
       * @param target Module to uncall
       * @param parameters Parameters to assign
       *
       * @author RevKit
       * @since  1.1
       */
        uncall_statement(std::shared_ptr<module> target, const std::vector<std::string>& parameters);

        /**
       * @brief Deconstructor
       *
       * @author RevKit
       * @since  1.1
       */
        ~uncall_statement() override;

        /**
       * @brief Sets the target module to uncall
       * 
       * @param target Module
       *
       * @author RevKit
       * @since  1.1
       */
        //void set_target(std::shared_ptr<module> target);

        /**
       * @brief Returns the target module to uncall
       * 
       * @return Module
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] std::shared_ptr<module> target() const;

        /**
       * @brief Sets the parameters to assign with the module uncall
       * 
       * @param parameters List of parameter names
       *
       * @author RevKit
       * @since  1.1
       */
        //[[maybe_unused]] void set_parameters(const std::vector<std::string>& parameters);

        /**
       * @brief Returns the parameters to assign with the module uncall
       * 
       * @return List of parameter names
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] const std::vector<std::string>& parameters() const;

        /**
       * @brief Prints the statement to the output stream \p os
       * 
       * @param os Output stream
       * 
       * @return Output stream
       *
       * @author RevKit
       * @since  1.1
       */
        //std::ostream& print(std::ostream& os) const override;

    private:
        class priv;
        priv* const d;
    };

    /**
     * @brief SKIP statement
     *
     * This class represents the SyReC \b skip statement, the
     * empty statement.
     *
     * @author RevKit
     * @since  1.1
     */
    class skip_statement: public statement {
    public:
        /**
       * @brief Deconstructor
       *
       * @author RevKit
       * @since  1.1
       */
        ~skip_statement() override;

        /**
       * @brief Prints the statement to the output stream \p os
       * 
       * @param os Output stream
       * 
       * @return Output stream
       *
       * @author RevKit
       * @since  1.1
       */
        // std::ostream& print(std::ostream& os) const override;
    };

    /**
     * @brief Prints the statement \p s to the output stream \p os
     * 
     * @param os Output stream
     * @param s Statement
     * 
     * @return Output stream
     *
     * @author RevKit
     * @since  1.1
     */
    //std::ostream& operator<<(std::ostream& os, const statement& s);

} // namespace syrec::applications

#endif /* STATEMENT_HPP */
