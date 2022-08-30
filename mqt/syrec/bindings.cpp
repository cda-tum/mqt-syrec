#include "algorithms/simulation/simple_simulation.hpp"
#include "algorithms/synthesis/syrec_synthesis.hpp"
#include "core/circuit.hpp"
#include "core/gate.hpp"
#include "core/properties.hpp"
#include "core/syrec/program.hpp"

#include <boost/dynamic_bitset.hpp>
#include <functional>
#include <pybind11/stl.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;
using namespace pybind11::literals;
using namespace syrec;

PYBIND11_MODULE(pysyrec, m) {
    m.doc() = "Python interface for the SyReC programming language for the synthesis of reversible circuits";

    py::class_<circuit, std::shared_ptr<circuit>>(m, "circuit")
            .def(py::init<>())
            .def_property("lines", &circuit::get_lines, &circuit::set_lines)
            .def_property_readonly("num_gates", &circuit::num_gates)
            .def("__iter__",
                 [](circuit& circ) { return py::make_iterator(circ.begin(), circ.end()); })
            .def_property("inputs", &circuit::get_inputs, &circuit::set_inputs)
            .def_property("outputs", &circuit::get_outputs, &circuit::set_outputs)
            .def_property("constants", &circuit::get_constants, &circuit::set_constants)
            .def_property("garbage", &circuit::get_garbage, &circuit::set_garbage)
            .def("annotations", [](const circuit& c, const gate& g) {
                py::dict   d{};
                const auto annotations = c.get_annotations(g);
                if (annotations) {
                    for (const auto& [first, second]: *annotations) {
                        d[py::cast(first)] = second;
                    }
                }
                return d;
            })
            .def("quantum_cost", &circuit::quantum_cost)
            .def("transistor_cost", &circuit::transistor_cost);

    py::class_<properties, std::shared_ptr<properties>>(m, "properties")
            .def(py::init<>())
            .def("set_string", &properties::set<std::string>)
            .def("set_bool", &properties::set<bool>)
            .def("set_int", &properties::set<int>)
            .def("set_unsigned", &properties::set<unsigned>)
            .def("set_double", &properties::set<double>)
            .def("get_string", py::overload_cast<const std::string&>(&properties::get<std::string>, py::const_))
            .def("get_double", py::overload_cast<const std::string&>(&properties::get<double>, py::const_));

    py::class_<read_program_settings>(m, "read_program_settings")
            .def(py::init<>())
            .def_readwrite("default_bitwidth", &read_program_settings::default_bitwidth);

    py::class_<program>(m, "program")
            .def(py::init<>())
            .def("add_module", &program::add_module)
            .def("read", &program::read, "filename"_a, "settings"_a = read_program_settings{});

    py::class_<boost::dynamic_bitset<>>(m, "bitset")
            .def(py::init<>())
            .def(py::init<int>())
            .def(py::init<int, unsigned long>())
            .def("set", py::overload_cast<boost::dynamic_bitset<>::size_type, bool>(&boost::dynamic_bitset<>::set))
            .def("__str__", [](const boost::dynamic_bitset<>& b) {
                std::string str{};
                boost::to_string(b, str);
                std::reverse(str.begin(), str.end());
                return str;
            });

    py::enum_<gate::types>(m, "gate_type")
            .value("toffoli", gate::types::Toffoli)
            .value("fredkin", gate::types::Fredkin)
            .export_values();

    py::class_<gate, std::shared_ptr<gate>>(m, "gate")
            .def(py::init<>())
            .def_readwrite("controls", &gate::controls)
            .def_readwrite("targets", &gate::targets)
            .def_readwrite("type", &gate::type);

    m.def("syrec_synthesis_additional_lines", &syrec_synthesis_additional_lines, "circ"_a, "program"_a, "settings"_a = properties::ptr(), "statistics"_a = properties::ptr());
    m.def("syrec_synthesis_no_additional_lines", &syrec_synthesis_no_additional_lines, "circ"_a, "program"_a, "settings"_a = properties::ptr(), "statistics"_a = properties::ptr());
    m.def("simple_simulation", &simple_simulation, "output"_a, "circ"_a, "input"_a, "statistics"_a = properties::ptr());

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
