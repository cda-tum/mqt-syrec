#include "algorithms/synthesis/syrec_synthesis_additional_lines.hpp"

namespace syrec {

    void SyrecSynthesisAdditionalLines::exp_add(const unsigned& bitwidth, std::vector<unsigned>& lines, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) {
        SyrecSynthesis::get_constant_lines(bitwidth, 0u, lines);
        SyrecSynthesis::bitwise_cnot(lines, lhs); // duplicate lhs
        SyrecSynthesis::increase(lines, rhs);
    }

    void SyrecSynthesisAdditionalLines::exp_subtract(const unsigned& bitwidth, std::vector<unsigned>& lines, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) {
        SyrecSynthesis::get_constant_lines(bitwidth, 0u, lines);
        SyrecSynthesis::bitwise_cnot(lines, lhs); // duplicate lhs
        SyrecSynthesis::decrease(lines, rhs);
    }

    void SyrecSynthesisAdditionalLines::exp_exor(const unsigned& bitwidth, std::vector<unsigned>& lines, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) {
        SyrecSynthesis::get_constant_lines(bitwidth, 0u, lines);
        SyrecSynthesis::bitwise_cnot(lines, lhs); // duplicate lhs
        SyrecSynthesis::bitwise_cnot(lines, rhs);
    }

    bool SyrecSynthesisAdditionalLines::synthesize(circuit& circ, const program& program, const properties::ptr& settings, const properties::ptr& statistics) {
        SyrecSynthesisAdditionalLines synthesizer(circ);
        return SyrecSynthesis::synthesize(&synthesizer, circ, program, settings, statistics);
    }
} // namespace syrec
