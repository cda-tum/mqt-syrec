#include "algorithms/synthesis/syrec_cost_aware_synthesis.hpp"

#include "algorithms/synthesis/syrec_synthesis.hpp"
#include "core/circuit.hpp"
#include "core/properties.hpp"
#include "core/syrec/program.hpp"

#include <vector>

namespace syrec {

    void CostAwareSynthesis::expAdd(const unsigned& bitwidth, std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs, const std::vector<unsigned>& lines) {
        getConstantLines(bitwidth, 0U, rhs);
        bitwiseCnot(rhs, lhs); // duplicate lhs
        increase(rhs, lines);
    }

    void CostAwareSynthesis::expSubtract(const unsigned& bitwidth, std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs, const std::vector<unsigned>& lines) {
        getConstantLines(bitwidth, 0U, rhs);
        bitwiseCnot(rhs, lhs); // duplicate lhs
        decrease(rhs, lines);
    }

    void CostAwareSynthesis::expExor(const unsigned& bitwidth, std::vector<unsigned>& lines, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) {
        getConstantLines(bitwidth, 0U, lines);
        bitwiseCnot(lines, lhs); // duplicate lhs
        bitwiseCnot(lines, rhs);
    }

    bool CostAwareSynthesis::synthesize(Circuit& circ, const Program& program, const Properties::ptr& settings, const Properties::ptr& statistics) {
        CostAwareSynthesis synthesizer(circ);
        return SyrecSynthesis::synthesize(&synthesizer, circ, program, settings, statistics);
    }
} // namespace syrec
