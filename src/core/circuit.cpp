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

#include "core/circuit.hpp"

//#include "core/functions/copy_circuit.hpp"
#include "core/gate.hpp"

#include <boost/range/adaptors.hpp>
#include <iostream>
#include <memory>

namespace revkit {
    using boost::adaptors::indirected;
    using boost::adaptors::transformed;

    struct num_gates_visitor {
        unsigned operator()(const standard_circuit& circ) const {
            return circ.gates.size();
        }

        unsigned operator()(const subcircuit& circ) const {
            return circ.to - circ.from;
        }
    };

    struct lines_setter {
        explicit lines_setter(unsigned _lines):
            lines(_lines) {}

        void operator()(standard_circuit& circ) const {
            circ.lines = lines;
            circ.inputs.resize(lines, "i");
            circ.outputs.resize(lines, "o");
            circ.constants.resize(lines, constant());
            circ.garbage.resize(lines, false);
        }

        void operator()(subcircuit& circ [[maybe_unused]]) const {
            // NOTE expand the sub-circuit and therewith automatically the base circuit (in future version)
            assert(false);
        }

    private:
        unsigned lines;
    };

    struct lines_visitor {
        unsigned operator()(const standard_circuit& circ) const {
            return circ.lines;
        }

        unsigned operator()(const subcircuit& circ) const {
            if (!circ.filter.empty()) {
                return circ.filter.size();
            } else {
                return circ.base.lines;
            }
        }
    };

    struct const_begin_visitor: public boost::static_visitor<circuit::const_iterator> {
        circuit::const_iterator operator()(const standard_circuit& circ) const {
            return boost::make_transform_iterator(boost::make_indirect_iterator(circ.gates.begin()), const_filter_circuit());
            //return boost::begin( circ.gates | indirected | transformed( const_filter_circuit() ) );
        }

        circuit::const_iterator operator()(const subcircuit& circ) const {
            if (!circ.filter.empty()) {
                return boost::make_transform_iterator(boost::make_indirect_iterator(circ.base.gates.begin() + circ.from), const_filter_circuit(circ));
            } else {
                return boost::make_transform_iterator(boost::make_indirect_iterator(circ.base.gates.begin() + circ.from), const_filter_circuit());
            }
        }
    };

    struct const_end_visitor: public boost::static_visitor<circuit::const_iterator> {
        circuit::const_iterator operator()(const standard_circuit& circ) const {
            return boost::make_transform_iterator(boost::make_indirect_iterator(circ.gates.end()), const_filter_circuit());
        }

        circuit::const_iterator operator()(const subcircuit& circ) const {
            if (!circ.filter.empty()) {
                return boost::make_transform_iterator(boost::make_indirect_iterator(circ.base.gates.begin() + circ.to), const_filter_circuit(circ));
            } else {
                return boost::make_transform_iterator(boost::make_indirect_iterator(circ.base.gates.begin() + circ.to), const_filter_circuit());
            }
        }
    };

    struct begin_visitor: public boost::static_visitor<circuit::iterator> {
        circuit::iterator operator()(standard_circuit& circ) const {
            return boost::make_transform_iterator(boost::make_indirect_iterator(circ.gates.begin()), filter_circuit());
        }

        circuit::iterator operator()(subcircuit& circ) const {
            if (!circ.filter.empty()) {
                return boost::make_transform_iterator(boost::make_indirect_iterator(circ.base.gates.begin() + circ.from), filter_circuit(circ));
            } else {
                return boost::make_transform_iterator(boost::make_indirect_iterator(circ.base.gates.begin() + circ.from), filter_circuit());
            }
        }
    };

    struct end_visitor: public boost::static_visitor<circuit::iterator> {
        circuit::iterator operator()(standard_circuit& circ) const {
            return boost::make_transform_iterator(boost::make_indirect_iterator(circ.gates.end()), filter_circuit());
        }

        circuit::iterator operator()(subcircuit& circ) const {
            if (!circ.filter.empty()) {
                return boost::make_transform_iterator(boost::make_indirect_iterator(circ.base.gates.begin() + circ.to), filter_circuit(circ));
            } else {
                return boost::make_transform_iterator(boost::make_indirect_iterator(circ.base.gates.begin() + circ.to), filter_circuit());
            }
        }
    };

