#include "Dummy.hpp"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11_json/pybind11_json.hpp"
#include "qiskit/QasmQobjExperiment.hpp"
#include "qiskit/QuantumCircuit.hpp"

#include <exception>
#include <memory>

namespace py = pybind11;
namespace nl = nlohmann;
using namespace pybind11::literals;

namespace syrec {
    // this function can be used to import any circuit from a file or via IBM's Qiskit
    qc::QuantumComputation importCircuit(const py::object& circ) {
        py::object QuantumCircuit       = py::module::import("qiskit").attr("QuantumCircuit");
        py::object pyQasmQobjExperiment = py::module::import("qiskit.qobj").attr("QasmQobjExperiment");

        auto qc = qc::QuantumComputation();

        if (py::isinstance<py::str>(circ)) {
            auto&& file = circ.cast<std::string>();
            qc.import(file);
        } else if (py::isinstance(circ, QuantumCircuit)) {
            qc::qiskit::QuantumCircuit::import(qc, circ);
        } else if (py::isinstance(circ, pyQasmQobjExperiment)) {
            qc::qiskit::QasmQobjExperiment::import(qc, circ);
        } else {
            throw std::runtime_error("PyObject is neither py::str, QuantumCircuit, nor QasmQobjExperiment");
        }

        return qc;
    }

    PYBIND11_MODULE(pyqcec, m) {
        m.doc() = "Python interface for the SyReC programming language for the synthesis of reversible circuits";

        py::class_<Dummy>(m, "Dummy")
                .def(py::init<>())
                .def(py::init<double>())
                .def("setVal", &Dummy::setVal)
                .def("getVal", &Dummy::getVal);

#ifdef VERSION_INFO
        m.attr("__version__") = VERSION_INFO;
#else
        m.attr("__version__") = "dev";
#endif
    }
} // namespace syrec
