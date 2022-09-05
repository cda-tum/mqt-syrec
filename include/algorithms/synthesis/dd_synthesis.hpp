#pragma once

#include "core/truthTable/truth_table.hpp"

namespace syrec {

    auto buildDD(const TruthTable& tt, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;

} // namespace syrec
