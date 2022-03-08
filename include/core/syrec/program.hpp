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

#include "core/syrec/module.hpp"

#include <vector>

namespace syrec::applications {

    /**
     * @brief SyReC program
     *
     * This class represents a SyReC program, which
     * is a collection of modules.
     *

     */
    class program {
    public:
        /**
       * @brief Standard constructor
       * 
       * Initializes default values
       *


       */
        program();

        /**
       * @brief Deconstructor
       *


       */
        ~program();

        /**
       * @brief Adds a module to the program
       * 
       * @param module Module
       *


       */
        void add_module(const module::ptr& module);

        /**
       * @brief Returns all modules of the program
       *
       * @return List of modules 
       *


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


       */
        [[nodiscard]] module::ptr find_module(const std::string& name) const;

    private:
        class priv;
        priv* const d = nullptr;
    };

} // namespace syrec::applications

#endif /* PROGRAM_HPP */
