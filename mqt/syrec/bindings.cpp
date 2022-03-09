#include "algorithms/simulation/simple_simulation.hpp"
#include "algorithms/synthesis/syrec_synthesis.hpp"
#include "core/circuit.hpp"
#include "core/gate.hpp"
#include "core/properties.hpp"
#include "core/syrec/parser.hpp"
#include "core/syrec/program.hpp"
#include "core/target_tags.hpp"
#include "core/utils/costs.hpp"

//#include <bits/stdc++.h>
#include <boost/dynamic_bitset.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <functional>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace syrec;
using namespace syrec::applications;

py::dict circuit_annotations(const circuit& c, const gate& g) {
    py::dict d{};

    const auto annotations = c.annotations(g);

    if (annotations) {
        for (const auto& [first, second]: *annotations) {
            d[py::cast(first)] = second;
        }
    }

    return d;
}

std::vector<gate> new_gates(const circuit& circ) {
    std::vector<gate> my_gates{};
    auto              first = circ.begin();
    auto              last  = circ.end();
    while (first != last) {
        my_gates.push_back(*first);
        ++first;
    }
    return my_gates;
}

template<typename T, typename C>
py::list list_getter(const C& circ, std::function<const std::vector<T>&(const C*)> fgetter) {
    py::list l;
    for (auto&& elem: fgetter(&circ)) {
        l.append(static_cast<T>(elem));
    }
    return l;
}

py::list inputs_getter(const circuit& circ) {
    return list_getter<std::string, circuit>(circ, &circuit::inputs);
}

py::list outputs_getter(const circuit& circ) {
    return list_getter<std::string, circuit>(circ, &circuit::outputs);
}

py::list constants_getter(const circuit& circ) {
    return list_getter<constant, circuit>(circ, &circuit::constants);
}

py::list garbage_getter(const circuit& circ) {
    return list_getter<bool, circuit>(circ, &circuit::garbage);
}

///Properties

template<typename T>
void properties_set(properties& prop, const std::string& key, const T& value) {
    prop.set(key, value);
}

template<typename T>
T properties_get(properties& prop, const std::string& key) {
    return prop.get<T>(key);
}

///Program

std::string py_read_program(program& prog, const std::string& filename, const read_program_settings& settings) {
    std::string error_message;

    if (!read_program(prog, filename, settings, &error_message)) {
        return error_message;
    } else {
        return {};
    }
}

///dynamic bitset

std::string bitset_to_string(boost::dynamic_bitset<> const& bitset) {
    std::string res;
    for (unsigned i = 0; i < bitset.size(); i++) {
        res += bitset[i] ? "1" : "0";
    }
    return res;
}

/*std::string bitset_to_string(boost::dynamic_bitset<> const& bitset) {
    std::string res;
    boost::to_string(bitset, res);
    std::reverse(res.begin(), res.end());
    return res;
}*/

///gates

namespace gate_types {
    enum _types {
        toffoli,
        fredkin,
    };
}

void gate_set_type(gate& g, unsigned value) {
    switch (value) {
        case gate_types::toffoli:
            g.set_type(toffoli_tag());
            break;
        case gate_types::fredkin:
            g.set_type(fredkin_tag());
            break;
        default:
            break;
    }
}

unsigned gate_get_type(const gate& g) {
    if (is_toffoli(g)) {
        return gate_types::toffoli;
    } else if (is_fredkin(g)) {
        return gate_types::fredkin;
    }
    return 0u;
}

/// control and target lines

py::list control_lines_func(const gate& g) {
    gate::line_container c;
    py::list             l;
    control_lines(g, std::insert_iterator<gate::line_container>(c, c.begin()));
    for (const auto& control: c) {
        l.append(control);
    }
    return l;
}

py::list target_lines_func(const gate& g) {
    gate::line_container c;
    py::list             l;
    target_lines(g, std::insert_iterator<gate::line_container>(c, c.begin()));
    for (const auto& target: c) {
        l.append(target);
    }
    return l;
}

///simulation

std::function<bool(boost::dynamic_bitset<>&, const circuit&, const boost::dynamic_bitset<>&, const properties::ptr&, const properties::ptr&)> sim_func = static_cast<bool (*)(boost::dynamic_bitset<>&, const circuit&, const boost::dynamic_bitset<>&, const properties::ptr&, const properties::ptr&)>(simple_simulation);

PYBIND11_MODULE(pysyrec, m) {
    m.doc() = "Python interface for the SyReC programming language for the synthesis of reversible circuits";
    m.def("py_syrec_synthesis", &syrec_synthesis, py::arg("circ"), py::arg("program"), py::arg("settings"), py::arg("statistics"));
    m.def("py_simple_simulation", sim_func, py::arg("output"), py::arg("circ"), py::arg("input"), py::arg("settings"), py::arg("statistics"));

    py::class_<circuit, std::shared_ptr<circuit>>(m, "circuit")
            .def(py::init<>())
            .def_property("lines", &circuit::lines, &circuit::set_lines)
            .def_property_readonly("num_gates", &circuit::num_gates)
            .def("gates", new_gates)
            .def_property_readonly("inputs", inputs_getter)
            .def_property_readonly("outputs", outputs_getter)
            .def_property_readonly("constants", constants_getter)
            .def_property_readonly("garbage", garbage_getter)
            .def("annotations", circuit_annotations);

    py::class_<properties, std::shared_ptr<properties>>(m, "properties")
            .def(py::init<>())
            .def("set_string", &properties_set<std::string>)
            .def("set_bool", &properties_set<bool>)
            .def("set_int", &properties_set<int>)
            .def("set_unsigned", &properties_set<unsigned>)
            .def("set_double", &properties_set<double>)
            .def("get_string", &properties_get<std::string>)
            .def("get_double", &properties_get<double>);

    py::class_<program>(m, "syrec_program")
            .def(py::init<>())
            .def("add_module", &program::add_module);

    m.def("py_read_program", py_read_program);

    py::class_<read_program_settings>(m, "read_program_settings")
            .def(py::init<>())
            .def_readwrite("default_bitwidth", &read_program_settings::default_bitwidth);

    m.def("quantum_costs", final_quantum_cost);
    m.def("transistor_costs", final_transistor_cost);

    py::class_<boost::dynamic_bitset<>>(m, "bitset")
            .def(py::init<>())
            .def(py::init<int>())
            .def(py::init<int, unsigned long>())
            .def("__str__", bitset_to_string);

    py::enum_<gate_types::_types>(m, "gate_type")
            .value("toffoli", gate_types::toffoli)
            .value("fredkin", gate_types::fredkin)
            .export_values();

    py::class_<gate>(m, "gate")
            .def(py::init<>())
            .def_property("type", gate_get_type, gate_set_type);

    m.def("control_lines", control_lines_func);
    m.def("target_lines", target_lines_func);

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
