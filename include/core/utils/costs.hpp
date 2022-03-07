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

/**
 * @file costs.hpp
 *
 * @brief Cost calculation for circuits
 */

#ifndef COSTS_HPP
#define COSTS_HPP

#include <core/circuit.hpp>

namespace syrec {

    /**
   * @brief Type for costs
   *
   * Costs is an unsigned long long, that is usually
   * at least a 64-bit integer, but depending on the
   * platform.
   *

   */
    typedef unsigned long long cost_t;

    cost_t final_quantum_cost(const circuit& circ, unsigned lines);

    cost_t my_quantum_costs(const gate& g, unsigned lines);

    cost_t my_transistor_costs(const gate& g, unsigned lines);

    cost_t final_transistor_cost(const circuit& circ, unsigned lines);
} // namespace syrec

#endif /* COSTS_HPP */