    struct const_rbegin_visitor: public boost::static_visitor<circuit::const_reverse_iterator> {
        circuit::const_reverse_iterator operator()(const standard_circuit& circ) const {
            return boost::make_transform_iterator(boost::make_indirect_iterator(circ.gates.rbegin()), const_filter_circuit());
        }

        circuit::const_reverse_iterator operator()(const subcircuit& circ) const {
            if (!circ.filter.empty()) {
                return boost::make_transform_iterator(boost::make_indirect_iterator(circ.base.gates.rbegin() + (circ.base.gates.size() - circ.to)), const_filter_circuit(circ));
            } else {
                return boost::make_transform_iterator(boost::make_indirect_iterator(circ.base.gates.rbegin() + (circ.base.gates.size() - circ.to)), const_filter_circuit());
            }
        }
    };

    struct const_rend_visitor: public boost::static_visitor<circuit::const_reverse_iterator> {
        circuit::const_reverse_iterator operator()(const standard_circuit& circ) const {
            return boost::make_transform_iterator(boost::make_indirect_iterator(circ.gates.rend()), const_filter_circuit());
        }

        circuit::const_reverse_iterator operator()(const subcircuit& circ) const {
            if (!circ.filter.empty()) {
                return boost::make_transform_iterator(boost::make_indirect_iterator(circ.base.gates.rbegin() + (circ.base.gates.size() - circ.from)), const_filter_circuit(circ));
            } else {
                return boost::make_transform_iterator(boost::make_indirect_iterator(circ.base.gates.rbegin() + (circ.base.gates.size() - circ.from)), const_filter_circuit());
            }
        }
    };

    struct rbegin_visitor: public boost::static_visitor<circuit::reverse_iterator> {
        circuit::reverse_iterator operator()(standard_circuit& circ) const {
            return boost::make_transform_iterator(boost::make_indirect_iterator(circ.gates.rbegin()), filter_circuit());
        }

        circuit::reverse_iterator operator()(subcircuit& circ) const {
            if (!circ.filter.empty()) {
                return boost::make_transform_iterator(boost::make_indirect_iterator(circ.base.gates.rbegin() + (circ.base.gates.size() - circ.to)), filter_circuit(circ));
            } else {
                return boost::make_transform_iterator(boost::make_indirect_iterator(circ.base.gates.rbegin() + (circ.base.gates.size() - circ.to)), filter_circuit());
            }
        }
    };

    struct rend_visitor: public boost::static_visitor<circuit::reverse_iterator> {
        circuit::reverse_iterator operator()(standard_circuit& circ) const {
            return boost::make_transform_iterator(boost::make_indirect_iterator(circ.gates.rend()), filter_circuit());
        }

        circuit::reverse_iterator operator()(subcircuit& circ) const {
            if (!circ.filter.empty()) {
                return boost::make_transform_iterator(boost::make_indirect_iterator(circ.base.gates.rbegin() + (circ.base.gates.size() - circ.from)), filter_circuit(circ));
            } else {
                return boost::make_transform_iterator(boost::make_indirect_iterator(circ.base.gates.rbegin() + (circ.base.gates.size() - circ.from)), filter_circuit());
            }
        }
    };

    struct append_gate_visitor: public boost::static_visitor<gate&> {
        explicit append_gate_visitor(circuit& c):
            c(c) {}

        gate& operator()(standard_circuit& circ) const {
            gate* g = new gate();
            circ.gates.push_back(std::shared_ptr<gate>(g));
            c.gate_added(*g);
            return *g;
        }

        gate& operator()(subcircuit& circ) const {
            circ.base.gates.insert(circ.base.gates.begin() + circ.to, std::make_shared<gate>());
            ++circ.to;

            if (!circ.filter.empty()) {
                gate& orig_gate = *((circ.base.gates.begin() + circ.to - 1u)->get());
                gate& g         = *(circ.filter_cache[(circ.base.gates.begin() + circ.to - 1)->get()] = new filtered_gate(orig_gate, circ.filter));
                c.gate_added(g);
                return g;
            } else {
                gate& g = **(circ.base.gates.begin() + circ.to - 1);
                c.gate_added(g);
                return g;
            }
        }

