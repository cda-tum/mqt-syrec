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

    cost_t quantum_cost(const circuit& circ, unsigned lines);

    cost_t single_gate_quantum_cost(const gate& g, unsigned lines);

    cost_t transistor_cost(const circuit& circ);
} // namespace syrec

#endif /* COSTS_HPP */
