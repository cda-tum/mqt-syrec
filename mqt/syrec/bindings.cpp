#include "algorithms/simulation/simple_simulation.hpp"
#include "algorithms/synthesis/syrec_cost_aware_synthesis.hpp"
#include "algorithms/synthesis/syrec_line_aware_synthesis.hpp"
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
            .def(py::init<>(), "Constructs circuit object.")
            .def_property("lines", &syrec::Circuit::getLines, &syrec::Circuit::setLines, "Returns the number of circuit lines.")
            .def_property_readonly("num_gates", &syrec::Circuit::numGates, "Returns the total number of gates in the circuit.")
            .def("__iter__",
                 [](Circuit& circ) { return py::make_iterator(circ.begin(), circ.end()); })
            .def_property("inputs", &syrec::Circuit::getInputs, &syrec::Circuit::setInputs, "Returns the input names of the lines in a circuit.")
            .def_property("outputs", &syrec::Circuit::getOutputs, &syrec::Circuit::setOutputs, "Returns the output names of the lines in a circuit.")
            .def_property("constants", &syrec::Circuit::getConstants, &syrec::Circuit::setConstants, "Returns the constant input line specification.")
            .def_property("garbage", &syrec::Circuit::getGarbage, &syrec::Circuit::setGarbage, "Returns whether outputs are garbage or not.")
            .def(
                    "annotations", [](const Circuit& c, const Gate& g) {
                        py::dict   d{};
                        const auto annotations = c.getAnnotations(g);
                        if (annotations) {
                            for (const auto& [first, second]: *annotations) {
                                d[py::cast(first)] = second;
                            }
                        }
                        return d;
                    },
                    "This method returns all annotations for a given gate.")
            .def("quantum_cost", &syrec::Circuit::quantumCost, "Returns the quantum cost of the circuit.")
            .def("transistor_cost", &syrec::Circuit::transistorCost, "Returns the transistor cost of the circuit.");

    py::class_<Properties, std::shared_ptr<Properties>>(m, "properties")
            .def(py::init<>(), "Constructs property map object.")
            .def("set_string", &Properties::set<std::string>)
            .def("set_bool", &Properties::set<bool>)
            .def("set_int", &Properties::set<int>)
            .def("set_unsigned", &Properties::set<unsigned>)
            .def("set_double", &Properties::set<double>)
            .def("get_string", py::overload_cast<const std::string&>(&Properties::get<std::string>, py::const_))
            .def("get_double", py::overload_cast<const std::string&>(&Properties::get<double>, py::const_));

    py::class_<ReadProgramSettings>(m, "read_program_settings")
            .def(py::init<>(), "Constructs ReadProgramSettings object.")
            .def_readwrite("default_bitwidth", &ReadProgramSettings::defaultBitwidth);

    py::class_<program>(m, "program")
            .def(py::init<>(), "Constructs SyReC program object.")
            .def("add_module", &program::addModule)
            .def("read", &program::read, "filename"_a, "settings"_a = ReadProgramSettings{}, "Read a SyReC program from a file.");

    py::class_<boost::dynamic_bitset<>>(m, "bitset")
            .def(py::init<>(), "Constructs bitset object of size zero.")
            .def(py::init<int>(), "n"_a, "Constructs bitset object of size n.")
            .def(py::init<int, uint64_t>(), "n"_a, "val"_a, "Constructs a bitset object of size n from an integer val")
            .def("set", py::overload_cast<boost::dynamic_bitset<>::size_type, bool>(&boost::dynamic_bitset<>::set), "n"_a, "val"_a, "Sets bit n if val is true, and clears bit n if val is false")
            .def(
                    "__str__", [](const boost::dynamic_bitset<>& b) {
                        std::string str{};
                        boost::to_string(b, str);
                        std::reverse(str.begin(), str.end());
                        return str;
                    },
                    "Returns the equivalent string of the bitset.");

    py::enum_<Gate::Types>(m, "gate_type")
            .value("toffoli", Gate::Types::Toffoli, "Toffoli gate type.")
            .value("fredkin", Gate::Types::Fredkin, "Fredkin gate type.")
            .export_values();

    py::class_<Gate, std::shared_ptr<Gate>>(m, "gate")
            .def(py::init<>(), "Constructs gate object.")
            .def_readwrite("controls", &Gate::controls, "Controls of the gate.")
            .def_readwrite("targets", &Gate::targets, "Targets of the gate.")
            .def_readwrite("type", &Gate::type, "Type of the gate.");

    m.def("cost_aware_synthesis", &CostAwareSynthesis::synthesize, "circ"_a, "program"_a, "settings"_a = Properties::ptr(), "statistics"_a = Properties::ptr(), "Cost-aware synthesis of the SyReC program.");
    m.def("line_aware_synthesis", &LineAwareSynthesis::synthesize, "circ"_a, "program"_a, "settings"_a = Properties::ptr(), "statistics"_a = Properties::ptr(), "Line-aware synthesis of the SyReC program.");
    m.def("simple_simulation", &simpleSimulation, "output"_a, "circ"_a, "input"_a, "statistics"_a = Properties::ptr(), "Simulation of the synthesized circuit circ.");

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
