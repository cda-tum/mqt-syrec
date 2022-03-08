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
 * @file simple_simulation.hpp
 *
 * @brief Very simple simulation, only efficient for small circuits
 */

#ifndef SIMPLE_SIMULATION_HPP
#define SIMPLE_SIMULATION_HPP

#include "core/circuit.hpp"
#include "core/properties.hpp"

#include <boost/dynamic_bitset.hpp>
#include <functional>
#include <memory>

namespace syrec {

    class gate;

    /**
   * @brief Functor for gate-wise simulation, used as a setting for \ref syrec::simple_simulation "simple_simulation"
   */
    typedef std::function<boost::dynamic_bitset<>&(const gate&, boost::dynamic_bitset<>&)> gate_simulation_func;

    struct core_gate_simulation {
        /**
     * @brief Simulation for a single gate \p g
     *
     * This operator performs simulation for a single gate and is called by
     * \ref syrec::simple_simulation "simple_simulation".
     *
     * \b Important: The return value always has to be \p input, and the
     * operator should modify \p input directly.
     *
     * @param g     The gate to be simulated
     * @param input An input pattern
     *
     * @return Returns a output pattern, it will be the same reference as \p input
     */
        boost::dynamic_bitset<>& operator()(const gate& g, boost::dynamic_bitset<>& input) const;
    };

    /**
   * @brief Simple Simulation function for a range of gates
   *
   * This method calls the \em gate_simulation setting's functor on
   * the gate range from \p first to (exclusive) \p last. Thereby,
   * the last calculated output pattern serves as the input pattern
   * for the next gate. The last calculated output pattern is written
   * to \p output. If a the \em step_result setting is set, it will
   * be called after each gate simulation passing the gate as well
   * as the step result.
   *
   * @param output Output pattern. The index of the pattern corresponds to the line index.
   * @param first Iterator pointing to the first gate.
   * @param last Iterator pointing to the last gate (exclusive).
   * @param input Input pattern. The index of the pattern corresponds to the line index.
   *              The bit-width of the input pattern has to be initialized properly to the
   *              number of lines.
   * @param settings <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Setting</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Default Value</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">gate_simulation</td>
   *     <td class="indexvalue">\ref syrec::gate_simulation_func "gate_simulation_func"</td>
   *     <td class="indexvalue">\ref syrec::core_gate_simulation "core_gate_simulation()"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">The gate-wise simulation functor to use.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">step_result</td>
   *     <td class="indexvalue">\ref syrec::step_result_func "step_result_func"</td>
   *     <td class="indexvalue">\ref syrec::step_result_func "step_result_func()" <i>(empty)</i></td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">A functor called with the last simulated gate and the last calculated output pattern.</td>
   *   </tr>
   * </table>
   * @param statistics <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Information</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Description</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">runtime</td>
   *     <td class="indexvalue">double</td>
   *     <td class="indexvalue">Run-time consumed by the algorithm in CPU seconds.</td>
   *   </tr>
   * </table>
   * @return true on success
   */
    bool simple_simulation(boost::dynamic_bitset<>& output, circuit::const_iterator first, circuit::const_iterator last, const boost::dynamic_bitset<>& input,
                           const properties::ptr& settings   = properties::ptr(),
                           const properties::ptr& statistics = properties::ptr());

    /**
   * @brief Simple Simulation function for a circuit
   *
   * This method calls the \em gate_simulation setting's functor on
   * all gates of the circuit \p circ. Thereby,
   * the last calculated output pattern serves as the input pattern
   * for the next gate. The last calculated output pattern is written
   * to \p output. If a the \em step_result setting is set, it will
   * be called after each gate simulation passing the gate as well
   * as the step result.
   *
   * @param output Output pattern. The index of the pattern corresponds to the line index.
   * @param circ Circuit to be simulated.
   * @param input Input pattern. The index of the pattern corresponds to the line index.
   *              The bit-width of the input pattern has to be initialized properly to the
   *              number of lines.
   * @param settings <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Setting</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Default Value</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">gate_simulation</td>
   *     <td class="indexvalue">\ref syrec::gate_simulation_func "gate_simulation_func"</td>
   *     <td class="indexvalue">\ref syrec::core_gate_simulation "core_gate_simulation()"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">The gate-wise simulation functor to use.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">step_result</td>
   *     <td class="indexvalue">\ref syrec::step_result_func "step_result_func"</td>
   *     <td class="indexvalue">\ref syrec::step_result_func "step_result_func()" <i>(empty)</i></td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">A functor called with the last simulated gate and the last calculated output pattern.</td>
   *   </tr>
   * </table>
   * @param statistics <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Information</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Description</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">runtime</td>
   *     <td class="indexvalue">double</td>
   *     <td class="indexvalue">Run-time consumed by the algorithm in CPU seconds.</td>
   *   </tr>
   * </table>
   * @return true on success
   */
    bool simple_simulation(boost::dynamic_bitset<>& output, const circuit& circ, const boost::dynamic_bitset<>& input,
                           const properties::ptr& settings   = properties::ptr(),
                           const properties::ptr& statistics = properties::ptr());

} // namespace syrec

#endif /* SIMPLE_SIMULATION_HPP */
