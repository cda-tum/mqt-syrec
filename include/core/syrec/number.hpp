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
 * @file number.hpp
 *
 * @brief SyReC number data type
 */
#ifndef NUMBER_HPP
#define NUMBER_HPP

#include <map>
#include <memory>
#include <string>

namespace revkit {
    namespace syrec {
        class binary_numeric_expr;

        /**
     * @brief SyReC number data type
     *
     * This class represents a number in the context of a SyReC
     * program. A number can be an integer value, the current
     * value of a loop variable or a binary conjunction (+,-,*,/)
     * of two numbers. The current value of a loop variable
     * can only be determined in an execution context of the program,
     * i.e. in a synthesis procedure.
     *
     * @author RevKit
     * @since  1.1
     */
        class number {
        public:
            /**
       * @brief Smart pointer
       *
       * @author RevKit
       * @since  1.1
       */
            typedef std::shared_ptr<number> ptr;

            /**
       * @brief Loop Variable Mapping
       *
       * A loop variable mapping assigns a current value
       * to a loop variable by name. A loop variable mapping
       * is required in the evaluation of a variable.
       *
       * @author RevKit
       * @since  1.1
       */
            typedef std::map<std::string, unsigned> loop_variable_mapping;

            /**
       * @brief Construct by number
       *
       * This constructor generates a number based on the value \p value.
       *
       * @param value Value
       *
       * @author RevKit
       * @since  1.1
       */
            explicit number(unsigned value);

            /**
       * @brief Construct by loop variable
       *
       * This constructor generates a number based on a loop variable with the name \p value.
       *
       * @param value Name of the loop variable
       *
       * @author RevKit
       * @since  1.1
       */
            explicit number(const std::string& value);

            /**
       * @brief Construct by binary conjunction
       *
       * This constructor generates a number based on two numbers \p lhs and \p rhs and 
       * the conjunction \p op.
       *
       * @param lhs Number on the left-hand side
       * @param op Binary conjunction 
       * @param rhs Number on the right-hand side
       *
       * @author RevKit
       * @since  1.3
       */
            explicit number(const number::ptr lhs, const unsigned op, const number::ptr rhs);

            /**
       * @brief Deconstructor
       *
       * @author RevKit
       * @since  1.1
       */
            ~number();

            /**
       * @brief Returns whether the number is a loop variable
       *
       * @return true, if the number is a loop variable
       *
       * @author RevKit
       * @since  1.1
       */
            bool is_loop_variable() const;

            /**
       * @brief Returns whether the number is a conjunction of two numbers
       *
       * @return true, if the number is a conjunction
       *
       * @author RevKit
       * @since  1.3
       */
            bool is_conjunction() const;

            /**
       * @brief Returns whether the number is a known constant number
       *
       * @return true, if the number is a known constant number
       *
       * @author RevKit
       * @since  1.3
       */
            bool is_constant() const;

            /**
       * @brief Returns the variable name in case it is a loop variable
       *
       * This method can only be called when is_loop_variable() returns true,
       * otherwise this method throws an assertion and causes program
       * termination.
       *
       * @return The name of the loop variable
       * 
       * @author RevKit
       * @since  1.1
       */
            const std::string& variable_name() const;

            /**
       * @brief Returns the binary expression in case it is a conjunction of two numbers
       * 
       * This method can only be called if is_conjunction() returns true, 
       * otherwise this method throws an assertion and causes program 
       * termination.
       * 
       * @return The represented binary expression
       * 
       * @author RevKit
       * @since 1.3
       */
            binary_numeric_expr* conjunction_expr() const;

            /**
       * @brief Evaluates the number in the context of an execution
       * 
       * This method works as follows. If the number is represented
       * by an integer, it is simply returned. If in contrast, the
       * number is represented by a loop variable, its current value
       * is looked up in \p map. In this case an entry for this loop
       * variable must exist. Otherwise, an assertion is thrown.
       * If the number is a conjunction (+,-,*,/) of two numbers, the 
       * result is calculated and returned.
       *
       * @param map Loop variable map to obtain the current values
       *            for loop variables
       * 
       * @return The evaluated number
       *
       * @author RevKit
       * @since  1.1
       */
            unsigned evaluate(const loop_variable_mapping& map) const;

            /** @cond 0 */
            friend std::ostream& operator<<(std::ostream& os, const number& n);
            /** @endcond */

        private:
            class priv;
            priv* const d = nullptr;
        };

        /**
     * @brief Prints the number to an output stream
     *
     * @param os Output stream
     * @param n Number
     *
     * @return Output stream
     *
     * @author RevKit
     * @since  1.1
     */
        std::ostream& operator<<(std::ostream& os, const number& n);

    } // namespace syrec
} // namespace revkit

#endif /* NUMBER_HPP */