    private:
        circuit& c;
    };

    struct prepend_gate_visitor: public boost::static_visitor<gate&> {
        explicit prepend_gate_visitor(circuit& c):
            c(c) {}

        gate& operator()(standard_circuit& circ) const {
            gate* g = new gate();
            circ.gates.insert(circ.gates.begin(), std::shared_ptr<gate>(g));
            c.gate_added(*g);
            return *g;
        }

        gate& operator()(subcircuit& circ) const {
            circ.base.gates.insert(circ.base.gates.begin() + circ.from, std::make_shared<gate>());
            ++circ.to;

            if (!circ.filter.empty()) {
                gate& orig_gate = *((circ.base.gates.begin() + circ.from)->get());
                gate& g         = *(circ.filter_cache[(circ.base.gates.begin() + circ.from)->get()] = new filtered_gate(orig_gate, circ.filter));
                c.gate_added(g);
                return g;
            } else {
                gate& g = **(circ.base.gates.begin() + circ.from);
                c.gate_added(g);
                return g;
            }
        }

    private:
        circuit& c;
    };

    struct insert_gate_visitor: public boost::static_visitor<gate&> {
        insert_gate_visitor(unsigned _pos, circuit& c):
            pos(_pos), c(c) {}

        gate& operator()(standard_circuit& circ) const {
            gate* g = new gate();
            circ.gates.insert(circ.gates.begin() + pos, std::shared_ptr<gate>(g));
            c.gate_added(*g);
            return *g;
        }

        gate& operator()(subcircuit& circ) const {
            circ.base.gates.insert(circ.base.gates.begin() + circ.from + pos, std::make_shared<gate>());
            ++circ.to;

            if (!circ.filter.empty()) {
                gate& orig_gate = *((circ.base.gates.begin() + circ.from + pos)->get());
                gate& g         = *(circ.filter_cache[(circ.base.gates.begin() + circ.from + pos)->get()] = new filtered_gate(orig_gate, circ.filter));
                c.gate_added(g);
                return g;
            } else {
                gate& g = **(circ.base.gates.begin() + circ.from + pos);
                c.gate_added(g);
                return g;
            }
        }

    private:
        unsigned pos;
        circuit& c;
    };

    struct remove_gate_at_visitor: public boost::static_visitor<> {
        explicit remove_gate_at_visitor(unsigned _pos):
            pos(_pos) {}

        void operator()(standard_circuit& circ) const {
            if (pos < circ.gates.size()) {
                circ.gates.erase(circ.gates.begin() + pos);
            }
        }

        void operator()(subcircuit& circ) const {
            if (pos < circ.to) {
                circ.base.gates.erase(circ.base.gates.begin() + circ.from + pos);
                --circ.to;
            }
        }

    private:
        unsigned pos;
    };

    struct inputs_setter: public boost::static_visitor<> {
        explicit inputs_setter(const std::vector<std::string>& _inputs):
            inputs(_inputs) {}

        void operator()(standard_circuit& circ) const {
            circ.inputs.clear();
            std::copy(inputs.begin(), inputs.end(), std::back_inserter(circ.inputs));
            circ.inputs.resize(circ.lines, "i");
        }

        void operator()(subcircuit& circ) const {
            circ.base.inputs.clear();
            std::copy(inputs.begin(), inputs.end(), std::back_inserter(circ.base.inputs));
            circ.base.inputs.resize(circ.base.lines, "i");
        }

    private:
        const std::vector<std::string>& inputs;
    };

    struct inputs_visitor {
        const std::vector<std::string>& operator()(const standard_circuit& circ) const {
            return circ.inputs;
        }

        const std::vector<std::string>& operator()(const subcircuit& circ) const {
            return circ.base.inputs;
        }
    };

    struct outputs_setter {
        explicit outputs_setter(const std::vector<std::string>& _outputs):
            outputs(_outputs) {}

