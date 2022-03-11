/**
 * @file circuit.hpp
 *
 * @brief Circuit class
 */

#ifndef CIRCUIT_HPP
#define CIRCUIT_HPP

#include "core/gate.hpp"

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/signals2.hpp>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>

namespace syrec {
    /**
   * @brief Type for determine whether line is constant or not
   *
   * The following table summarizes the use of constant values
   * in circuit representations.
   *
   * <table border="0">
   *   <tr>
   *     <td class="indexkey">Description</td>
   *     <td class="indexkey">Char representation</td>
   *     <td class="indexkey">Typed value</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">No constant input line</td>
   *     <td align="center" class="indexvalue">'-'</td>
   *     <td class="indexvalue">@code constant() @endcode</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">Constant input line with value 0</td>
   *     <td align="center" class="indexvalue">'0'</td>
   *     <td class="indexvalue">@code constant( 0 ) @endcode</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">Constant input line with value 1</td>
   *     <td align="center" class="indexvalue">'1'</td>
   *     <td class="indexvalue">@code constant( 1 ) @endcode</td>
   *   </tr>
   * </table>
   *
   * @section Example
   * This example demonstrates how to access the constant values.
   * @code
   * constant c = // some constant
   *
   * if ( c ) // checks whether c is set or not
   * {
   *   if ( *c ) // c is checked, checks the value of c
   *   {
   *     std::cout << "constant value 1" << std::endl;
   *   }
   *   else
   *   {
   *     std::cout << "constant value 0" << std::endl;
   *   }
   * }
   * else
   * {
   *   std::cout << "no constant value" << std::endl;
   * }
   * @endcode
   *

   */
    typedef std::optional<bool> constant;

    class standard_circuit {
    public:
        standard_circuit():
            lines(0) {}

        /** @cond 0 */
        std::vector<std::shared_ptr<gate>> gates;
        unsigned                           lines;

        std::vector<std::string> inputs;
        std::vector<std::string> outputs;
        std::vector<constant>    constants;
        std::vector<bool>        garbage;
        std::string              name;

        std::map<const gate*, std::map<std::string, std::string>> annotations;
        /** @endcond */
    };

    typedef standard_circuit circuit_variant;

    struct const_filter_circuit {
        typedef const gate& result_type;

        const gate& operator()(const gate& g) const {
            return g;
        }

    private:
    };
    /** @endcond */

    /**
   * @brief Main circuit class
   *
   * This class represents a circuit and can be used generically for standard circuits and sub circuits.
   *

   */
    class circuit {
    public:
        /**
     * @brief Default constructor
     *
     * This constructor initializes a standard_circuit with 0 lines, also called an empty circuit.
     * Empty circuits are usually used as parameters for parsing functions, optimization algorithms, etc.
     *

     */
        circuit() = default;

        /**
     * @brief Constant iterator for accessing the gates in a circuit
         *
     */
        typedef boost::transform_iterator<const_filter_circuit, boost::indirect_iterator<std::vector<std::shared_ptr<gate>>::const_iterator>> const_iterator;

        /**
     * @brief Constant reverse iterator for accessing the gates in a circuit
     */
        typedef boost::transform_iterator<const_filter_circuit, boost::indirect_iterator<std::vector<std::shared_ptr<gate>>::const_reverse_iterator>> const_reverse_iterator;

        /**
     * @brief Returns the number of gates
     *
     * This method returns the number of gates in the circuit.
     * 
     * @return Number of gates
     *

     */
        [[nodiscard]] unsigned num_gates() const;

        /**
     * @brief Sets the number of line
     *
     * This method sets the number of lines of the circuit.
     *
     * Changing this number will not affect the data in the gates.
     * For example: If there is a gate with a control on line 3,
     * and the number of lines is reduced to 2 in the circuit, then
     * the control is still on line 3 but not visible in this circuit.
     *
     * So, changing the lines after already adding gates can lead
     * to invalid gates.
     *
     * @param lines Number of lines
     *

     */
        void set_lines(unsigned lines);

        /**
     * @brief Returns the number of lines
     *
     * This method returns the number of lines.
     *
     * @return Number of lines
     *

     */
        [[nodiscard]] unsigned lines() const;

        /**
     * @brief Constant begin iterator pointing to gates
     *
     * @return Constant begin iterator
     *

     */
        [[nodiscard]] const_iterator begin() const;

        /**
     * @brief Constant end iterator pointing to gates
     *
     * @return Constant end iterator
     *

     */
        [[nodiscard]] const_iterator end() const;

        /**
     * @brief Inserts a gate at the end of the circuit
     *
     * This method inserts a gate at the end of the circuit.
     *
     * @return Reference to the newly created empty gate
     *

     */
        gate& append_gate();

        /**
     * @brief Inserts a gate into the circuit
     *
     * This method inserts a gate at an arbitrary position in the circuit
     *
     * @param pos  Position where to insert the gate
     *
     * @return Reference to the newly created empty gate
     *

     */
        gate& insert_gate(unsigned pos);

