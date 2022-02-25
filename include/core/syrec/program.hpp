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
 * @file program.hpp
 *
 * @brief SyReC program
 */
#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include <core/syrec/module.hpp>
#include <vector>

namespace revkit::syrec {

        /**
     * @brief SyReC program
     *
     * This class represents a SyReC program, which
     * is a collection of modules.
     *
     * @author RevKit
     * @since  1.1
     */
        class program {
        public:
            /**
       * @brief Standard constructor
       * 
       * Initializes default values
       *
       * @author RevKit
       * @since  1.1
       */
            program();

            /**
       * @brief Deconstructor
       *
       * @author RevKit
       * @since  1.1
       */
            ~program();

            /**
       * @brief Adds a module to the program
       * 
       * @param module Module
       *
       * @author RevKit
       * @since  1.1
       */
            void add_module(module::ptr module);

            /**
       * @brief Returns all modules of the program
       *
       * @return List of modules 
       *
       * @author RevKit
       * @since  1.1
       */
            [[nodiscard]] const module::vec& modules() const;

            /**
       * @brief Finds a module by its name
       * 
       * @param name Name of the module
       * 
       * @return Returns a smart pointer to the module if there is a module with the name \p name.
       *         Otherwise, the empty smart pointer is returned.
       *
       * @author RevKit
       * @since  1.1
       */
            [[nodiscard]] module::ptr find_module(const std::string& name) const;

        private:
            class priv;
            priv* const d = nullptr;
        };

        /**
     * @brief Prints a program to an output stream
     * 
     * @param os Output stream
     * @param p Program
     * 
     * @return Output stream
     *
     * @author RevKit
     * @since  1.1
     */
        std::ostream& operator<<(std::ostream& os, const program& p);

    } // namespace revkit

#endif /* PROGRAM_HPP */
