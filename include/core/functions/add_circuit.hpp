/**
 * @file add_circuit.hpp
 *
 * @brief inserting and appending circuits to another circuit
 */

#include "core/circuit.hpp"

namespace syrec {

    /**
   * @brief Insert a circuit \p src at the end of another circuit \p circ
   *
   * @param circ Destination circuit
   * @param src  Source circuit
   * @param controls Controls, which are added to each gate in \p src (introduced in version 1.1)
   *

   */
    void append_circuit(circuit& circ, const circuit& src, const gate::line_container& controls = gate::line_container());

    /**
   * @brief Insert a circuit \p src before gate \p pos (counting from 0) of another circuit \p circ
   *
   * @param circ Destination circuit
   * @param pos  Position where to insert
   * @param src  Source circuit
   * @param controls Controls, which are added to each gate in \p src (introduced in version 1.1)
   *

   */
    void insert_circuit(circuit& circ, unsigned pos, const circuit& src, const gate::line_container& controls = gate::line_container());

} // namespace syrec