        /**
     * @brief Sets the input names of the lines in a circuit
     *
     * This method sets the input names of the lines in a circuit.
     * This is useful for functions when writing them to a file,
     * printing them, or creating images.
     *
     * @param inputs Input names
     *

     */
        void set_inputs(const std::vector<std::string>& inputs);

        /**
     * @brief Returns the input names of the lines in a circuit
     *
     * This method returns the input names of the lines in a circuit.
     * This is useful for functions when writing them to a file,
     * printing them, or creating images.
     *
     * @return Input names
     *

     */
        [[nodiscard]] const std::vector<std::string>& inputs() const;

        /**
     * @brief Sets the output names of the lines in a circuit
     *
     * This method sets the output names of the lines in a circuit.
     * This is useful for functions when writing them to a file,
     * printing them, or creating images.
     *
     * @param outputs Output names
     *

     */
        void set_outputs(const std::vector<std::string>& outputs);

        /**
     * @brief Returns the output names of the lines in a circuit
     *
     * This method returns the output names of the lines in a circuit.
     * This is useful for functions when writing them to a file,
     * printing them, or creating images.
     *
     * @return Output names
     *

     */
        [[nodiscard]] const std::vector<std::string>& outputs() const;

        /**
     * @brief Sets the constant input line specifications
     *
     * This method sets the constant input line specification.
     *
     * Lines are by default not constant. If less values are given
     * than lines exist, the last ones will be not constant. If more
     * values are given than lines exist, they will be truncated.
     *
     * @sa constant
     *
     * @param constants Constant Lines
     *

     */
        void set_constants(const std::vector<constant>& constants);

        /**
     * @brief Returns the constant input line specification
     *
     * This method returns the constant input line specification.
     *
     * @return Constant input line specification
     *

     */
        [[nodiscard]] const std::vector<constant>& constants() const;

        /**
     * @brief Sets whether outputs are garbage or not
     *
     * If an output is garbage it means, that the resulting
     * output value is not necessary for the function.
     *
     * Lines are by default not garbage. If less values are given
     * than lines exist, the last ones will be not garbage. If more
     * values are given than lines exist, they will be truncated.
     *
     * @param garbage Garbage line specification
     *

     */
        void set_garbage(const std::vector<bool>& garbage);

        /**
     * @brief Returns whether outputs are garbage or not
     *
     * This method returns the garbage line specification.
     *
     * @return Garbage output line specification
     *

     */
        [[nodiscard]] const std::vector<bool>& garbage() const;

        /**
     * @brief Returns all annotations for a given gate
     *
     * This method returns all annotations for a given gate. For the
     * purpose of efficiency, this method returns an optional data type
     * containing the property map. So, first check whether there are
     * items by assierting the optional, and then go through the map
     * by dereferencing the optional:
     * @code
     *
     * if ( annotations )
     * {
     *   // annotations exists
     *   typedef std::pair<std::string, std::string> pair_t;
     *   foreach_ ( const pair_t& p, *annotations )
     *   {
     *     const std::string& key = p.first;
     *     const std::string& value = p.second;
     *     // do something with key and value
     *   }
     * }
     * @endcode
     * 
     * @param g Gate
     * 
     * @return Map of annotations encapsulated in an optional
     *

     */
        [[nodiscard]] std::optional<const std::map<std::string, std::string>> annotations(const gate& g) const;

        /**
     * @brief Annotates a gate
     * 
     * With this method a gate can be annotated using a key and a value.
     * If there is an annotation with the same key, it will be overwritten.
     *
     * @param g Gate
     * @param key Key of the annotation
     * @param value Value of the annotation
     *

     */
        void annotate(const gate& g, const std::string& key, const std::string& value);

        // SIGNALS
        /**
     * @brief Signal which is emitted after adding a gate
     *
     * The gate is always empty, since when adding a gate to the
     * circuit an empty gate is returned as reference and then
     * further processed by functions such as append_toffoli.
     */
        boost::signals2::signal<void(gate&)> gate_added;

        /** @cond */
        explicit operator circuit_variant&() {
            return circ;
        }

        explicit operator const circuit_variant&() const {
            return circ;
        }
        /** @endcond */
    private:
        /** @cond */
        circuit_variant circ;
        /** @endcond */
    };

    /**
   * @brief Add a line to a circuit with specifying all meta-data
   *
   * This function helps adding a line to the circuit.
   * Besides incrementing the line counter, all meta-data information
   * is adjusted as well.
   *
   * @param circ Circuit
   * @param input Name of the input of the line
   * @param output Name of the output of the line
   * @param c Constant value of that line (Default: Not constant)
   * @param g If true, line is a garbage line
   *
   * @return The index of the newly added line
   *
   (Return value since 1.1)
   */
    unsigned add_line_to_circuit(circuit& circ, const std::string& input, const std::string& output, const constant& c = constant(), bool g = false);

} // namespace syrec

#endif /* CIRCUIT_HPP */
