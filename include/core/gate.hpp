/**
 * @file gate.hpp
 *
 * @brief Gate class
 */

#ifndef GATE_HPP
#define GATE_HPP

#include <algorithm>
#include <any>
#include <iostream>
#include <set>
#include <vector>

namespace syrec {
    /**
   * @brief Represents a gate in a circuit
   *

   */
    struct gate {
        /**
        * @brief Represents a gate type
        */
        enum class types { None,
                           Fredkin,
                           Toffoli };

        /**
        * @brief Type for accessing the line (line index)
        */
        typedef std::size_t line;

        /**
        * @brief Container for storing lines
        */
        typedef std::set<line> line_container;

        /**
        * @brief Default constructor
        */
        gate()  = default;
        ~gate() = default;

        line_container controls{};
        line_container targets{};
        types          type = types::None;
    };

} // namespace syrec

#endif /* GATE_HPP */
