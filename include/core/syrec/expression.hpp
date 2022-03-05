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
 * @file expression.hpp
 *
 * @brief SyReC expression data types
 */
#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include "core/syrec/variable.hpp"

#include <iostream>
#include <memory>

namespace syrec::applications {

    /**
     * @brief SyReC expression
     *
     * This abstract class represents the base of an SyReC expression.
     * Eeach expression is derived from this class and has to
     * implement the bitwidth() and the print() methods.
     *
     * @author RevKit
     * @since  1.1
     */
    class expression {
    public:
        /**
       * @brief Shared Pointer interface to the class
       *
       * @author RevKit
       * @since  1.1
       */
        typedef std::shared_ptr<expression> ptr;

        typedef std::vector<ptr> vec;

        /**
       * @brief Standard constructor
       *
       * @author RevKit
       * @since  1.1
       */
        expression();

        /**
       * @brief Standard deconstructor
       *
       * @author RevKit
       * @since  1.1
       */
        virtual ~expression();

        /**
       * @brief Bit-width of the expression
       *
       * This method returns the bit-width of an expression.
       *
       * @return Bit-width of the expression
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] virtual unsigned bitwidth() const = 0;

        /**
       * @brief Helper function used by the ostream operator<< function
       *
       * This method has to be implemented by all child
       * classes in order to print the expression to an output
       * stream using the << operator.
       *
       * @param os Output Stream
       *
       * @author RevKit
       * @since  1.1
       *
       * @return Output Stream
       */
        // virtual std::ostream& print(std::ostream& os) const = 0;

    private:
        class priv;
        priv* const d = nullptr;
    };

    /**
     * @brief Numeric Expression
     *
     * @author RevKit
     * @since  1.1
     */
    class numeric_expression: public expression {
    public:
        /**
       * @brief Operation to perform in case of binary numeric expression
       *
       * @author RevKit
       * @since  1.1
       */
        enum {
            /**
         * @brief Addition
         */
            add,

            /**
         * @brief Subtraction
         */
            subtract,

            /**
         * @brief Multiplication
         */
            multiply,

            /**
         * @brief Division
         */
            divide,

            modulo,

            logical_and,

            logical_or,

            bitwise_and,

            bitwise_or,

            less_than,

            greater_than,

            less_equals,

            greater_equals,

            equals,

            not_equals

        };
        /**
       * @brief Standard Constructor
       *
       * @author RevKit
       * @since  1.1
       */
        numeric_expression();

        /**
       * @brief Creates a numeric expression with a value and a bit-width
       *
       * @param value Value
       * @param bitwidth Bit-width of the value
       *
       * @author RevKit
       * @since  1.1
       */
        numeric_expression(const number::ptr& value, unsigned bitwidth);

        /**
       * @brief Standard Deconstructor
       *
       * @author RevKit
       * @since  1.1
       */
        ~numeric_expression() override;

        /**
       * @brief Sets the value of the expression
       *
       * @param value Value
       *
       * @author RevKit
       * @since  1.1
       */
        //void set_value(const number::ptr& value);

        /**
       * @brief Returns the value of the expression
       *
       * @return Value of the expression
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] const number::ptr& value() const;

        /**
       * @brief Returns the bit-width of the expression
       *
       * @return Bit-width
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] unsigned bitwidth() const override;

        /**
       * @brief Prints the expression to an output stream
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
     * @brief Variable expression
     *
     * This class represents a variable expression and
     * capsulates a variable access pointer var().
     *
     * @author RevKit
     * @since  1.1
     */
    class variable_expression: public expression {
    public:
        /**
       * @brief Standard constructor
       * 
       * Initializes default values
       *
       * @author RevKit
       * @since  1.1
       */
        variable_expression();

        /**
       * @brief Constructor with variable
       * 
       * @param var Variable access
       * 
       * @author RevKit
       * @since  1.1
       */
        explicit variable_expression(variable_access::ptr var);

        /**
       * @brief Deconstructor
       *
       * @author RevKit
       * @since  1.1
       */
        ~variable_expression() override;

        /**
       * @brief Sets the variable of the expression
       * 
       * @param var Variable access
       *
       * @author RevKit
       * @since  1.1
       */
        //void set_var(variable_access::ptr var);

        /**
       * @brief Returns the variable of the expression
       * 
       * @return Variable access
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] variable_access::ptr var() const;

        /**
       * @brief Returns the bit-width of the variable access
       * 
       * This method calls variable_access::bitwidth() internally.
       *
       * @return Bit-width of the variable access
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] unsigned bitwidth() const override;

        /**
       * @brief Prints the expression to an output stream
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
     * @brief Binary expression
     *
     * This class represents a binary expression between two sub
     * expressions lhs() and rhs() by an operation op(). 
     *
     * @author RevKit
     * @since  1.1
     */
    class binary_expression: public expression {
    public:
        /**
       * @brief Operation to perform
       *
       * @author RevKit
       * @since  1.1
       */
        enum {
            /**
         * @brief Addition
         */
            add,

