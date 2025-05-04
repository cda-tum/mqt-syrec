/*
 * Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
 * Copyright (c) 2025 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#pragma once

#include "core/truthTable/truth_table.hpp"
#include "ir/QuantumComputation.hpp"

namespace syrec {

    auto buildTruthTable(const qc::QuantumComputation& qc, TruthTable& tt) -> void;

} // namespace syrec
