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

     */
    class expression {
    public:
        /**
       * @brief Shared Pointer interface to the class
       *


       */
        typedef std::shared_ptr<expression> ptr;

        typedef std::vector<ptr> vec;

        /**
       * @brief Standard constructor
       *


       */
        expression();

        /**
       * @brief Standard deconstructor
       *


       */
        virtual ~expression();

        /**
       * @brief Bit-width of the expression
       *
       * This method returns the bit-width of an expression.
       *
       * @return Bit-width of the expression
       *


       */
        [[nodiscard]] virtual unsigned bitwidth() const = 0;

    private:
        class priv;
        priv* const d = nullptr;
    };

    /**
     * @brief Numeric Expression
     *

     */
    class numeric_expression: public expression {
    public:
        /**
       * @brief Operation to perform in case of binary numeric expression
       *


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
       * @brief Creates a numeric expression with a value and a bit-width
       *
       * @param value Value
       * @param bitwidth Bit-width of the value
       *


       */
        numeric_expression(const number::ptr& value, unsigned bitwidth);

        /**
       * @brief Standard Deconstructor
       *


       */
        ~numeric_expression() override;

        /**
       * @brief Returns the value of the expression
       *
       * @return Value of the expression
       *


       */
        [[nodiscard]] const number::ptr& value() const;

        /**
       * @brief Returns the bit-width of the expression
       *
       * @return Bit-width
       *


       */
        [[nodiscard]] unsigned bitwidth() const override;

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

     */
    class variable_expression: public expression {
    public:
        /**
       * @brief Constructor with variable
       * 
       * @param var Variable access
       * 


       */
        explicit variable_expression(variable_access::ptr var);

        /**
       * @brief Deconstructor
       *


       */
        ~variable_expression() override;

        /**
       * @brief Returns the variable of the expression
       * 
       * @return Variable access
       *


       */
        [[nodiscard]] variable_access::ptr var() const;

        /**
       * @brief Returns the bit-width of the variable access
       * 
       * This method calls variable_access::bitwidth() internally.
       *
       * @return Bit-width of the variable access
       *


       */
        [[nodiscard]] unsigned bitwidth() const override;

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

     */
    class binary_expression: public expression {
    public:
        /**
       * @brief Operation to perform
       *


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
       * @brief Constructor which initializes a operation
       * 
       * @param lhs Expression on left hand side
       * @param op Operation to be performed
       * @param rhs Expression on right hand side
       * 


       */
        binary_expression(expression::ptr lhs,
                          unsigned        op,
                          expression::ptr rhs);

        /**
       * @brief Deconstructor
       *


       */
        ~binary_expression() override;

        /**
       * @brief Returns the left hand side of the expression
       * 
       * @return Expression
       *


       */
        [[nodiscard]] expression::ptr lhs() const;

        /**
       * @brief Returns the right hand side of the expression
       * 
       * @return Expression
       *


       */
        [[nodiscard]] expression::ptr rhs() const;

        /**
       * @brief Returns the operation to be performed
       *
       * @return Operation
       * 


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


       */
        [[nodiscard]] unsigned bitwidth() const override;

    private:
        class priv;
        priv* const d = nullptr;
    };

    /**
     * @brief Shift expression
     *
     * This class represents a binary expression with a
     * sub-expression lhs() and a number rhs() by a shift operation op(). 
     *

     */
    class shift_expression: public expression {
    public:
        /**
       * @brief Shft Operation
       * 


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


       */
        shift_expression(expression::ptr    lhs,
                         unsigned           op,
                         const number::ptr& rhs);

        /**
       * @brief Deconstructor
       * 


       */
        ~shift_expression() override;

        /**
       * @brief Returns the left-hand side expression
       * 
       * @return Expression
       *


       */
        [[nodiscard]] expression::ptr lhs() const;

        /**
       * @brief Returns the number of bits to shift
       * 
       * @return Number
       *


       */
        [[nodiscard]] const number::ptr& rhs() const;

        /**
       * @brief Returns the shift operation
       * 
       * @return Shift operation
       *


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


       */
        [[nodiscard]] unsigned bitwidth() const override;

    private:
        class priv;
        priv* const d = nullptr;
    };

} // namespace syrec::applications

#endif /* EXPRESSION_HPP */
