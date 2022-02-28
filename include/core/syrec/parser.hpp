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
 * @file parser.hpp
 *
 * @brief SyReC parser routines
 */
#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>

namespace syrec {

    /**
   * @brief SyReC Namespace
   */
    namespace applications {
        class program;
    }

    /**
   * @brief Settings for read_program function
   * 
   * @author RevKit
   * @since  1.1
   */
    struct read_program_settings {
        /**
     * @brief Standard constructor
     *
     * Initializes default values
     * 
     * @author RevKit
     * @since  1.1
     */
        read_program_settings();

        /**
     * @brief Default Bit-width
     *
     * Variables can be defined without specifying the bit-width.
     * In this case, this default bit-width is assigned.
     * The default value is \b 32u.
     */
        unsigned default_bitwidth;
    };

    /**
   * @brief Parser for a SyReC program
   *
   * This function call performs both the lexical parsing
   * as well as the semantic analysis of the program which
   * creates the corresponding C++ constructs for the
   * program. 
   *
   * @param prog Empty SyReC program
   * @param filename File-name to parse from
   * @param settings Settings
   * @param error Error Message, in case the function returns false
   * 
   * @return true if parsing was successful, otherwise false
   *
   * @author RevKit
   * @since  1.1
   */
    bool read_program(applications::program& prog, const std::string& filename, const read_program_settings& settings = read_program_settings(), std::string* error = nullptr);

} // namespace syrec

#endif /* PARSER_HPP */
