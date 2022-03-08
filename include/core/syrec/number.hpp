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

namespace syrec::applications {
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

     */
    class number {
    public:
        /**
       * @brief Smart pointer
       *


       */
        typedef std::shared_ptr<number> ptr;

        /**
       * @brief Loop Variable Mapping
       *
       * A loop variable mapping assigns a current value
       * to a loop variable by name. A loop variable mapping
       * is required in the evaluation of a variable.
       *


       */
        typedef std::map<std::string, unsigned> loop_variable_mapping;

        /**
       * @brief Construct by number
       *
       * This constructor generates a number based on the value \p value.
       *
       * @param value Value
       *


       */
        explicit number(unsigned value);

        /**
       * @brief Construct by loop variable
       *
       * This constructor generates a number based on a loop variable with the name \p value.
       *
       * @param value Name of the loop variable
       *


       */
        explicit number(const std::string& value);

        /**
       * @brief Deconstructor
       *


       */
        ~number();

        /**
       * @brief Returns whether the number is a loop variable
       *
       * @return true, if the number is a loop variable
       *


       */
        [[nodiscard]] bool is_loop_variable() const;

        /**
       * @brief Returns whether the number is a known constant number
       *
       * @return true, if the number is a known constant number
       *


       */
        [[nodiscard]] bool is_constant() const;

        /**
       * @brief Returns the variable name in case it is a loop variable
       *
       * This method can only be called when is_loop_variable() returns true,
       * otherwise this method throws an assertion and causes program
       * termination.
       *
       * @return The name of the loop variable
       * 


       */
        [[nodiscard]] const std::string& variable_name() const;

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


       */
        [[nodiscard]] unsigned evaluate(const loop_variable_mapping& map) const;

    private:
        class priv;
        priv* const d = nullptr;
    };

} // namespace syrec::applications

#endif /* NUMBER_HPP */
