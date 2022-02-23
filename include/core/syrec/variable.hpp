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
 * @file variable.hpp
 *
 * @brief SyReC variable data type
 */
#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include <optional>
#include <memory>
#include <core/syrec/number.hpp>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace revkit {
    namespace syrec {

        class expression;

        /**
     * @brief SyReC variable data type
     *
     * This class represents variable data type. A variable
     * can either be a parameter passed to a module, or a
     * local variable.
     *
     * @author RevKit
     * @since  1.1
     */
        class variable {
        public:
            /**
       * @brief Type of variable
       *
       * @author RevKit
       * @since  1.1
       */
            enum types {
                /**
         * @brief Module Input Parameter (garbage output)
         */
                in,

                /**
         * @brief Module Output Parameter (constant inputs with value 0)
         */
                out,

                /**
         * @brief Module Input/Output Parameter
         */
                inout,

                /**
         * @brief State Variable (local in top module)
         */
                state,

                /**
         * @brief Local variable (constant inputs with value 0 and garbage output)
         */
                wire
            };

            /**
       * @brief Smart pointer
       *
       * @author RevKit
       * @since  1.1
       */
            typedef std::shared_ptr<variable> ptr;

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
            variable();

            /**
       * @brief Constructor
       *
       * Initializes the type, the name, and the bit-width of the variable
       *
       * @param type Type
       * @param name Name
       * @param bitwidth Bitwidth of the variable (Default value: 32)
       *
       * @author RevKit
       * @since  1.1
       */
            variable(unsigned type, const std::string& name, unsigned bitwidth);

            variable(unsigned type, const std::string& name, const std::vector<unsigned>& dimensions, unsigned bitwidth);

            /**
       * @brief Deconstructor
       *
       * @author RevKit
       * @since  1.1
       */
            ~variable();

            /**
       * @brief Sets the type of the variable
       *
       * @param type Type
       *
       * @author RevKit
       * @since  1.1
       */
            void set_type(unsigned type);

            /**
       * @brief Returns the type of the variable
       *
       * @return Type
       *
       * @author RevKit
       * @since  1.1
       */
            unsigned type() const;

            /**
       * @brief Sets the name of the variable
       *
       * @param name Name
       *
       * @author RevKit
       * @since  1.1
       */
            void set_name(const std::string& name);

            /**
       * @brief Returns the name of the variable
       *
       * @return Name
       *
       * @author RevKit
       * @since  1.1
       */
            const std::string& name() const;

            /**
       * @brief Sets the bit-width of the variable
       *
       * @param bitwidth Bit-width
       *
       * @author RevKit
       * @since  1.1
       */
            void set_bitwidth(unsigned bitwidth);

            /**
       * @brief Returns the bit-width of the variable
       *
       * @return Bit-width
       *
       * @author RevKit
       * @since  1.1
       */
            unsigned bitwidth() const;

            void          set_reference(variable::ptr reference);
            variable::ptr reference() const;

            void                         set_dimensions(const std::vector<unsigned>& dimensions);
            const std::vector<unsigned>& dimensions() const;

        private:
            class priv;
            priv* const d = nullptr;
        };

        /**
     * @brief Variable Access
     *
     * This class represents the access of a variable inside
     * a statement or an expression.
     *
     * @author RevKit
     * @since  1.1
     */
        class variable_access {
        public:
            /**
       * @brief Smart pointer
       *
       * @author RevKit
       * @since  1.1
       */
            typedef std::shared_ptr<variable_access> ptr;

            /**
       * @brief Standard constructor
       *
       * Initializes default values
       *
       * @author RevKit
       * @since  1.1
       */
            variable_access();

            /**
       * @brief Constructor with variable
       *
       * @param var Variable
       *
       * @author RevKit
       * @since  1.1
       */
            variable_access(variable::ptr var);

            /**
       * @brief Deconstructor
       *
       * @author RevKit
       * @since  1.1
       */
            ~variable_access();

            /**
       * @brief Sets the variable of this access
       *
       * @param var Variable
       *
       * @author RevKit
       * @since  1.1
       */
            void set_var(variable::ptr var);

            /**
       * @brief Returns the variable of this access
       *
       * @return Variable
       *
       * @author RevKit
       * @since  1.1
       */
            variable::ptr var() const;

            /**
       * @brief Sets the range of this access
       *
       * If the parameter is empty, i.e. the default vaule @code boost::optional<std::pair<number::ptr, number::ptr> >() @endcode
       * the full variable is considered. Otherwise, the values of the numbers are evaluated for determining the range.
       *
       * The first number can be larger than the second one. In this case, the range is also considered backwards.
       *
       * @param range Range
       *
       * @author RevKit
       * @since  1.1
       */
            void set_range(const std::optional<std::pair<number::ptr, number::ptr>>& range);

            /**
       * @brief Returns the range of this access
       *
       * @return Range
       *
       * @author RevKit
       * @since  1.1
       */
            const std::optional<std::pair<number::ptr, number::ptr>>& range() const;

            /**
       * @brief Returns the bit-width of the variable access
       *
       * The bit-width can only be evaluated, if
       * - both bounds in the range are no loop variables, or
       * - both bounds in the range are the same loop variable.
       * In other cases, this message throws an assertion.
       *
       * @return Bit-width of the range, if possible to calculate
       *
       * @author RevKit
       * @since  1.1
       */
            unsigned bitwidth() const;

            void                                            set_indexes(const std::vector<std::shared_ptr<expression>>& indexes);
            const std::vector<std::shared_ptr<expression>>& indexes() const;

        private:
            class priv;
            priv* const d = nullptr;
        };

        /**
     * @brief Prints variable to output stream
     *
     * @param os Output Stream
     * @param v Variable
     *
     * @return Output Stream
     *
     * @author RevKit
     * @since  1.1
     */
        std::ostream& operator<<(std::ostream& os, const variable& v);

        /**
     * @brief Prints variable access to output stream
     *
     * @param os Output Stream
     * @param v Variable access
     *
     * @return Output Stream
     *
     * @author RevKit
     * @since  1.1
     */
        std::ostream& operator<<(std::ostream& os, const variable_access& v);

    } // namespace syrec
} // namespace revkit

#endif /* VARIABLE_HPP */
