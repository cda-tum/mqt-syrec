#pragma once

#include "QuantumComputation.hpp"
#include "core/truthTable/truth_table.hpp"
#include "dd/Simulation.hpp"

namespace syrec {

    auto buildTruthTable(const qc::QuantumComputation& qc, TruthTable& tt) -> void;

} // namespace syrec
