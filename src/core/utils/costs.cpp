/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2011  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include "core/utils/costs.hpp"
#include "core/target_tags.hpp"

namespace syrec {

    cost_t my_quantum_costs(const gate& g, unsigned lines) {
        cost_t costs;

        unsigned n = lines;
        unsigned c = std::distance(g.begin_controls(), g.end_controls());

        if (is_fredkin(g)) {
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
            case 8u:
                if (e >= 6u) {
                    costs = 74ull;
                } else if (e >= 1u) {
                    costs = 128ull;
                } else {
                    costs = 509ull;
                }
                break;
            case 9u:
                if (e >= 7u) {
                    costs = 86ull;
                } else if (e >= 1u) {
                    costs = 152ull;
                } else {
                    costs = 1021ull;
                }
                break;
            default:
                if (e >= c - 2u) {
                    costs = 12ull * c - 33ull;
                } else if (e >= 1u) {
                    costs = 24ull * c - 87ull;
                } else {
                    costs = (1ull << (c + 1ull)) - 3ull;
                }
        }

        return costs;
    }

    cost_t final_quantum_cost(const circuit& circ, unsigned lines) {
        circuit::const_iterator first = circ.begin();
        circuit::const_iterator last  = circ.end();

        cost_t final_q_costs = 0ull;

        while (first != last) {
            final_q_costs = final_q_costs + my_quantum_costs(*first, lines);
            ++first;
        }
        return final_q_costs;
    }

    cost_t my_transistor_costs(const gate& g, unsigned lines [[maybe_unused]]) {
        return 8ull * std::distance(g.begin_controls(), g.end_controls());
    }

    cost_t final_transistor_cost(const circuit& circ, unsigned lines) {
        circuit::const_iterator first = circ.begin();
        circuit::const_iterator last  = circ.end();

        cost_t final_t_costs = 0ull;

        while (first != last) {
            final_t_costs = final_t_costs + my_transistor_costs(*first, lines);
            ++first;
        }
        return final_t_costs;
    }

} // namespace syrec
