#include "algorithms/simulation/simple_simulation.hpp"

#include "core/gate.hpp"
#include "core/target_tags.hpp"
#include "core/utils/timer.hpp"

namespace syrec {

    boost::dynamic_bitset<>& core_gate_simulation::operator()(const gate& g, boost::dynamic_bitset<>& input) const {
        if (is_toffoli(g)) {
            boost::dynamic_bitset<> c_mask(input.size());
            for (gate::const_iterator itControl = g.begin_controls(); itControl != g.end_controls(); ++itControl) {
                c_mask.set(*itControl);
            }

            if (c_mask.none() || ((input & c_mask) == c_mask)) {
                input.flip(*g.begin_targets());
            }

            return input;
        } else if (is_fredkin(g)) {
            boost::dynamic_bitset<> c_mask(input.size());
            for (gate::const_iterator itControl = g.begin_controls(); itControl != g.end_controls(); ++itControl) {
                c_mask.set(*itControl);
            }

            if (c_mask.none() || ((input & c_mask) == c_mask)) {
                // get both positions and values
                gate::const_iterator it = g.begin_targets();
                unsigned             t1 = *it++;
                unsigned             t2 = *it;

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
            assert(false);
        }
        return input;
    }

    bool simple_simulation(boost::dynamic_bitset<>& output, circuit::const_iterator first, circuit::const_iterator last, const boost::dynamic_bitset<>& input,
                           const properties::ptr& settings,
                           const properties::ptr& statistics) {
        auto gate_simulation = get<gate_simulation_func>(settings, "gate_simulation", core_gate_simulation());

        timer<properties_timer> t;

        if (statistics) {
            properties_timer rt(statistics);
            t.start(rt);
        }

        output = input;
        while (first != last) {
            output = gate_simulation(*first, output);
            ++first;
        }
        return true;
    }

    bool simple_simulation(boost::dynamic_bitset<>& output, const circuit& circ, const boost::dynamic_bitset<>& input,
                           const properties::ptr& settings,
                           const properties::ptr& statistics) {
        return simple_simulation(output, circ.begin(), circ.end(), input, settings, statistics);
    }

} // namespace syrec
