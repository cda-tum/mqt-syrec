#include "algorithms/simulation/simple_simulation.hpp"

#include "core/gate.hpp"
#include "core/utils/timer.hpp"

namespace syrec {

    boost::dynamic_bitset<>& core_gate_simulation(const gate& g, boost::dynamic_bitset<>& input) {
        if (gateType::Toffoli == g.type()) {
            boost::dynamic_bitset<> c_mask(input.size());
            for (auto itControl = g.begin_controls(); itControl != g.end_controls(); ++itControl) {
                c_mask.set(*itControl);
            }

            if (c_mask.none() || ((input & c_mask) == c_mask)) {
                input.flip(*g.begin_targets());
            }

            return input;
        } else if (gateType::Fredkin == g.type()) {
            boost::dynamic_bitset<> c_mask(input.size());
            for (auto itControl = g.begin_controls(); itControl != g.end_controls(); ++itControl) {
                c_mask.set(*itControl);
            }

            if (c_mask.none() || ((input & c_mask) == c_mask)) {
                // get both positions and values
                auto     it = g.begin_targets();
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

            return input;
        } else {
            std::cerr << "Unknown gate: Simulation error" << std::endl;
        }
        return input;
    }

    bool simple_simulation(boost::dynamic_bitset<>& output, circuit::const_iterator first, circuit::const_iterator last, const boost::dynamic_bitset<>& input,
                           const properties::ptr& statistics) {
        timer<properties_timer> t;

        if (statistics) {
            properties_timer rt(statistics);
            t.start(rt);
        }

        output = input;
        while (first != last) {
            output = core_gate_simulation(*first, output);
            ++first;
        }

        if (statistics) {
            t.stop();
        }

        return true;
    }

    bool simple_simulation(boost::dynamic_bitset<>& output, const circuit& circ, const boost::dynamic_bitset<>& input,
                           const properties::ptr& statistics) {
        return simple_simulation(output, circ.begin(), circ.end(), input, statistics);
    }

} // namespace syrec