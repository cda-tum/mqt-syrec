
#include <algorithms/simulation/simple_simulation.hpp>
#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <core/circuit.hpp>
#include <core/functions/control_lines.hpp>
#include <core/functions/target_lines.hpp>
#include <core/gate.hpp>
#include <core/properties.hpp>
#include <core/syrec/parser.hpp>
#include <core/syrec/program.hpp>
#include <core/target_tags.hpp>
#include <core/utils/costs.hpp>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace syrec;
using namespace syrec::applications;

/// Algorithms / Optimization

[[maybe_unused]] circuit::const_iterator (circuit::*begin1)() const = &circuit::begin;
[[maybe_unused]] circuit::const_iterator (circuit::*end1)() const   = &circuit::end;
[[maybe_unused]] circuit::iterator (circuit::*begin2)()             = &circuit::begin;
[[maybe_unused]] circuit::iterator (circuit::*end2)()               = &circuit::end;

[[maybe_unused]] circuit::const_reverse_iterator (circuit::*rbegin1)() const = &circuit::rbegin;
[[maybe_unused]] circuit::const_reverse_iterator (circuit::*rend1)() const   = &circuit::rend;
[[maybe_unused]] circuit::reverse_iterator (circuit::*rbegin2)()             = &circuit::rbegin;
[[maybe_unused]] circuit::reverse_iterator (circuit::*rend2)()               = &circuit::rend;

py::dict circuit_annotations(const circuit& c, const gate& g) {
    py::dict d;

    std::optional<const std::map<std::string, std::string>> annotations = c.annotations(g);

    if (annotations) {
        typedef std::pair<std::string, std::string> pair_t;
        for (const pair_t p: *annotations) {
            d[py::cast(p.first)] = p.second;
        }
    }

    return d;
}

std::vector<gate> new_gates(const circuit& circ) {
    std::vector<gate>       my_gates;
    circuit::const_iterator first = circ.begin();
    circuit::const_iterator last  = circ.end();
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

int add_new(int i, int j) {
    return i + j;
}

///Properties

template<typename T>
void properties_set(properties& prop, const std::string& key, const T& value) {
    prop.set(key, value);
}

template<typename T>
T properties_get1(properties& prop, const std::string& key) {
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

///gates

namespace gate_types {
    enum _types {
        toffoli,
        peres,
        fredkin,
        v,
        vplus,
        module
    };
}

void gate_set_type(gate& g, unsigned value) {
    switch (value) {
        case gate_types::toffoli:
            g.set_type(toffoli_tag());
            break;
        case gate_types::peres:
            g.set_type(peres_tag());
            break;
        case gate_types::fredkin:
            g.set_type(fredkin_tag());
            break;
        case gate_types::v:
            g.set_type(v_tag());
            break;
        case gate_types::vplus:
            g.set_type(vplus_tag());
            break;
        case gate_types::module:
            g.set_type(module_tag());
            break;
        default:
            assert(false);
            break;
    }
}

unsigned gate_get_type(const gate& g) {
    if (is_toffoli(g)) {
        return gate_types::toffoli;
    } else if (is_peres(g)) {
        return gate_types::peres;
    } else if (is_fredkin(g)) {
        return gate_types::fredkin;
    } else if (is_v(g)) {
        return gate_types::v;
    } else if (is_vplus(g)) {
        return gate_types::vplus;
    } else if (is_module(g)) {
        return gate_types::module;
    }

    assert(false);
    return 0u;
}

std::string gate_module_name(const gate& g) {
    if (is_module(g)) {
        return std::any_cast<module_tag>(g.type()).name;
    } else {
        return {};
    }
}

/// control and target lines

py::list control_lines1(const gate& g) {
    gate::line_container c;
    py::list             l;
    control_lines(g, std::insert_iterator<gate::line_container>(c, c.begin()));
    for (const auto& control: c) {
        l.append(control);
    }
    return l;
}

py::list target_lines1(const gate& g) {
    gate::line_container c;
    py::list             l;
    target_lines(g, std::insert_iterator<gate::line_container>(c, c.begin()));
    for (const auto& target: c) {
        l.append(target);
    }
    return l;
}

PYBIND11_MODULE(pysyrec, m) {
    m.def("py_syrec_synthesis", &syrec_synthesis);
    m.def("py_simple_simulation", static_cast<bool (*)(boost::dynamic_bitset<>&, const circuit&, const boost::dynamic_bitset<>&, const properties::ptr&, const properties::ptr&)>(simple_simulation));

    py::class_<circuit, std::shared_ptr<circuit>>(m, "circuit")
            .def(py::init<>())
            .def(py::init<unsigned>())
            .def_property("lines", &circuit::lines, &circuit::set_lines)
            .def_property_readonly("num_gates", &circuit::num_gates)
            .def("gates", new_gates)
            .def_property_readonly("inputs", inputs_getter)
            .def_property_readonly("outputs", outputs_getter)
            .def_property_readonly("constants", constants_getter)
            .def_property_readonly("garbage", garbage_getter)
            .def("annotations", circuit_annotations);

    m.def("add_new", &add_new, "A function that adds two numbers");

    py::class_<properties, std::shared_ptr<properties>>(m, "properties")
            .def(py::init<>())
            .def("set_string", &properties_set<std::string>)
            .def("set_bool", &properties_set<bool>)
            .def("set_int", &properties_set<int>)
            .def("set_unsigned", &properties_set<unsigned>)
            .def("set_double", &properties_set<double>)
            .def("set_char", &properties_set<char>)
            .def("get_string", &properties_get1<std::string>)
            .def("get_double", &properties_get1<double>);

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
            .value("peres", gate_types::peres)
            .value("fredkin", gate_types::fredkin)
            .value("v", gate_types::v)
            .value("vplus", gate_types::vplus)
            .value("module", gate_types::module)
            .export_values();

    py::class_<gate>(m, "gate")
            .def(py::init<>())
            .def_property("type", gate_get_type, gate_set_type)
            .def_property_readonly("module_name", gate_module_name);


    m.def("control_lines", control_lines1);
    m.def("target_lines", target_lines1);
}
