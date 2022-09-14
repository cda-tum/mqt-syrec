#include "algorithms/simulation/simple_simulation.hpp"
#include "algorithms/synthesis/syrec_synthesis_additional_lines.hpp"
#include "algorithms/synthesis/syrec_synthesis_no_additional_lines.hpp"
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

    py::class_<Circuit, std::shared_ptr<Circuit>>(m, "circuit")
            .def(py::init<>())
            .def_property("lines", &syrec::Circuit::getLines, &syrec::Circuit::setLines)
            .def_property_readonly("num_gates", &syrec::Circuit::numGates)
            .def("__iter__",
                 [](Circuit& circ) { return py::make_iterator(circ.begin(), circ.end()); })
            .def_property("inputs", &syrec::Circuit::getInputs, &syrec::Circuit::setInputs)
            .def_property("outputs", &syrec::Circuit::getOutputs, &syrec::Circuit::setOutputs)
            .def_property("constants", &syrec::Circuit::getConstants, &syrec::Circuit::setConstants)
            .def_property("garbage", &syrec::Circuit::getGarbage, &syrec::Circuit::setGarbage)
            .def("annotations", [](const Circuit& c, const Gate& g) {
                py::dict   d{};
                const auto annotations = c.getAnnotations(g);
                if (annotations) {
                    for (const auto& [first, second]: *annotations) {
                        d[py::cast(first)] = second;
                    }
                }
                return d;
            })
            .def("quantum_cost", &syrec::Circuit::quantumCost)
            .def("transistor_cost", &syrec::Circuit::transistorCost);

    py::class_<Properties, std::shared_ptr<Properties>>(m, "properties")
            .def(py::init<>())
            .def("set_string", &Properties::set<std::string>)
            .def("set_bool", &Properties::set<bool>)
            .def("set_int", &Properties::set<int>)
            .def("set_unsigned", &Properties::set<unsigned>)
            .def("set_double", &Properties::set<double>)
            .def("get_string", py::overload_cast<const std::string&>(&Properties::get<std::string>, py::const_))
            .def("get_double", py::overload_cast<const std::string&>(&Properties::get<double>, py::const_));

    py::class_<ReadProgramSettings>(m, "read_program_settings")
            .def(py::init<>())
            .def_readwrite("default_bitwidth", &ReadProgramSettings::defaultBitwidth);

    py::class_<program>(m, "program")
            .def(py::init<>())
            .def("add_module", &program::addModule)
            .def("read", &program::read, "filename"_a, "settings"_a = ReadProgramSettings{});

    py::class_<boost::dynamic_bitset<>>(m, "bitset")
            .def(py::init<>())
            .def(py::init<int>())
            .def(py::init<int, uint64_t>())
            .def("set", py::overload_cast<boost::dynamic_bitset<>::size_type, bool>(&boost::dynamic_bitset<>::set))
            .def("__str__", [](const boost::dynamic_bitset<>& b) {
                std::string str{};
                boost::to_string(b, str);
                std::reverse(str.begin(), str.end());
                return str;
            });

    py::enum_<Gate::Types>(m, "gate_type")
            .value("toffoli", Gate::Types::Toffoli)
            .value("fredkin", Gate::Types::Fredkin)
            .export_values();

    py::class_<Gate, std::shared_ptr<Gate>>(m, "gate")
            .def(py::init<>())
            .def_readwrite("controls", &Gate::controls)
            .def_readwrite("targets", &Gate::targets)
            .def_readwrite("type", &Gate::type);

    m.def("syrec_synthesis_additional_lines", &SyrecSynthesisAdditionalLines::synthesize, "circ"_a, "program"_a, "settings"_a = Properties::ptr(), "statistics"_a = Properties::ptr());
    m.def("syrec_synthesis_no_additional_lines", &SyrecSynthesisNoAdditionalLines::synthesize, "circ"_a, "program"_a, "settings"_a = Properties::ptr(), "statistics"_a = Properties::ptr());
    m.def("simple_simulation", &simpleSimulation, "output"_a, "circ"_a, "input"_a, "statistics"_a = Properties::ptr());

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