            /**
         * @brief Subtraction
         */
            subtract,

            /**
         * @brief Bit-wise EXOR
         */
            exor,

            /**
         * @brief Multiplication
         *
         * Returns n least significant bits, where n is the bit-width of lhs() and rhs()
         */
            multiply,

            /**
         * @brief Division
         *
         * Returns n least significant bits, where n is the bit-width of lhs() and rhs()
         */
            divide,

            /**
         * @brief Modulo Operation
         */
            modulo,

            /**
         * @brief Multiplication (most significant bits)
         *
         * Performs multiplication and returns the n most significant bits, where n is the bit-width of lhs() and rhs()
         */
            frac_divide,

            /**
         * @brief Logical AND
         */
            logical_and,

            /**
         * @brief Logical OR
         */
            logical_or,

            /**
         * @brief Bitwise AND
         */
            bitwise_and,

            /**
         * @brief Bitwise OR
         */
            bitwise_or,

            /**
         * @brief Less than
         */
            less_than,

            /**
         * @brief Greater than
         */
            greater_than,

            /**
         * @brief Equals
         */
            equals,

            /**
         * @brief Not equals
         */
            not_equals,

            /**
         * @brief Less equals
         */
            less_equals,

            /**
         * @brief Greater equals
         */
            greater_equals
        };

        /**
       * @brief Standard constructor
       * 
       * Initializes default values
       *
       * @author RevKit
       * @since  1.1
       */
        binary_expression();

        /**
       * @brief Constructor which initializes a operation
       * 
       * @param lhs Expression on left hand side
       * @param op Operation to be performed
       * @param rhs Expression on right hand side
       * 
       * @author RevKit
       * @since  1.1
       */
        binary_expression(expression::ptr lhs,
                          unsigned        op,
                          expression::ptr rhs);

        /**
       * @brief Deconstructor
       *
       * @author RevKit
       * @since  1.1
       */
        ~binary_expression() override;

        /**
       * @brief Sets the left hand side of the expression
       * 
       * @param lhs Expression
       *
       * @author RevKit
       * @since  1.1
       */
        //[[maybe_unused]] void set_lhs(expression::ptr lhs);

        /**
       * @brief Returns the left hand side of the expression
       * 
       * @return Expression
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] expression::ptr lhs() const;

        /**
       * @brief Sets the right hand side of the expression
       * 
       * @param rhs Expression
       *
       * @author RevKit
       * @since  1.1
       */
        //[[maybe_unused]] void set_rhs(expression::ptr rhs);

        /**
       * @brief Returns the right hand side of the expression
       * 
       * @return Expression
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] expression::ptr rhs() const;

        /**
       * @brief Sets the operation to be performed
       * 
       * @param op Operation
       *
       * @author RevKit
       * @since  1.1
       */
        //void set_op(unsigned op);

        /**
       * @brief Returns the operation to be performed
       *
       * @return Operation
       * 
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] unsigned op() const;

        /**
       * @brief Bit-width of the expression
       * 
       * It is assumed that the bit-width of the left
       * hand side and the right hand side are the same.
       * Thus, the bit-width of the lhs() expression is
       * returned. If a logical operation is performed, 
       * however, the returned bit-width is always 1.
       * 
       * @return Bit-width of the expression
       *
       * @author RevKit
       * @since  1.1
       */
        [[nodiscard]] unsigned bitwidth() const override;

        /**
       * @brief Prints the expression to an output stream
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
     * @brief Unary expression
     * 
     * This class represents a unary expression composed of a 
     * sub expression expr() and an operation op().
     * 
     * @author RevKit
     * @since 1.3
     */
    //class unary_expression: public expression {
    //public:
    /**
       * @brief Operation to perform
       *
       * @author RevKit
       * @since  1.3
       */
    //enum {
    /**
         * @brief Logical NOT
         *
         * Returns 1 iff expr() evaluates to 0.
         */
    //   logical_not,
    /**
         * @brief Bitwise NOT
         */
    //   bitwise_not
    //};

    /**
       * @brief Standard constructor
       * 
       * Initializes default values
       *
       * @author RevKit
       * @since  1.3
       */
    //unary_expression();

    /**
       * @brief Constructor which initializes an operation
       * 
       * @param op Operation to be performed
       * @param expr Expression to perform the operation on
       * 
       * @author RevKit
       * @since  1.3
       */
    //unary_expression(unsigned op, expression::ptr expr);

