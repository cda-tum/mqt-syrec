#pragma once

#include "core/syrec/variable.hpp"

#include <iostream>
#include <memory>
#include <utility>

namespace syrec {

    /**
     * @brief SyReC expression
     *
     * This abstract class represents the base of an SyReC expression.
     * Eeach expression is derived from this class and has to
     * implement the bitwidth() and the print() methods.
     */
    struct expression {
        /**
       * @brief Shared Pointer interface to the class
       */
        typedef std::shared_ptr<expression> ptr;

        typedef std::vector<ptr> vec;

        /**
       * @brief Standard constructor
       */
        expression() = default;

        /**
       * @brief Standard destructor
       */
        virtual ~expression() = default;

        /**
       * @brief Bit-width of the expression
       *
       * This method returns the bit-width of an expression.
       *
       * @return Bit-width of the expression
       */
        [[nodiscard]] virtual unsigned bitwidth() const = 0;
    };

    /**
     * @brief Numeric Expression
     */
    struct numeric_expression: public expression {
        /**
       * @brief Operation to perform in case of binary numeric expression
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
       */
        numeric_expression(number::ptr value, unsigned bitwidth):
            value(std::move(value)), bwidth(bitwidth) {}

        [[nodiscard]] unsigned bitwidth() const override {
            return bwidth;
        }

        number::ptr value = nullptr;
        unsigned    bwidth{};
    };

    /**
     * @brief Variable expression
     *
     * This class represents a variable expression and
     * capsulates a variable access pointer var().
     */
    struct variable_expression: public expression {
        /**
       * @brief Constructor with variable
       * 
       * @param var Variable access
       */
        explicit variable_expression(variable_access::ptr var):
            var(std::move(var)) {}

        [[nodiscard]] unsigned bitwidth() const override {
            return var->bitwidth();
        }

        variable_access::ptr var = nullptr;
    };

    /**
     * @brief Binary expression
     *
     * This class represents a binary expression between two sub
     * expressions lhs() and rhs() by an operation op(). 
     */
    struct binary_expression: public expression {
        /**
       * @brief Operation to perform
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
       */
        binary_expression(expression::ptr lhs,
                          unsigned        op,
                          expression::ptr rhs):
            lhs(std::move(lhs)),
            op(op), rhs(std::move(rhs)) {}

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
       */
        [[nodiscard]] unsigned bitwidth() const override {
            switch (op) {
                case logical_and:
                case logical_or:
                case less_than:
                case greater_than:
                case equals:
                case not_equals:
                case less_equals:
                case greater_equals:
                    return 1;

                default:
                    // lhs and rhs are assumed to have the same bit-width
                    return lhs->bitwidth();
            }
        }

        expression::ptr lhs = nullptr;
        unsigned        op{};
        expression::ptr rhs = nullptr;
    };

    /**
     * @brief Shift expression
     *
     * This class represents a binary expression with a
     * sub-expression lhs() and a number rhs() by a shift operation op(). 
     */
    struct shift_expression: public expression {
        /**
       * @brief Shft Operation
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
       */
        shift_expression(expression::ptr lhs,
                         unsigned        op,
                         number::ptr     rhs):
            lhs(std::move(lhs)),
            op(op), rhs(std::move(rhs)) {}

        /**
       * @brief Returns the bit-width of the expression
       *
       * The bit-width of a shift expression is the bit-width
       * of the lhs sub-expression.
       * 
       * @return Bit-width of the expression
       */
        [[nodiscard]] unsigned bitwidth() const override {
            return lhs->bitwidth();
        }

        expression::ptr lhs = nullptr;
        unsigned        op{};
        number::ptr     rhs = nullptr;
    };

} // namespace syrec
