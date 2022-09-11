#include "algorithms/synthesis/syrec_synthesis_additional_lines.hpp"

namespace syrec {

    void SyrecSynthesisAdditionalLines::expAdd(const unsigned& bitwidth, std::vector<unsigned>& lines, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) {
        SyrecSynthesis::getConstantLines(bitwidth, 0U, lines);
        SyrecSynthesis::bitwiseCnot(lines, lhs); // duplicate lhs
        SyrecSynthesis::increase(lines, rhs);
    }

    void SyrecSynthesisAdditionalLines::expSubtract(const unsigned& bitwidth, std::vector<unsigned>& lines, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) {
        SyrecSynthesis::getConstantLines(bitwidth, 0U, lines);
        SyrecSynthesis::bitwiseCnot(lines, lhs); // duplicate lhs
        SyrecSynthesis::decrease(lines, rhs);
    }

    void SyrecSynthesisAdditionalLines::expExor(const unsigned& bitwidth, std::vector<unsigned>& lines, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) {
        SyrecSynthesis::getConstantLines(bitwidth, 0U, lines);
        SyrecSynthesis::bitwiseCnot(lines, lhs); // duplicate lhs
        SyrecSynthesis::bitwiseCnot(lines, rhs);
    }

    bool SyrecSynthesisAdditionalLines::synthesize(Circuit& circ, const program& program, const Properties::ptr& settings, const Properties::ptr& statistics) {
        SyrecSynthesisAdditionalLines synthesizer(circ);
        return SyrecSynthesis::synthesize(&synthesizer, circ, program, settings, statistics);
    }
} // namespace syrec
