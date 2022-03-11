/**
 * @file costs.hpp
 *
 * @brief Cost calculation for circuits
 */

#ifndef COSTS_HPP
#define COSTS_HPP

#include "core/circuit.hpp"

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
