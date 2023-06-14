#pragma once

#include <algorithm>
#include <any>
#include <cassert>
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

            c += static_cast<std::size_t>(std::max(-1 * static_cast<int>(c), 0));
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
         * @brief Returns the QASM representation of the gate.
         *
         * @note Multiple controls are not supported by QASM without corresponding definitions.
         * Since these definitions grow really large, we do not add them to the resulting QASM files.
         * Instead, multiple controls are merely indicated by prefixing the gate name with as many 'c' as there are controls.
         * The MQT can handle this notation.
         * This special handling will become obsolete once OpenQASM 3.0 is supported within the MQT.
         *
         * @return QASM string
         */
        [[nodiscard]] std::string toQasm() const {
            std::stringstream ss;
            ss << std::string(controls.size(), 'c');
            switch (type) {
                case Types::Fredkin:
                    ss << "swap";
                    break;
                case Types::Toffoli:
                    ss << "x";
                    break;
                // GCOVR_EXCL_START
                default:
                    throw std::runtime_error("Gate not supported");
                    // GCOVR_EXCL_STOP
            }
            for (const auto& control: controls) {
                ss << " q[" << control << "],";
            }
            if (type == Types::Toffoli) {
                assert(targets.size() == 1U);
                ss << " q[" << *targets.begin() << "];";
            } else {
                assert(type == Types::Fredkin);
                assert(targets.size() == 2U);
                ss << " q[" << *targets.begin() << "], q[" << *std::next(targets.begin()) << "];";
            }
            return ss.str();
        }

        line_container controls{};
        line_container targets{};
        Types          type = Types::None;
    };

} // namespace syrec
