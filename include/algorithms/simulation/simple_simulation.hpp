#pragma once

#include "core/circuit.hpp"
#include "core/properties.hpp"

#include <boost/dynamic_bitset.hpp>
#include <functional>
#include <memory>

namespace syrec {

    /**
    * @brief Simulation for a single gate \p g
    *
    * This operator performs simulation for a single gate and is called by
    * \ref syrec::simple_simulation "simple_simulation".
    *
    * \b Important: The operator should modify \p input directly.
    *
    * @param g     The gate to be simulated
    * @param input An input pattern
    */
    void core_gate_simulation(const gate& g, boost::dynamic_bitset<>& input);

    /**
    * @brief Simple Simulation function for a circuit
    *
    * This method calls the \em gate_simulation setting's functor on
    * all gates of the circuit \p circ. Thereby,
    * the last calculated output pattern serves as the input pattern
    * for the next gate. The last calculated output pattern is written
    * to \p output.
    *
    * @param output Output pattern. The index of the pattern corresponds to the line index.
    * @param circ Circuit to be simulated.
    * @param input Input pattern. The index of the pattern corresponds to the line index.
    *              The bit-width of the input pattern has to be initialized properly to the
    *              number of lines.
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
    */
    void simple_simulation(boost::dynamic_bitset<>& output, const circuit& circ, const boost::dynamic_bitset<>& input,
                           const properties::ptr& statistics = properties::ptr());

} // namespace syrec
