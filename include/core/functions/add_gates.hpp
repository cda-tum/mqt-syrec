/**
 * @file add_gates.hpp
 *
 * @brief Adding typical gates to a circuit
 */

#ifndef ADD_GATES_HPP
#define ADD_GATES_HPP

#include "core/circuit.hpp"

namespace syrec {

    /**
   * @brief Helper class for adding lines in an easier way
   *
   * This class should not be used stand alone but just with the add_\em gate methods
   * designed for this purpose. See also \ref sub_add_gates.
   *

   *
   * @sa \ref sub_add_gates
   */
    class target_line_adder {
    public:
        /**
     * @brief Default constructor
     *
     * @param gate Gate, to which target lines should be added
     *
     * @sa \ref sub_add_gates
     *

     */
        explicit target_line_adder(gate* gate);

        /**
     * @brief Add one target line
     *
     * @param  l1 First target line
     *
     * @return A smart pointer to the gate
     *
     * @sa \ref sub_add_gates
     *

     */
        gate& operator()(const gate::line& l1);

        /**
     * @brief Add two target lines
     *
     * @param  l1 First target line
     * @param  l2 Second target line
     *
     * @return A smart pointer to the gate
     *
     * @sa \ref sub_add_gates
     *

     */
        gate& operator()(const gate::line& l1, const gate::line& l2);

    private:
        gate* g = nullptr;
    };

    /**
   * @brief Helper class for adding lines in an easier way
   *
   * This class should not be used stand alone but just with the add_\em gate methods
   * designed for this purpose. See also \ref sub_add_gates.
   *

   *
   * @sa \ref sub_add_gates
   */
    class control_line_adder {
    public:
        /**
     * @brief Default constructor
     *
     * @param g Gate, to which control lines should be added
     *
     * @sa \ref sub_add_gates
     *

     */
        explicit control_line_adder(gate& g);

        /**
     * @brief Add no control line
     *
     * @return A target_line_adder
     *
     * @sa \ref sub_add_gates
     *

     */
        target_line_adder operator()();

        /**
     * @brief Add two control lines
     *
     * @param  l1 First control line
     * @param  l2 Second control line
     *
     * @return A target_line_adder
     *
     * @sa \ref sub_add_gates
     *

     */
        target_line_adder operator()(const gate::line& l1, const gate::line& l2);

    private:
        gate* g = nullptr;
    };

    /**
   * @brief Helper function for appending a \b Toffoli gate
   *
   * @param circ     Circuit
   * @param controls Control Lines
   * @param target   Target Line
   *
   * @return Gate reference
   *

   */
    gate& append_toffoli(circuit& circ, const gate::line_container& controls, const gate::line& target);

    /**
   * @brief Helper function for appending a \b CNOT gate
   *
   * @param circ    Circuit
   * @param control Control Line
   * @param target  Target Line
   *
   * @return Gate reference
   *

   */
    gate& append_cnot(circuit& circ, const gate::line& control, const gate::line& target);

    /**
   * @brief Helper function for appending a \b NOT gate
   *
   * @param circ    Circuit
   * @param target  Target Line
   *
   * @return Gate reference
   *

   */
    gate& append_not(circuit& circ, const gate::line& target);

    /**
   * @brief Helper function for appending a generic gate using the control_line_adder
   *
   * @param circ Circuit
   * @param tag  Gate type tag
   *
   * @return A control_line_adder
   *
   * @sa \ref sub_add_gates
   * @sa \ref sub_target_tags
   *

   */
    control_line_adder append_gate(circuit& circ, const std::any& tag);

    /**
   * @brief Helper function for appending a \b Toffoli gate using the control_line_adder
   *
   * @param circ Circuit
   *
   * @return A control_line_adder
   *
   * @sa \ref sub_add_gates
   *

   */
    control_line_adder append_toffoli(circuit& circ);

    /**
   * @brief Helper function for appending a \b Fredkin gate using the control_line_adder
   *
   * @param circ Circuit
   *
   * @return A control_line_adder
   *
   * @sa \ref sub_add_gates
   *

   */
    control_line_adder append_fredkin(circuit& circ);

} // namespace syrec

#endif /* ADD_GATES_HPP */
