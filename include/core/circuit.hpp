/*
 * Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
 * Copyright (c) 2025 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#pragma once

#include "gate.hpp"

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

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
        Circuit() = default;

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
         * Register or update a global gate annotation. Global gate annotations are added to all future gates added to the circuit. Existing gates are not modified.
         * @param key The key of the global gate annotation
         * @param value The value of the global gate annotation
         * @return Whether an existing annotation was updated.
         */
        [[maybe_unused]] bool setOrUpdateGlobalGateAnnotation(const std::string_view& key, const std::string& value) {
            auto existingAnnotationForKey = activeGlobalGateAnnotations.find(key);
            if (existingAnnotationForKey != activeGlobalGateAnnotations.end()) {
                existingAnnotationForKey->second = value;
                return true;
            }
            activeGlobalGateAnnotations.emplace(static_cast<std::string>(key), value);
            return false;
        }

        /**
         * Remove a global gate annotation. Existing annotations of the gates of the circuit are not modified.
         * @param key The key of the global gate annotation to be removed
         * @return Whether a global gate annotation was removed.
         */
        [[maybe_unused]] bool removeGlobalGateAnnotation(const std::string_view& key) {
            // We utilize the ability to use a std::string_view to erase a matching element
            // of std::string in a std::map<std::string, ...> without needing to cast the
            // std::string_view to std::string for the std::map<>::erase() operation
            // (see further: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2077r3.html)
            auto existingAnnotationForKey = activeGlobalGateAnnotations.find(key);
            if (existingAnnotationForKey != activeGlobalGateAnnotations.end()) {
                activeGlobalGateAnnotations.erase(existingAnnotationForKey);
                return true;
            }
            return false;
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

        [[maybe_unused]] Gate::ptr createAndAddToffoliGate(const Gate::Line controlLineOne, const Gate::Line controlLineTwo, const Gate::Line targetLine) {
            // Due to the activation, deactivation and propagation of control lines using the control line propagation scopes, the defined function signature expecting two
            // control lines might be misleading since a toffoli gate with only one control line can be implemented as a CNOT gate. Due to the possibility of 'implementing'
            // the toffoli gate with a CNOT gate we only require a single active control line to be active.
            // Additionally, the validation that none of the two control lines matches the target lines can only be performed after all deregistered control lines from the
            // control line propagation scopes were removed.
            constexpr std::size_t expectedMinimumNumberOfControlLines = 1;
            return createAndAddGate(Gate::Type::Toffoli, Gate::LinesLookup({controlLineOne, controlLineTwo}), Gate::LinesLookup({targetLine}), expectedMinimumNumberOfControlLines);
        }

        [[maybe_unused]] Gate::ptr createAndAddMultiControlToffoliGate(const Gate::LinesLookup& controlLines, const Gate::Line targetLine) {
            // The validation that the two control line does not match the target line can only be performed after all deregistered control lines from the
            // control line propagation scopes were removed.
            constexpr std::size_t expectedMinimumNumberOfControlLines = 1;
            auto                  createdMultiControlToffoliGate      = createAndAddGate(Gate::Type::Toffoli, controlLines, Gate::LinesLookup({targetLine}), expectedMinimumNumberOfControlLines);
            if (createdMultiControlToffoliGate != nullptr && createdMultiControlToffoliGate->controls.empty()) {
                return nullptr;
            }
            return createdMultiControlToffoliGate;
        }

        [[maybe_unused]] Gate::ptr createAndAddCnotGate(const Gate::Line controlLine, Gate::Line targetLine) {
            // The validation that the two control line does not match the target line can only be performed after all deregistered control lines from the
            // control line propagation scopes were removed.
            constexpr std::size_t expectedMinimumNumberOfControlLines = 1;
            return createAndAddGate(Gate::Type::Toffoli, Gate::LinesLookup({controlLine}), Gate::LinesLookup({targetLine}), expectedMinimumNumberOfControlLines);
        }

        [[maybe_unused]] Gate::ptr createAndAddNotGate(Gate::Line targetLine) {
            return createAndAddGate(Gate::Type::Toffoli, std::nullopt, Gate::LinesLookup({targetLine}));
        }

        [[maybe_unused]] Gate::ptr createAndAddFredkinGate(const Gate::Line targetLineOne, const Gate::Line targetLineTwo) {
            if (targetLineOne == targetLineTwo) {
                return nullptr;
            }
            return createAndAddGate(Gate::Type::Fredkin, std::nullopt, Gate::LinesLookup({targetLineOne, targetLineTwo}));
        }

        /**
         * Activate a new control line propagation scope.
         *
         * @remarks All active control lines registered in the currently active propagation scopes
         * will be added to any future gate added to the circuit. Already existing gates are not modified.
         */
        void activateControlLinePropagationScope() {
            controlLinePropagationScopes.emplace_back();
        }

        /**
         * Deactivates the last activated control line propagation scope.
         *
         * @remarks
         * All control lines registered in the last activated control line propagation scope are removed from the aggregate of all active control lines.
         * Control lines active prior to the last activated control line propagation scope and deregistered in said scope are activated again. \n
         * \n
         * Example:
         * Assuming that the aggregate A contains the control lines (1,2,3), a propagation scope is activated and the control lines (3,4)
         * registered which will set the aggregate to (1,2,3,4). After the local scope is deactivated, only the control line 4 that is
         * local to the scope is removed from the aggregate while control line 3 will remain in the aggregate thus the aggregate is equal to (1,2,3) again.
         */
        void deactivateControlLinePropagationScope() {
            if (controlLinePropagationScopes.empty()) {
                return;
            }

            const auto& localControlLineScope = controlLinePropagationScopes.back();
            for (const auto [controlLine, wasControlLineActiveInParentScope]: localControlLineScope) {
                if (wasControlLineActiveInParentScope) {
                    // Control lines registered prior to the local scope and deactivated by the latter should still be registered in the parent
                    // scope after the local one was deactivated.
                    aggregateOfPropagatedControlLines.emplace(controlLine);
                } else {
                    aggregateOfPropagatedControlLines.erase(controlLine);
                }
            }
            controlLinePropagationScopes.pop_back();
        }

        /**
         * Deregister a control line from the last activated control line propagation scope.
         *
         * @remarks The control line is only removed from the aggregate if the last activated local scope registered the @p controlLine.
         * The deregistered control line is not 'inherited' by any gate added while the current scope is active. Additionally, no deregistered control line
         * is filtered from the user provided control lines when adding a new gate.
         * @param controlLine The control line to deregister
         * @return Whether the control line exists in the circuit and whether it was deregistered from the last activated propagation scope.
         */
        [[maybe_unused]] bool deregisterControlLineFromPropagationInCurrentScope(const Gate::Line controlLine) {
            if (controlLinePropagationScopes.empty() || !isLineWithinRange(controlLine)) {
                return false;
            }

            auto& localControlLineScope = controlLinePropagationScopes.back();
            if (localControlLineScope.count(controlLine) == 0) {
                return false;
            }

            aggregateOfPropagatedControlLines.erase(controlLine);
            return true;
        }

        /**
         * Register a control line in the last activated control line propagation scope.
         *
         * @remarks If no active local control line scope exists, a new one is created.
         * @param controlLine The control line to register
         * @return Whether the control line exists in the circuit and whether it was registered in the last activated propagation scope.
         */
        [[maybe_unused]] bool registerControlLineForPropagationInCurrentAndNestedScopes(const Gate::Line controlLine) {
            if (!isLineWithinRange(controlLine)) {
                return false;
            }

            if (controlLinePropagationScopes.empty()) {
                activateControlLinePropagationScope();
            }

            auto& localControlLineScope = controlLinePropagationScopes.back();
            // If an entry for the to be registered control line already exists in the current scope then the previously determine value of the flag indicating whether the control line existed in the parent scope
            // should have the same value that it had when the control line was initially added to the current scope

            if (localControlLineScope.count(controlLine) == 0) {
                localControlLineScope.emplace(std::make_pair(controlLine, aggregateOfPropagatedControlLines.count(controlLine) != 0));
            }
            aggregateOfPropagatedControlLines.emplace(controlLine);
            return true;
        }

        // SIGNALS
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
                return false; // GCOVR_EXCL_LINE
            }
            file << toQasm();
            file.close();
            return true;
        }

    protected:
        /**
         * Create and add a gate of type \p gateType to the circuit.
         *
         * @remarks All active control lines of the currently active control line propagation scopes are added as control lines to the created gate instance
         * @remarks All registered global gate annotations are added to the created gate instance.
         * @remarks None of the provided \p targetLines can be equal to any active control line from any of the active control line propagation scopes (note that child scopes
         * can register/deregister control lines and only the status of the control line after all propagation scopes were processed is considered).
         * @param gateType The type of gate to be added
         * @param controlLines The control lines of the gate to be added. Additionally, the registered control lines of all active local control line scopes will be added as control lines of the gate.
         * @param targetLines The control lines of the gate to be added.
         * @param requiredMinimumNumberOfControlLines Set the minimum number of required control lines of the gate.
         * @return A smart pointer to the created gate instance. If no gate was created a nullptr is returned.
         */
        [[maybe_unused]] Gate::ptr createAndAddGate(Gate::Type gateType, const std::optional<Gate::LinesLookup>& controlLines, const Gate::LinesLookup& targetLines, std::size_t requiredMinimumNumberOfControlLines = 0) {
            if ((controlLines.has_value() && !areLinesWithinRange(*controlLines)) || !areLinesWithinRange(targetLines)) {
                return nullptr;
            }

            auto gateInstance  = std::make_shared<Gate>();
            gateInstance->type = gateType;

            // All control lines deregistered in any parent control line propagation scope are already removed from the aggregate
            gateInstance->controls.insert(aggregateOfPropagatedControlLines.cbegin(), aggregateOfPropagatedControlLines.cend());
            if (controlLines.has_value()) {
                gateInstance->controls.insert(controlLines->cbegin(), controlLines->cend());
            }

            if (gateInstance->controls.size() < requiredMinimumNumberOfControlLines || std::any_of(targetLines.cbegin(), targetLines.cend(), [&gateInstance](const Gate::Line targetLine) { return gateInstance->controls.count(targetLine) != 0; })) {
                return nullptr;
            }

            gateInstance->targets = targetLines;
            gates.emplace_back(gateInstance);
            for (const auto& [annotationKey, annotationValue]: activeGlobalGateAnnotations) {
                annotate(*gateInstance, annotationKey, annotationValue);
            }
            return gateInstance;
        }

        /**
         * Determine whether the given line is known in the circuit assuming that zero-based indexing is used for all circuit lines.
         * @param line The line to check
         * @return Whether the line is known in the circuit
         */
        [[nodiscard]] bool isLineWithinRange(const Gate::Line line) const noexcept {
            return line < lines;
        }

        [[nodiscard]] bool areLinesWithinRange(const Gate::LinesLookup& linesToCheck) const noexcept {
            return std::all_of(linesToCheck.cbegin(), linesToCheck.cend(), [&](const Gate::Line line) { return isLineWithinRange(line); });
        }

    private:
        std::vector<std::shared_ptr<Gate>> gates;
        unsigned                           lines = 0;

        std::vector<std::string> inputs;
        std::vector<std::string> outputs;
        std::vector<constant>    constants;
        std::vector<bool>        garbage;
        std::string              name;

        Gate::LinesLookup                                 aggregateOfPropagatedControlLines;
        std::vector<std::unordered_map<Gate::Line, bool>> controlLinePropagationScopes;

        std::map<const Gate*, std::map<std::string, std::string>> annotations;
        // To be able to use a std::string_view key lookup (heterogeneous lookup) in a std::map/std::unordered_set
        // we need to define the transparent comparator (std::less<>). This feature is only available starting with C++17
        // For further information, see: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3465.pdf
        std::map<std::string, std::string, std::less<>> activeGlobalGateAnnotations;
    };

} // namespace syrec
