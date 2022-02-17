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
 * @file module.hpp
 *
 * @brief SyReC module data type
 */
#ifndef MODULE_HPP
#define MODULE_HPP

#include <boost/shared_ptr.hpp>
#include <core/syrec/statement.hpp>
#include <core/syrec/variable.hpp>
#include <iostream>
#include <string>
#include <vector>

namespace revkit {
    namespace syrec {

        class statement;

        /**
     * @brief SyReC module data type
     *
     * This class represents a SyReC module. It containes of a name(), parameters(),
     * local variables(), and a list of statements().
     *
     * @author RevKit
     * @since  1.1
     */
        class module {
        public:
            /**
       * @brief Smart pointer
       *
       * @author RevKit
       * @since  1.1
       */
            typedef std::shared_ptr<module> ptr;

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
            module();

            /**
       * @brief Constructor
       *
       * Initializes a module with a name
       *
       * @param name Name of the module
       *
       * @author RevKit
       * @since  1.1
       */
            explicit module(const std::string& name);

            /**
       * @brief Deconstructor
       */
            ~module();

            /**
       * @brief Sets the name of the module
       *
       * @param name Name
       *
       * @author RevKit
       * @since  1.1
       */
            void set_name(const std::string& name);

            /**
       * @brief Returns the name of the module
       *
       * @return Name
       *
       * @author RevKit
       * @since  1.1
       */
            const std::string& name() const;

            /**
       * @brief Adds a parameter to the module
       *
       * @param parameter Parameter
       *
       * @author RevKit
       * @since  1.1
       */
            void add_parameter(variable::ptr parameter);

            /**
       * @brief Returns all parameters of the module
       *
       * @return Vector of parameters
       *
       * @author RevKit
       * @since  1.1
       */
            const variable::vec& parameters() const;

            /**
       * @brief Adds a variable to the module
       *
       * @param variable Variable
       *
       * @author RevKit
       * @since  1.1
       */
            void add_variable(variable::ptr variable);

            /**
       * @brief Returns all variables of the module
       *
       * @return Vector of variables
       *
       * @author RevKit
       * @since  1.1
       */
            const variable::vec& variables() const;

            /**
       * @brief Finds a parameter or variable in the module
       *
       * This methods tries to find a parameter or a variable
       * by its name. If no such parameter or variable exists,
       * then the empty smart pointer variable::ptr() is returned.
       * Otherwise, using the \ref variable::type() "type" it can
       * be determined, whether it is a parameter of a variable.
       *
       * @author RevKit
       * @since  1.1
       */
            variable::ptr find_parameter_or_variable(const std::string& name) const;

            /**
       * @brief Adds a statement to the module
       *
       * @param statement Statement
       *
       * @author RevKit
       * @since  1.1
       */
            void add_statement(std::shared_ptr<statement> statement);

            /**
       * @brief Returns all statements of the module
       *
       * @return Vector of statements
       *
       * @author RevKit
       * @since  1.1
       */
            const std::vector<std::shared_ptr<statement>>& statements() const;

        private:
            class priv;
            priv* const d;
        };

        /**
     * @brief Prints module to output stream
     *
     * @param os Output Stream
     * @param m Module
     *
     * @return Output Stream
     *
     * @author RevKit
     * @since  1.1
     */
        std::ostream& operator<<(std::ostream& os, const module& m);

    } // namespace syrec
} // namespace revkit

#endif /* MODULE_HPP */
