#pragma once

#include "gate.hpp"

#include <boost/signals2.hpp>
#include <fstream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>

namespace syrec {
    /**
   * @brief Type for determine whether line is constant or not
   */
    using constant = std::optional<bool>;

    /**
   * @brief Main circuit class
   */
    class Circuit {
    public:
        /**
         * @brief Default constructor
         *
         * This constructor initializes a standard_circuit with 0 lines, also called an empty circuit.
         * Empty circuits are usually used as parameters for parsing functions, optimization algorithms, etc.
         */
        Circuit()  = default;
        ~Circuit() = default;

        /**
         * @brief Returns the number of gates
         *
         * This method returns the number of gates in the circuit.
         *
         * @return Number of gates
         */
        [[nodiscard]] auto numGates() const {
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
        void setLines(unsigned l) {
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
        [[nodiscard]] unsigned getLines() const {
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
        Gate& appendGate() {
            gates.emplace_back(std::make_shared<Gate>());
            gateAdded(*gates.back());
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
        Gate& insertGate(unsigned pos) {
            auto ins = gates.insert(gates.begin() + pos, std::make_shared<Gate>());
            gateAdded(**ins);
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
        void setInputs(const std::vector<std::string>& in) {
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
        [[nodiscard]] const std::vector<std::string>& getInputs() const {
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
        void setOutputs(const std::vector<std::string>& out) {
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
        [[nodiscard]] const std::vector<std::string>& getOutputs() const {
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
        void setConstants(const std::vector<constant>& con) {
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
        [[nodiscard]] const std::vector<constant>& getConstants() const {
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
        void setGarbage(const std::vector<bool>& gar) {
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
        [[nodiscard]] const std::vector<bool>& getGarbage() const {
            return garbage;
        }

        /**
     * @brief Returns all annotations for a given gate
     *
     * This method returns all annotations for a given gate. For the
     * purpose of efficiency, this method returns an optional data type
     * containing the property map. So, first check whether there are
     * items by asserting the optional, and then go through the map
     * by dereferencing the optional:
     * @code
     *
     * if ( annotations )
     * {
     *   typedef std::pair<std::string, std::string> pair_t;
     *   foreach_ ( const pair_t& p, *annotations )
     *   {
     *     const std::string& key = p.first;
     *     const std::string& value = p.second;
     *   }
     * }
     * @endcode
     *
     * @param g Gate
     *
     * @return Map of annotations encapsulated in an optional
     */
        [[nodiscard]] std::optional<const std::map<std::string, std::string>> getAnnotations(const Gate& g) const {
            if (auto it = annotations.find(&g); it != annotations.end()) {
                return {it->second};
            }
            return {};
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
        void annotate(const Gate& g, const std::string& key, const std::string& value) {
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
        unsigned addLine(const std::string& input, const std::string& output, const constant& c = constant(), bool g = false) {
            lines += 1;
            inputs.emplace_back(input);
            outputs.emplace_back(output);
            constants.emplace_back(c);
            garbage.emplace_back(g);

            return lines - 1;
        }

        ///add circuit

        void insertCircuit(unsigned pos, const Circuit& src, const Gate::line_container& controls) {
            if (controls.empty()) {
                for (const auto& g: src) {
                    Gate& newGate = insertGate(pos++);
                    newGate       = *g;
                    auto anno     = src.getAnnotations(*g);
                    if (anno) {
                        for (const auto& [first, second]: *anno) {
                            annotate(newGate, first, second);
                        }
                    }
                }
            } else {
                for (const auto& g: src) {
                    Gate& newGate = insertGate(pos++);
                    for (const auto& control: controls) {
                        newGate.controls.emplace(control);
                    }
                    for (const auto& c: g->controls) {
                        newGate.controls.emplace(c);
                    }
                    for (const auto& t: g->targets) {
                        newGate.targets.emplace(t);
                    }
                    newGate.type = g->type;
                    auto anno    = src.getAnnotations(*g);
                    if (anno) {
                        for (const auto& [first, second]: *anno) {
                            annotate(newGate, first, second);
                        }
                    }
                }
            }
        }

        ///add gates
        Gate& appendMultiControlToffoli(const Gate::line_container& controls, const Gate::line& target) {
            Gate& g = appendGate();
            for (const auto& control: controls) {
                g.controls.emplace(control);
            }
            g.targets.emplace(target);
            g.type = Gate::Types::Toffoli;

            return g;
        }

        Gate& appendToffoli(const Gate::line& control1, const Gate::line& control2, const Gate::line& target) {
            Gate& g = appendGate();
            g.controls.emplace(control1);
            g.controls.emplace(control2);
            g.targets.emplace(target);
            g.type = Gate::Types::Toffoli;
            return g;
        }

        Gate& appendCnot(const Gate::line& control, const Gate::line& target) {
            Gate& g = appendGate();
            g.controls.emplace(control);
            g.targets.emplace(target);
            g.type = Gate::Types::Toffoli;
            return g;
        }

        Gate& appendNot(const Gate::line& target) {
            Gate& g = appendGate();
            g.targets.emplace(target);
            g.type = Gate::Types::Toffoli;
            return g;
        }

        Gate& appendFredkin(const Gate::line& target1, const Gate::line& target2) {
            Gate& g = appendGate();
            g.targets.emplace(target1);
            g.targets.emplace(target2);
            g.type = Gate::Types::Fredkin;
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
        boost::signals2::signal<void(Gate&)> gateAdded; //NOLINT(cppcoreguidelines-pro-type-member-init)

        [[nodiscard]] Gate::cost_t quantumCost() const {
            Gate::cost_t cost = 0U;
            for (const auto& g: gates) {
                cost += g->quantumCost(lines);
            }
            return cost;
        }

        [[nodiscard]] Gate::cost_t transistorCost() const {
            Gate::cost_t cost = 0U;
            for (const auto& g: gates) {
                cost += (8ULL * g->controls.size());
            }
            return cost;
        }

        /**
         * @brief Convert circuit to QASM string.
         * @return QASM string
         */
        [[nodiscard]] std::string toQasm() const {
            std::stringstream ss;
            ss << "OPENQASM 2.0;\ninclude \"qelib1.inc\";\nqreg q[" << lines << "];\n";
            for (const auto& g: gates) {
                ss << g->toQasm() << "\n";
            }
            return ss.str();
        }

        /**
         * @brief Write circuit to QASM file.
         * @param filename Filename (should end with .qasm)
         * @return True if successful, false otherwise
         */
        [[nodiscard]] bool toQasmFile(const std::string& filename) const {
            std::ofstream file(filename);
            if (!file.is_open()) {
                return false;
            }
            file << toQasm();
            file.close();
            return true;
        }

    private:
        std::vector<std::shared_ptr<Gate>> gates{};
        unsigned                           lines{};

        std::vector<std::string> inputs{};
        std::vector<std::string> outputs{};
        std::vector<constant>    constants{};
        std::vector<bool>        garbage{};
        std::string              name{};

        std::map<const Gate*, std::map<std::string, std::string>> annotations;
    };

} // namespace syrec
