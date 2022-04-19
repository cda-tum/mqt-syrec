/**
 * @file circuit.hpp
 *
 * @brief Circuit class
 */

#ifndef CIRCUIT_HPP
#define CIRCUIT_HPP

#include "core/gate.hpp"

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
   */
    typedef std::optional<bool> constant;

    /**
   * @brief Main circuit class
   */
    class circuit {
    public:
        /**
         * @brief Default constructor
         *
         * This constructor initializes a standard_circuit with 0 lines, also called an empty circuit.
         * Empty circuits are usually used as parameters for parsing functions, optimization algorithms, etc.
         */
        circuit()  = default;
        ~circuit() = default;

        /**
         * @brief Returns the number of gates
         *
         * This method returns the number of gates in the circuit.
         *
         * @return Number of gates
         */
        [[nodiscard]] unsigned num_gates() const {
            return gates.size();
        }

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
         * @param l Number of lines
         */
        void set_lines(unsigned l) {
            lines = l;
            inputs.resize(lines, "i");
            outputs.resize(lines, "o");
            constants.resize(lines, constant());
            garbage.resize(lines, false);
        }

        /**
         * @brief Returns the number of lines
         *
         * This method returns the number of lines.
         *
         * @return Number of lines
         */
        [[nodiscard]] unsigned get_lines() const {
            return lines;
        }

        /**
         * @brief Constant begin iterator pointing to gates
         *
         * @return Constant begin iterator
         */
        [[nodiscard]] auto cbegin() const {
            return gates.cbegin();
        }

        /**
         * @brief Begin iterator pointing to gates
         *
         * @return Begin iterator
         */
        [[nodiscard]] auto begin() const {
            return gates.begin();
        }

        /**
         * @brief Constant end iterator pointing to gates
         *
         * @return Constant end iterator
         */
        [[nodiscard]] auto cend() const {
            return gates.cend();
        }

        /**
         * @brief End iterator pointing to gates
         *
         * @return End iterator
         */
        [[nodiscard]] auto end() const {
            return gates.end();
        }

        /**
     * @brief Inserts a gate at the end of the circuit
     *
     * This method inserts a gate at the end of the circuit.
     *
     * @return Reference to the newly created empty gate
     */
        gate& append_gate() {
            gates.emplace_back(std::make_shared<gate>());
            gate_added(*gates.back());
            return *gates.back();
        }

        /**
     * @brief Inserts a gate into the circuit
     *
     * This method inserts a gate at an arbitrary position in the circuit
     *
     * @param pos  Position where to insert the gate
     *
     * @return Reference to the newly created empty gate
     */
        gate& insert_gate(unsigned pos) {
            auto ins = gates.insert(gates.begin() + pos, std::make_shared<gate>());
            gate_added(**ins);
            return **ins;
        }

        /**
     * @brief Sets the input names of the lines in a circuit
     *
     * This method sets the input names of the lines in a circuit.
     * This is useful for functions when writing them to a file,
     * printing them, or creating images.
     *
     * @param inputs Input names
     */
        void set_inputs(const std::vector<std::string>& in) {
            inputs = in;
            inputs.resize(lines, "i");
        }

        /**
     * @brief Returns the input names of the lines in a circuit
     *
     * This method returns the input names of the lines in a circuit.
     * This is useful for functions when writing them to a file,
     * printing them, or creating images.
     *
     * @return Input names
     */
        [[nodiscard]] const std::vector<std::string>& get_inputs() const {
            return inputs;
        }

        /**
     * @brief Sets the output names of the lines in a circuit
     *
     * This method sets the output names of the lines in a circuit.
     * This is useful for functions when writing them to a file,
     * printing them, or creating images.
     *
     * @param outputs Output names
     */
        void set_outputs(const std::vector<std::string>& out) {
            outputs = out;
            outputs.resize(lines, "o");
        }

        /**
     * @brief Returns the output names of the lines in a circuit
     *
     * This method returns the output names of the lines in a circuit.
     * This is useful for functions when writing them to a file,
     * printing them, or creating images.
     *
     * @return Output names
     */
        [[nodiscard]] const std::vector<std::string>& get_outputs() const {
            return outputs;
        }

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
     */
        void set_constants(const std::vector<constant>& con) {
            constants = con;
            constants.resize(lines, constant());
        }

        /**
     * @brief Returns the constant input line specification
     *
     * This method returns the constant input line specification.
     *
     * @return Constant input line specification
     */
        [[nodiscard]] const std::vector<constant>& get_constants() const {
            return constants;
        }

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
     */
        void set_garbage(const std::vector<bool>& gar) {
            garbage = gar;
            garbage.resize(lines, false);
        }

        /**
     * @brief Returns whether outputs are garbage or not
     *
     * This method returns the garbage line specification.
     *
     * @return Garbage output line specification
     */
        [[nodiscard]] const std::vector<bool>& get_garbage() const {
            return garbage;
        }

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
     */
        [[nodiscard]] std::optional<const std::map<std::string, std::string>> get_annotations(const gate& g) const {
            auto it = annotations.find(&g);
            if (it != annotations.end()) {
                return {it->second};
            } else {
                return {};
            }
        }

        /**
     * @brief Annotates a gate
     * 
     * With this method a gate can be annotated using a key and a value.
     * If there is an annotation with the same key, it will be overwritten.
     *
     * @param g Gate
     * @param key Key of the annotation
     * @param value Value of the annotation
     */
        void annotate(const gate& g, const std::string& key, const std::string& value) {
            annotations[&g][key] = value;
        }

        /**
       * @brief Add a line to a circuit with specifying all meta-data
       *
       * This function helps adding a line to the circuit.
       * Besides incrementing the line counter, all meta-data information
       * is adjusted as well.
       *
       * @param input Name of the input of the line
       * @param output Name of the output of the line
       * @param c Constant value of that line (Default: Not constant)
       * @param g If true, line is a garbage line
       *
       * @return The index of the newly added line
       */
        unsigned add_line(const std::string& input, const std::string& output, const constant& c = constant(), bool g = false) {
            lines += 1;
            inputs.emplace_back(input);
            outputs.emplace_back(output);
            constants.emplace_back(c);
            garbage.emplace_back(g);

            return lines - 1;
        }

        ///add circuit

        void insert_circuit(unsigned pos, const circuit& src, const gate::line_container& controls) {
            typedef std::pair<std::string, std::string> pair_t;
            if (controls.empty()) {
                for (const auto& g: src) {
                    gate& new_gate = insert_gate(pos++);
                    new_gate       = *g;
                    auto anno      = src.get_annotations(*g);
                    if (anno) {
                        for (const pair_t p: *anno) {
                            annotate(new_gate, p.first, p.second);
                        }
                    }
                }
            } else {
                for (const auto& g: src) {
                    gate& new_gate = insert_gate(pos++);
                    for (const auto& control: controls) {
                        new_gate.controls.emplace(control);
                    }
                    for (const auto& c: g->controls) {
                        new_gate.controls.emplace(c);
                    }
                    for (const auto& t: g->targets) {
                        new_gate.targets.emplace(t);
                    }
                    new_gate.type = g->type;
                    auto anno     = src.get_annotations(*g);
                    if (anno) {
                        for (const pair_t p: *anno) {
                            annotate(new_gate, p.first, p.second);
                        }
                    }
                }
            }
        }

        ///add gates
        gate& append_multi_control_toffoli(const gate::line_container& controls, const gate::line& target) {
            gate& g = append_gate();
            for (const auto& control: controls) {
                g.controls.emplace(control);
            }
            g.targets.emplace(target);
            g.type = gate::types::Toffoli;

            return g;
        }

        gate& append_toffoli(const gate::line& control1, const gate::line& control2, const gate::line& target) {
            gate& g = append_gate();
            g.controls.emplace(control1);
            g.controls.emplace(control2);
            g.targets.emplace(target);
            g.type = gate::types::Toffoli;
            return g;
        }

        gate& append_cnot(const gate::line& control, const gate::line& target) {
            gate& g = append_gate();
            g.controls.emplace(control);
            g.targets.emplace(target);
            g.type = gate::types::Toffoli;
            return g;
        }

        gate& append_not(const gate::line& target) {
            gate& g = append_gate();
            g.targets.emplace(target);
            g.type = gate::types::Toffoli;
            return g;
        }

        gate& append_fredkin(const gate::line& target1, const gate::line& target2) {
            gate& g = append_gate();
            g.targets.emplace(target1);
            g.targets.emplace(target2);
            g.type = gate::types::Fredkin;
            return g;
        }

        // SIGNALS
        /**
     * @brief Signal which is emitted after adding a gate
     *
     * The gate is always empty, since when adding a gate to the
     * circuit an empty gate is returned as reference and then
     * further processed by functions such as append_toffoli.
     */
        boost::signals2::signal<void(gate&)> gate_added;

        [[nodiscard]] gate::cost_t quantum_cost() const {
            gate::cost_t cost = 0U;
            for (const auto& g: gates) {
                cost += g->quantum_cost(lines);
            }
            return cost;
        }

        [[nodiscard]] gate::cost_t transistor_cost() const {
            gate::cost_t cost = 0U;
            for (const auto& g: gates) {
                cost += (8ull * g->controls.size());
            }
            return cost;
        }

    private:
        std::vector<std::shared_ptr<gate>> gates{};
        unsigned                           lines{};

        std::vector<std::string> inputs{};
        std::vector<std::string> outputs{};
        std::vector<constant>    constants{};
        std::vector<bool>        garbage{};
        std::string              name{};

        std::map<const gate*, std::map<std::string, std::string>> annotations;
    };

} // namespace syrec

#endif /* CIRCUIT_HPP */
