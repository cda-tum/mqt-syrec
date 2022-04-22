#include "algorithms/simulation/simple_simulation.hpp"

#include "core/gate.hpp"
#include "core/utils/timer.hpp"

namespace syrec {

    void core_gate_simulation(const gate& g, boost::dynamic_bitset<>& input) {
        if (g.type == gate::types::Toffoli) {
            boost::dynamic_bitset<> c_mask(input.size());
            for (const auto& c: g.controls) {
                c_mask.set(c);
            }

            if (c_mask.none() || ((input & c_mask) == c_mask)) {
                input.flip(*g.targets.begin());
            }
        } else if (g.type == gate::types::Fredkin) {
            boost::dynamic_bitset<> c_mask(input.size());
            for (const auto& c: g.controls) {
                c_mask.set(c);
            }

            if (c_mask.none() || ((input & c_mask) == c_mask)) {
                // get both positions and values
                auto     it = g.targets.begin();
                unsigned t1 = *it++;
                unsigned t2 = *it;

                bool t1v = input.test(t1);
                bool t2v = input.test(t2);

                // only swap when different
                if (t1v != t2v) {
                    input.set(t1, t2v);
                    input.set(t2, t1v);
                }
            }
        } else {
            std::cerr << "Unknown gate: Simulation error" << std::endl;
        }
    }

    void simple_simulation(boost::dynamic_bitset<>& output, const circuit& circ, const boost::dynamic_bitset<>& input,
                           const properties::ptr& statistics) {
        timer<properties_timer> t;

        if (statistics) {
            properties_timer rt(statistics);
            t.start(rt);
        }

        output = input;
        for (const auto& g: circ) {
            core_gate_simulation(*g, output);
        }

        if (statistics) {
            t.stop();
        }
    }

} // namespace syrec