        void operator()(standard_circuit& circ) const {
            circ.outputs.clear();
            std::copy(outputs.begin(), outputs.end(), std::back_inserter(circ.outputs));
            circ.outputs.resize(circ.lines, "o");
        }

        void operator()(subcircuit& circ) const {
            circ.base.outputs.clear();
            std::copy(outputs.begin(), outputs.end(), std::back_inserter(circ.base.outputs));
            circ.base.outputs.resize(circ.base.lines, "o");
        }

    private:
        const std::vector<std::string>& outputs;
    };

    struct outputs_visitor {
        const std::vector<std::string>& operator()(const standard_circuit& circ) const {
            return circ.outputs;
        }

        const std::vector<std::string>& operator()(const subcircuit& circ) const {
            return circ.base.outputs;
        }
    };

    struct constants_setter {
        explicit constants_setter(const std::vector<constant>& _constants):
            constants(_constants) {}

        void operator()(standard_circuit& circ) const {
            circ.constants.clear();
            std::copy(constants.begin(), constants.end(), std::back_inserter(circ.constants));
            circ.constants.resize(circ.lines, constant());
        }

        void operator()(subcircuit& circ) const {
            circ.base.constants.clear();
            std::copy(constants.begin(), constants.end(), std::back_inserter(circ.base.constants));
            circ.base.constants.resize(circ.base.lines, constant());
        }

    private:
        const std::vector<constant>& constants;
    };

    struct constants_visitor {
        const std::vector<constant>& operator()(const standard_circuit& circ) const {
            return circ.constants;
        }

        const std::vector<constant>& operator()(const subcircuit& circ) const {
            return circ.base.constants;
        }
    };

    struct garbage_setter {
        explicit garbage_setter(const std::vector<bool>& _garbage):
            garbage(_garbage) {}

        void operator()(standard_circuit& circ) const {
            circ.garbage.clear();
            std::copy(garbage.begin(), garbage.end(), std::back_inserter(circ.garbage));
            circ.garbage.resize(circ.lines, false);
        }

        void operator()(subcircuit& circ) const {
            circ.base.garbage.clear();
            std::copy(garbage.begin(), garbage.end(), std::back_inserter(circ.base.garbage));
            circ.base.garbage.resize(circ.base.lines, false);
        }

    private:
        const std::vector<bool>& garbage;
    };

    struct garbage_visitor {
        const std::vector<bool>& operator()(const standard_circuit& circ) const {
            return circ.garbage;
        }

        const std::vector<bool>& operator()(const subcircuit& circ) const {
            return circ.base.garbage;
        }
    };

    struct circuit_name_setter {
        explicit circuit_name_setter(const std::string& _name):
            name(_name) {}

        void operator()(standard_circuit& circ) const {
            circ.name = name;
        }

        void operator()(subcircuit& circ) const {
            circ.base.name = name;
        }

    private:
        const std::string& name;
    };

    struct circuit_name_visitor {
        const std::string& operator()(const standard_circuit& circ) const {
            return circ.name;
        }

        const std::string& operator()(const subcircuit& circ) const {
            return circ.base.name;
        }
    };

    struct const_inputbuses_visitor {
        const bus_collection& operator()(const standard_circuit& circ) const {
            return circ.inputbuses;
        }

        const bus_collection& operator()(const subcircuit& circ) const {
            return circ.base.inputbuses;
        }
    };

    struct inputbuses_visitor {
        bus_collection& operator()(standard_circuit& circ) const {
            return circ.inputbuses;
        }

        bus_collection& operator()(subcircuit& circ) const {
            return circ.base.inputbuses;
        }
    };

    struct const_outputbuses_visitor {
        const bus_collection& operator()(const standard_circuit& circ) const {
            return circ.outputbuses;
        }

        const bus_collection& operator()(const subcircuit& circ) const {
            return circ.base.outputbuses;
        }
    };

    struct outputbuses_visitor {
        bus_collection& operator()(standard_circuit& circ) const {
            return circ.outputbuses;
        }

        bus_collection& operator()(subcircuit& circ) const {
            return circ.base.outputbuses;
        }
    };

