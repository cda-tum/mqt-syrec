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

#include "core/gate.hpp"

#include <memory>
#include <vector>

namespace syrec {

    struct num_gates_visitor {
        unsigned operator()(const standard_circuit& circ) const {
            return circ.gates.size();
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

    private:
        unsigned lines;
    };

    struct lines_visitor {
        unsigned operator()(const standard_circuit& circ) const {
            return circ.lines;
        }
    };

    struct const_begin_visitor {
        circuit::const_iterator operator()(const standard_circuit& circ) const {
            return boost::make_transform_iterator(boost::make_indirect_iterator(circ.gates.begin()), const_filter_circuit());
        }
    };

    struct const_end_visitor {
        circuit::const_iterator operator()(const standard_circuit& circ) const {
            return boost::make_transform_iterator(boost::make_indirect_iterator(circ.gates.end()), const_filter_circuit());
        }
    };

    struct append_gate_visitor {
        explicit append_gate_visitor(circuit& c):
            c(c) {}

        gate& operator()(standard_circuit& circ) const {
            gate* g = new gate();
            circ.gates.push_back(std::shared_ptr<gate>(g));
            c.gate_added(*g);
            return *g;
        }

    private:
        circuit& c;
    };

    struct insert_gate_visitor {
        insert_gate_visitor(unsigned _pos, circuit& c):
            pos(_pos), c(c) {}

        gate& operator()(standard_circuit& circ) const {
            gate* g = new gate();
            circ.gates.insert(circ.gates.begin() + pos, std::shared_ptr<gate>(g));
            c.gate_added(*g);
            return *g;
        }

    private:
        unsigned pos;
        circuit& c;
    };

    struct inputs_setter {
        explicit inputs_setter(const std::vector<std::string>& _inputs):
            inputs(_inputs) {}

        void operator()(standard_circuit& circ) const {
            circ.inputs.clear();
            std::copy(inputs.begin(), inputs.end(), std::back_inserter(circ.inputs));
            circ.inputs.resize(circ.lines, "i");
        }

    private:
        const std::vector<std::string>& inputs;
    };

    struct inputs_visitor {
        const std::vector<std::string>& operator()(const standard_circuit& circ) const {
            return circ.inputs;
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

    private:
        const std::vector<std::string>& outputs;
    };

    struct outputs_visitor {
        const std::vector<std::string>& operator()(const standard_circuit& circ) const {
            return circ.outputs;
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

    private:
        const std::vector<constant>& constants;
    };

    struct constants_visitor {
        const std::vector<constant>& operator()(const standard_circuit& circ) const {
            return circ.constants;
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

    private:
        const std::vector<bool>& garbage;
    };

    struct garbage_visitor {
        const std::vector<bool>& operator()(const standard_circuit& circ) const {
            return circ.garbage;
        }
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

    private:
        const gate&        g;
        const std::string& key;
        const std::string& value;
    };

    unsigned circuit::num_gates() const {
        num_gates_visitor numGatesVisitor;
        return numGatesVisitor(circ);
    }

    void circuit::set_lines(unsigned lines) {
        lines_setter linesSetter(lines);
        return linesSetter(circ);
    }

    unsigned circuit::lines() const {
        lines_visitor linesVisitor;
        return linesVisitor(circ);
    }

    circuit::const_iterator circuit::begin() const {
        const_begin_visitor constBeginVisitor;
        return constBeginVisitor(circ);
    }

    circuit::const_iterator circuit::end() const {
        const_end_visitor constEndVisitor;
        return constEndVisitor(circ);
    }

    gate& circuit::append_gate() {
        append_gate_visitor appendGateVisitor(*this);
        return appendGateVisitor(circ);
    }

    gate& circuit::insert_gate(unsigned pos) {
        insert_gate_visitor insertGateVisitor(pos, *this);
        return insertGateVisitor(circ);
    }

    void circuit::set_inputs(const std::vector<std::string>& inputs) {
        inputs_setter inputsSetter(inputs);
        return inputsSetter(circ);
    }

    const std::vector<std::string>& circuit::inputs() const {
        inputs_visitor inputsVisitor;
        return inputsVisitor(circ);
    }

    void circuit::set_outputs(const std::vector<std::string>& outputs) {
        outputs_setter outputsSetter(outputs);
        return outputsSetter(circ);
    }

    const std::vector<std::string>& circuit::outputs() const {
        outputs_visitor outputsVisitor;
        return outputsVisitor(circ);
    }

    void circuit::set_constants(const std::vector<constant>& constants) {
        constants_setter constSetter(constants);
        return constSetter(circ);
    }

    const std::vector<constant>& circuit::constants() const {
        constants_visitor constVisitor;
        return constVisitor(circ);
    }

    void circuit::set_garbage(const std::vector<bool>& garbage) {
        garbage_setter garbageSetter(garbage);
        return garbageSetter(circ);
    }

    const std::vector<bool>& circuit::garbage() const {
        garbage_visitor garbageVisitor;
        return garbageVisitor(circ);
    }

    std::optional<const std::map<std::string, std::string>> circuit::annotations(const gate& g) const {
        annotations_visitor annotationsVisitor(g);
        return annotationsVisitor(circ);
    }

    void circuit::annotate(const gate& g, const std::string& key, const std::string& value) {
        annotate_visitor annotateVisitor(g, key, value);
        return annotateVisitor(circ);
    }

    unsigned add_line_to_circuit(circuit& circ, const std::string& input, const std::string& output, const constant& c, bool g) {
        std::vector<std::string> ins  = circ.inputs();
        std::vector<std::string> outs = circ.outputs();
        std::vector<constant>    cs   = circ.constants();
        std::vector<bool>        gar  = circ.garbage();

        circ.set_lines(circ.lines() + 1u);

        ins.emplace_back(input);
        circ.set_inputs(ins);

        outs.emplace_back(output);
        circ.set_outputs(outs);

        cs.emplace_back(c);
        circ.set_constants(cs);

        gar.emplace_back(g);
        circ.set_garbage(gar);

        return circ.lines() - 1u;
    }

} // namespace syrec
