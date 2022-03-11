/**
 * @file parser.hpp
 *
 * @brief SyReC parser routines
 */
#ifndef PARSER_HPP
#define PARSER_HPP

#include "core/syrec/expression.hpp"
#include "core/syrec/grammar.hpp"
#include "core/syrec/module.hpp"
#include "core/syrec/number.hpp"
#include "core/syrec/program.hpp"
#include "core/syrec/statement.hpp"
#include "core/syrec/variable.hpp"

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

   */
    struct read_program_settings {
        /**
     * @brief Standard constructor
     *
     * Initializes default values
     * 

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

   */
    bool read_program(applications::program& prog, const std::string& filename, const read_program_settings& settings = read_program_settings(), std::string* error = nullptr);

} // namespace syrec

#endif /* PARSER_HPP */
