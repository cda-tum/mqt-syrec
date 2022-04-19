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

        typedef std::uint_least64_t cost_t;

        [[nodiscard]] cost_t quantum_cost(unsigned lines) const {
            cost_t costs = 0U;

            unsigned n = lines;
            unsigned c = controls.size();

            if (type == gate::types::Fredkin) {
                c += 1u;
            }

            c += std::max(-1 * (int)c, 0);
            c = std::min(c, lines - 1u);

            unsigned e = n - c - 1u; // empty lines

            switch (c) {
                case 0u:
                case 1u:
                    costs = 1ull;
                    break;
                case 2u:
                    costs = 5ull;
                    break;
                case 3u:
                    costs = 13ull;
                    break;
                case 4u:
                    costs = (e >= 2u) ? 26ull : 29ull;
                    break;
                case 5u:
                    if (e >= 3u) {
                        costs = 38ull;
                    } else if (e >= 1u) {
                        costs = 52ull;
                    } else {
                        costs = 61ull;
                    }
                    break;
                case 6u:
                    if (e >= 4u) {
                        costs = 50ull;
                    } else if (e >= 1u) {
                        costs = 80ull;
                    } else {
                        costs = 125ull;
                    }
                    break;
                case 7u:
                    if (e >= 5u) {
                        costs = 62ull;
                    } else if (e >= 1u) {
                        costs = 100ull;
                    } else {
                        costs = 253ull;
                    }
                    break;
                default:
                    if (e >= c - 2u) {
                        costs = 12ull * c - 22ull;
                    } else if (e >= 1u) {
                        costs = 24ull * c - 87ull;
                    } else {
                        costs = (1ull << (c + 1ull)) - 3ull;
                    }
            }

            return costs;
        }

        line_container controls{};
        line_container targets{};
        types          type = types::None;
    };

} // namespace syrec

#endif /* GATE_HPP */
