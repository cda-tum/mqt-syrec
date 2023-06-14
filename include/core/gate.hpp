#pragma once

#include <algorithm>
#include <any>
#include <iostream>
#include <set>
#include <sstream>
#include <vector>

namespace syrec {
    /**
   * @brief Represents a gate in a circuit
   */
    struct Gate {
        /**
        * @brief Represents a gate type
        */
        enum class Types { None,
                           Fredkin,
                           Toffoli };

        /**
        * @brief Type for accessing the line (line index)
        */
        using line = std::size_t;

        /**
        * @brief Container for storing lines
        */
        using line_container = std::set<line>;

        /**
        * @brief Default constructor
        */
        Gate()  = default;
        ~Gate() = default;

        using cost_t = std::uint_least64_t;

        [[nodiscard]] cost_t quantumCost(unsigned lines) const {
            cost_t costs = 0U;

            unsigned    n = lines;
            std::size_t c = controls.size();

            if (type == Gate::Types::Fredkin) {
                c += 1U;
            }

            c += std::max(-1 * static_cast<int>(c), 0);
            c = std::min(static_cast<unsigned>(c), lines - 1U);

            unsigned e = n - static_cast<unsigned>(c) - 1U; // empty lines

            switch (c) {
                case 0U:
                case 1U:
                    costs = 1ULL;
                    break;
                case 2U:
                    costs = 5ULL;
                    break;
                case 3U:
                    costs = 13ULL;
                    break;
                case 4U:
                    costs = (e >= 2U) ? 26ULL : 29ULL;
                    break;
                case 5U:
                    if (e >= 3U) {
                        costs = 38ULL;
                    } else if (e >= 1U) {
                        costs = 52ULL;
                    } else {
                        costs = 61ULL;
                    }
                    break;
                case 6U:
                    if (e >= 4U) {
                        costs = 50ULL;
                    } else if (e >= 1U) {
                        costs = 80ULL;
                    } else {
                        costs = 125ULL;
                    }
                    break;
                case 7U:
                    if (e >= 5U) {
                        costs = 62ULL;
                    } else if (e >= 1U) {
                        costs = 100ULL;
                    } else {
                        costs = 253ULL;
                    }
                    break;
                default:
                    if (e >= c - 2U) {
                        costs = 12ULL * c - 22ULL;
                    } else if (e >= 1U) {
                        costs = 24ULL * c - 87ULL;
                    } else {
                        costs = (1ULL << (c + 1ULL)) - 3ULL;
                    }
            }

            return costs;
        }

        /**
         * @brief Returns the QASM string representation of the gate. Only supports X, CX, CCX, and SWAP gates.
         * @return
         */
        [[nodiscard]] std::string toQasm() const {
            std::stringstream ss;
            if (type == Types::Fredkin) {
                switch (controls.size()) {
                    case 0:
                        ss << "swap q[" << *targets.begin() << "], q[" << *std::next(targets.begin()) << "];";
                        break;
                    case 1:
                        ss << "cswap q[" << *controls.begin() << "], q[" << *targets.begin() << "], q[" << *std::next(targets.begin()) << "];";
                }
            } else if (type == Types::Toffoli && controls.size() <= 3) {
                switch (controls.size()) {
                    case 0:
                        ss << "x q[" << *targets.begin() << "];";
                        break;
                    case 1:
                        ss << "cx q[" << *controls.begin() << "], q[" << *targets.begin() << "];";
                        break;
                    case 2:
                        ss << "ccx q[" << *controls.begin() << "], q[" << *std::next(controls.begin()) << "], q[" << *targets.begin() << "];";
                        break;
                    case 3:
                        ss << "mcx q[" << *controls.begin() << "], q["
                           << *std::next(controls.begin()) << "], q["
                           << *std::next(std::next(controls.begin())) << "], q["
                           << *targets.begin() << "];";
                        break;
                }
            } else {
                // throw gate not supported error
                throw std::runtime_error("Gate not supported");
            }
            return ss.str();
        }

        line_container controls{};
        line_container targets{};
        Types          type = Types::None;
    };

} // namespace syrec
