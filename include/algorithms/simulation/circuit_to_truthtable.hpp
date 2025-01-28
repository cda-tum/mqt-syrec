#pragma once

#include "core/truthTable/truth_table.hpp"
#include "ir/QuantumComputation.hpp"

namespace syrec {

    auto buildTruthTable(const qc::QuantumComputation& qc, TruthTable& tt) -> void;

} // namespace syrec
