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

     */
    class statement {
    public:
        /**
       * @brief Smart pointer
       *


       */
        typedef std::shared_ptr<statement> ptr;

        /**
       * @brief Vector of smart pointers
       *


       */
        typedef std::vector<ptr> vec;

        /**
       * @brief Standard constructor
       *
       * Initializes default values
       *


       */
        statement();

        /**
       * @brief Deconstructor
       *


       */
        virtual ~statement();

        /**
       * @brief Sets the line number of the SyReC code
       * 
       * @param line_number Line number in SyReC code
       *


       */
        void set_line_number(unsigned line_number);

        /**
       * @brief Returns the line number of the SyReC code
       *
       * @return Line number in SyReC code
       *


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

     */
    class swap_statement: public statement {
    public:
        /**
       * @brief Constructor
       * 
       * @param lhs Variable access on left hand side
       * @param rhs Variable access on right hand side
       *


       */
        swap_statement(variable_access::ptr lhs,
                       variable_access::ptr rhs);

        /**
       * @brief Deconstructor
       * 


       */
        ~swap_statement() override;

        /**
       * @brief Returns variable access on left hand side
       * 
       * @return Variable access
       *


       */
        [[nodiscard]] variable_access::ptr lhs() const;

        /**
       * @brief Returns variable access on right hand side
       * 
       * @return Variable access
       *


       */
        [[nodiscard]] variable_access::ptr rhs() const;

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

     */
    class unary_statement: public statement {
    public:
        /**
       * @brief Type of the statement
       *


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
       * 


       */
        unary_statement(unsigned             op,
                        variable_access::ptr var);

        /**
       * @brief Deconstructor
       *


       */
        ~unary_statement() override;

        /**
       * @brief Returns the operation of the statement
       * 
       * @return Operation
       *


       */
        [[nodiscard]] unsigned op() const;

        /**
       * @brief Returns the variable access of the statement
       * 
       * @return Variable access
       *


       */
        [[nodiscard]] variable_access::ptr var() const;

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

     */
    class assign_statement: public statement {
    public:
        /**
       * @brief Type of assignment
       *


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
       * @brief Constructor
       * 
       * @param lhs Variable access to which the operation is applied
       * @param op Operation to be applied
       * @param rhs Expression to be evaluated
       * 


       */
        assign_statement(variable_access::ptr lhs,
                         unsigned             op,
                         expression::ptr      rhs);

        /**
       * @brief Deconstructor
       *


       */
        ~assign_statement() override;

        /**
       * @brief Returns variable access to which the operation is applied
       * 
       * @return Variable access
       *


       */
        [[nodiscard]] variable_access::ptr lhs() const;

        /**
       * @brief Returns the expression to be evaluated
       * 
       * @return Expression
       *


       */
        [[nodiscard]] expression::ptr rhs() const;

        /**
       * @brief Returns the operation to be applied
       * 
       * 
       * @return Operation
       *


       */
        [[nodiscard]] unsigned op() const;

    private:
        class priv;
        priv* const d = nullptr;
    };

    /**
     * @brief IF Statement
     *
     * This class represents the SyReC \b if statement
     *

     */
    class if_statement: public statement {
    public:
        /**
       * @brief Standard constructor
       * 
       * Initializes default values
       *


       */
        if_statement();

        /**
       * @brief Deconstructor
       * 


       */
        ~if_statement() override;

        /**
       * @brief Sets the condition for the execution of the then_statements()
       *
       * The expression \p condition is assumed to have a bit-width of 1 bit.
       * 
       * @param condition Expression
       *


       */
        void set_condition(expression::ptr condition);

        /**
       * @brief Returns the condition for the execution of the then_statements()
       * 
       * @return Expression
       *


       */
        [[nodiscard]] expression::ptr condition() const;

        /**
       * @brief Adds a statement to the then branch
       * 
       * @param then_statement Statement to be executed in the if branch
       *


       */
        void add_then_statement(const statement::ptr& then_statement);

        /**
       * @brief Returns all statements in the if branch
       * 
       * @return List of statements
       *


       */
        [[nodiscard]] const statement::vec& then_statements() const;

        /**
       * @brief Adds a statement to the else branch
       * 
       * @param else_statement Statement to be executed in the else branch
       *


       */
        void add_else_statement(const statement::ptr& else_statement);

        /**
       * @brief Returns all statements in the else branch
       * 
       * @return List of statements
       *


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


       */
        void set_fi_condition(expression::ptr fi_condition);

        /**
       * @brief Returns the reverse condition for the execution of the if_statements()
       * 
       * @return Expression
       *


       */
        [[nodiscard]] expression::ptr fi_condition() const;

    private:
        class priv;
        priv* const d = nullptr;
    };

    /**
     * @brief FOR Statement
     *
     * This class represents the SyReC \b for statement
     *

     */
    class for_statement: public statement {
    public:
        /**
       * @brief Standard constructor
       *
       * Initilizes default values
       *


       */
        for_statement();

        /**
       * @brief Deconstructor
       *


       */
        ~for_statement() override;

        /**
       * @brief Sets the name of the loop variable.
       *
       * Setting a loop variable is optional.
       * 
       * @param loop_variable Name of the loop variable
       *


       */
        void set_loop_variable(const std::string& loop_variable);

        /**
       * @brief Returns the name of the loop variable
       * 
       * 
       * @return Name of the loop variable
       *


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


       */
        void set_range(const std::pair<number::ptr, number::ptr>& range);

        /**
       * @brief Returns the range of the loop
       * 
       * @return Range of the loop
       *


       */
        [[nodiscard]] const std::pair<number::ptr, number::ptr>& range() const;

        /**
       * @brief Returns the step of the loop
       * 
       * @return Step of the loop
       *


       */
        [[nodiscard]] const number::ptr& step() const;

        /**
       * @brief Adds a statement to be executed in the loop
       * 
       * @param statement Statement
       *


       */
        void add_statement(const statement::ptr& statement);

        /**
       * @brief Returns the statements to be executed in the loop
       * 
       * @return List of statements
       *


       */
        [[nodiscard]] const statement::vec& statements() const;

    private:
        class priv;
        priv* const d = nullptr;
    };

    /**
     * @brief CALL Statement
     *
     * This class represents the SyReC \b call statement to call a module.
     *

     */
    class call_statement: public statement {
    public:
        /**
       * @brief Constructor with module and parameters
       * 
       * @param target Module to call
       * @param parameters Parameters to assign
       *


       */
        call_statement(std::shared_ptr<module> target, const std::vector<std::string>& parameters);

        /**
       * @brief Deconstructor
       *


       */
        ~call_statement() override;

        /**
       * @brief Returns the target module to call
       * 
       * @return Module
       *


       */
        [[nodiscard]] std::shared_ptr<module> target() const;

        /**
       * @brief Returns the parameters to assign with the module call
       * 
       * @return List of parameter names
       *


       */
        [[nodiscard]] const std::vector<std::string>& parameters() const;

    private:
        class priv;
        priv* const d = nullptr;
    };

    /**
     * @brief UNCALL Statement
     *
     * This class represents the SyReC \b uncall statement to uncall a module.
     *

     */
    class uncall_statement: public statement {
    public:
        /**
       * @brief Constructor with module and parameters
       * 
       * @param target Module to uncall
       * @param parameters Parameters to assign
       *


       */
        uncall_statement(std::shared_ptr<module> target, const std::vector<std::string>& parameters);

        /**
       * @brief Deconstructor
       *


       */
        ~uncall_statement() override;

        /**
       * @brief Returns the target module to uncall
       * 
       * @return Module
       *


       */
        [[nodiscard]] std::shared_ptr<module> target() const;

        /**
       * @brief Returns the parameters to assign with the module uncall
       * 
       * @return List of parameter names
       *


       */
        [[nodiscard]] const std::vector<std::string>& parameters() const;

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

     */
    class skip_statement: public statement {
    public:
        /**
       * @brief Deconstructor
       *


       */
        ~skip_statement() override;
    };

    /**
 * @brief Reverse Statements
 */

    struct reverse_statements {
        typedef statement::ptr result_type;

        statement::ptr operator()(statement::ptr _statement) const;
    };

} // namespace syrec::applications

#endif /* STATEMENT_HPP */