    /**
       * @brief Deconstructor
       *
       * @author RevKit
       * @since  1.3
       */
    //~unary_expression() override;

    /**
       * @brief Sets the sub expression
       * 
       * @param expr Expression
       *
       * @author RevKit
       * @since  1.3
       */
    //void set_expr(expression::ptr expr);

    /**
       * @brief Returns the sub expression
       * 
       * @return Expression
       *
       * @author RevKit
       * @since  1.3
       */
    //[[nodiscard]] expression::ptr expr() const;

    /**
       * @brief Sets the operation to be performed
       * 
       * @param op Operation
       *
       * @author RevKit
       * @since  1.3
       */
    //void set_op(unsigned op);

    /**
       * @brief Returns the operation to be performed
       *
       * @return Operation
       * 
       * @author RevKit
       * @since  1.3
       */
    //[[nodiscard]] unsigned op() const;

    /**
       * @brief Bit-width of the expression
       * 
       * If a logical operation is performed, 
       * the returned bit-width is always 1. 
       * Otherwise, the bit-width of the sub 
       * expression is returned.
       * 
       * @return Bit-width of the expression
       *
       * @author RevKit
       * @since  1.3
       */
    //[[nodiscard]] unsigned bitwidth() const override;

    /**
       * @brief Prints the expression to an output stream
       *
       * @param os Output stream
       *
       * @return Output stream
       *
       * @author RevKit
       * @since  1.3
       */
    //std::ostream& print(std::ostream& os) const override;

    /* private:
        class priv;
        priv* const d = nullptr;
    };*/

    /**
     * @brief Shift expression
     *
     * This class represents a binary expression with a
     * sub-expression lhs() and a number rhs() by a shift operation op(). 
     *
     * @author RevKit
     * @since  1.1
     */
    class shift_expression: public expression {
    public:
        /**
       * @brief Shft Operation
       * 
       * @author RevKit
       * @since  1.1 
       */
        enum {
            /**
         * @brief Left-Shift
         */
            left,

            /**
         * @brief Right-Shift
         */
            right
        };

        /**
       * @brief Standard constructor
       * 
       * Initializes default values
       *
       * @author RevKit
       * @since  1.1 
       */
        shift_expression();

        /**
       * @brief Constructor
       *
       * This constructs a shift expression with a sub-expression
       * as the \p lhs, a shift operation \p os, and a number
       * \p rhs.
       * 
       * @param lhs Expression to be shifted 
       * @param op Shift operation
       * @param rhs Number of bits to shift
       * 
       * @author RevKit
       * @since  1.1
       */
        shift_expression(expression::ptr    lhs,
                         unsigned           op,
                         const number::ptr& rhs);

        /**
       * @brief Deconstructor
       * 
       * @author RevKit
       * @since  1.1 
       */
        ~shift_expression() override;

        /**
       * @brief Sets the left-hand side expression
       * 
       * @param lhs Expression
       *
       * @author RevKit
       * @since  1.1
       */
        //[[maybe_unused]] void set_lhs(expression::ptr lhs);

        /**
       * @brief Returns the left-hand side expression
       * 
       * @return Expression
       *
       * @author RevKit
       * @since  1.1 
       */
        [[nodiscard]] expression::ptr lhs() const;

        /**
       * @brief Sets the number of bits to shift
       * 
       * @param rhs Number
       *
       * @author RevKit
       * @since  1.1
       */
        //[[maybe_unused]] void set_rhs(const number::ptr& rhs);

        /**
       * @brief Returns the number of bits to shift
       * 
       * @return Number
       *
       * @author RevKit
       * @since  1.1 
       */
        [[nodiscard]] const number::ptr& rhs() const;

        /**
       * @brief Sets the shift operation
       * 
       * @param op Shift operation
       *
       * @author RevKit
       * @since  1.1
       */
        //void set_op(unsigned op);

        /**
       * @brief Returns the shift operation
       * 
       * @return Shift operation
       *
       * @author RevKit
       * @since  1.1 
       */
        [[nodiscard]] unsigned op() const;

        /**
       * @brief Returns the bit-width of the expression
       *
       * The bit-width of a shift expression is the bit-width
       * of the lhs sub-expression.
       * 
       * @return Bit-width of the expression
       *
       * @author RevKit
       * @since  1.1 
       */
        [[nodiscard]] unsigned bitwidth() const override;

        /**
       * @brief Prints the expression to an output stream
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
     * @brief Prints an expression
     * 
     * @param os Output stream
     * @param e Expression
     * 
     * @return Output stream
     *
     * @author RevKit
     * @since  1.1
     */
    // std::ostream& operator<<(std::ostream& os, const expression& e);

} // namespace syrec::applications

#endif /* EXPRESSION_HPP */