    struct const_statesignals_visitor {
        const bus_collection& operator()(const standard_circuit& circ) const {
            return circ.statesignals;
        }

        const bus_collection& operator()(const subcircuit& circ) const {
            return circ.base.statesignals;
        }
    };

    struct statesignals_visitor {
        bus_collection& operator()(standard_circuit& circ) const {
            return circ.statesignals;
        }

        bus_collection& operator()(subcircuit& circ) const {
            return circ.base.statesignals;
        }
    };

    struct is_subcircuit_visitor {
        bool operator()(const standard_circuit& circ [[maybe_unused]]) const {
            return false;
        }

        bool operator()(const subcircuit& circ [[maybe_unused]]) const {
            return true;
        }
    };

    struct filter_visitor {
        std::pair<unsigned, std::vector<unsigned>> operator()(const standard_circuit& circ [[maybe_unused]]) const {
            return std::make_pair(0u, std::vector<unsigned>());
        }

        std::pair<unsigned, std::vector<unsigned>> operator()(const subcircuit& circ) const {
            if (!circ.filter.empty()) {
                return std::make_pair(circ.base.lines, circ.filter);
            } else {
                return std::make_pair(0u, std::vector<unsigned>());
            }
        }
    };

    struct offset_visitor {
        unsigned operator()(const standard_circuit& circ [[maybe_unused]]) const {
            return 0u;
        }

        unsigned operator()(const subcircuit& circ) const {
            return circ.from;
        }
    };

    struct annotation_visitor {
        annotation_visitor(const gate& g, const std::string& key, const std::string& default_value):
            g(g), key(key), default_value(default_value) {
        }

        const std::string& operator()(const standard_circuit& circ) const {
            auto it = circ.annotations.find(&g);
            if (it != circ.annotations.end()) {
                auto it2 = it->second.find(key);
                if (it2 != it->second.end()) {
                    return it2->second;
                } else {
                    return default_value;
                }
            } else {
                return default_value;
            }
        }

        const std::string& operator()(const subcircuit& circ) const {
            return operator()(circ.base);
        }

    private:
        const gate&        g;
        const std::string& key;
        const std::string& default_value;
    };

    struct annotations_visitor {
        explicit annotations_visitor(const gate& g):
            g(g) {}

        std::optional<const std::map<std::string, std::string>> operator()(const standard_circuit& circ) const {
            auto it = circ.annotations.find(&g);
            if (it != circ.annotations.end()) {
                return {it->second};
            } else {
                return {};
            }
        }

        std::optional<const std::map<std::string, std::string>> operator()(const subcircuit& circ) const {
            return operator()(circ.base);
        }

    private:
        const gate& g;
    };

    struct annotate_visitor {
        annotate_visitor(const gate& g, const std::string& key, const std::string& value):
            g(g), key(key), value(value) {
        }

        void operator()(standard_circuit& circ) const {
            circ.annotations[&g][key] = value;
        }

        void operator()(subcircuit& circ) const {
            operator()(circ.base);
        }

    private:
        const gate&        g;
        const std::string& key;
        const std::string& value;
    };

    unsigned circuit::num_gates() const {
        return std::visit(num_gates_visitor(), circ);
    }

    void circuit::set_lines(unsigned lines) {
        std::visit(lines_setter(lines), circ);
    }

    unsigned circuit::lines() const {
        return std::visit(lines_visitor(), circ);
    }

    circuit::const_iterator circuit::begin() const {
        return std::visit(const_begin_visitor(), circ);
    }

    circuit::const_iterator circuit::end() const {
        return std::visit(const_end_visitor(), circ);
    }

    circuit::iterator circuit::begin() {
        return std::visit(begin_visitor(), circ);
    }

    circuit::iterator circuit::end() {
        return std::visit(end_visitor(), circ);
    }

    circuit::const_reverse_iterator circuit::rbegin() const {
        return std::visit(const_rbegin_visitor(), circ);
    }

    circuit::const_reverse_iterator circuit::rend() const {
        return std::visit(const_rend_visitor(), circ);
    }

