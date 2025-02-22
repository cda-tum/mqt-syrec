#pragma once

#include "ir/QuantumComputation.hpp"

#include <cstddef>
#include <iosfwd>
#include <string>

namespace syrec {

    class RealParser {
    public:
        [[nodiscard]] static auto imports(const std::string& realString) -> qc::QuantumComputation;
        [[nodiscard]] static auto importf(const std::string& filename) -> qc::QuantumComputation;
        [[nodiscard]] static auto import(std::istream& is) -> qc::QuantumComputation;

    private:
        RealParser() = default;
        explicit RealParser(qc::QuantumComputation& circ):
            qc(&circ) {}

        qc::QuantumComputation* qc{};
        size_t                  nqubits{0};
        size_t                  nclassics{0};

        [[nodiscard]] auto readRealHeader(std::istream& is) -> int;
        auto               readRealGateDescriptions(std::istream& is, int line) const -> void;
    };
} // namespace syrec