    circuit::reverse_iterator circuit::rbegin() {
        return std::visit(rbegin_visitor(), circ);
    }

    circuit::reverse_iterator circuit::rend() {
        return std::visit(rend_visitor(), circ);
    }

    const gate& circuit::operator[](unsigned index) const {
        return *(begin() + index);
    }

    gate& circuit::operator[](unsigned index) {
        return *(begin() + index);
    }

    gate& circuit::append_gate() {
        return std::visit(append_gate_visitor(*this), circ);
    }

    gate& circuit::prepend_gate() {
        return std::visit(prepend_gate_visitor(*this), circ);
    }

    gate& circuit::insert_gate(unsigned pos) {
        return std::visit(insert_gate_visitor(pos, *this), circ);
    }

    void circuit::remove_gate_at(unsigned pos) {
        std::visit(remove_gate_at_visitor(pos), circ);
    }

    void circuit::set_inputs(const std::vector<std::string>& inputs) {
        std::visit(inputs_setter(inputs), circ);
    }

    const std::vector<std::string>& circuit::inputs() const {
        return std::visit(inputs_visitor(), circ);
    }

    void circuit::set_outputs(const std::vector<std::string>& outputs) {
        std::visit(outputs_setter(outputs), circ);
    }

    const std::vector<std::string>& circuit::outputs() const {
        return std::visit(outputs_visitor(), circ);
    }

    void circuit::set_constants(const std::vector<constant>& constants) {
        std::visit(constants_setter(constants), circ);
    }

    const std::vector<constant>& circuit::constants() const {
        return std::visit(constants_visitor(), circ);
    }

    void circuit::set_garbage(const std::vector<bool>& garbage) {
        std::visit(garbage_setter(garbage), circ);
    }

    const std::vector<bool>& circuit::garbage() const {
        return std::visit(garbage_visitor(), circ);
    }

    void circuit::set_circuit_name(const std::string& name) {
        std::visit(circuit_name_setter(name), circ);
    }

    const std::string& circuit::circuit_name() const {
        return std::visit(circuit_name_visitor(), circ);
    }

    const bus_collection& circuit::inputbuses() const {
        return std::visit(const_inputbuses_visitor(), circ);
    }

    bus_collection& circuit::inputbuses() {
        return std::visit(inputbuses_visitor(), circ);
    }

    const bus_collection& circuit::outputbuses() const {
        return std::visit(const_outputbuses_visitor(), circ);
    }

    bus_collection& circuit::outputbuses() {
        return std::visit(outputbuses_visitor(), circ);
    }

    const bus_collection& circuit::statesignals() const {
        return std::visit(const_statesignals_visitor(), circ);
    }

    bus_collection& circuit::statesignals() {
        return std::visit(statesignals_visitor(), circ);
    }

    bool circuit::is_subcircuit() const {
        return std::visit(is_subcircuit_visitor(), circ);
    }

    std::pair<unsigned, std::vector<unsigned>> circuit::filter() const {
        return std::visit(filter_visitor(), circ);
    }

    unsigned circuit::offset() const {
        return std::visit(offset_visitor(), circ);
    }

    void circuit::add_module(const std::string& name, const std::shared_ptr<circuit>& module) {
        _modules.insert(std::make_pair(name, module));
    }

    void circuit::add_module(const std::string& name, const circuit& module [[maybe_unused]]) {
        auto* copy = new circuit();
        //copy_circuit(module, *copy);
        add_module(name, std::shared_ptr<circuit>(copy));
    }

    const std::map<std::string, std::shared_ptr<circuit>>& circuit::modules() const {
        return _modules;
    }

    const std::string& circuit::annotation(const gate& g, const std::string& key, const std::string& default_value) const {
        return std::visit(annotation_visitor(g, key, default_value), circ);
    }

    std::optional<const std::map<std::string, std::string>> circuit::annotations(const gate& g) const {
        return std::visit(annotations_visitor(g), circ);
    }

    void circuit::annotate(const gate& g, const std::string& key, const std::string& value) {
        std::visit(annotate_visitor(g, key, value), circ);
    }

} // namespace revkit
