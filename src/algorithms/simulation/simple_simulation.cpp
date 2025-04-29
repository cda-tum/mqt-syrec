/*
 * Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
 * Copyright (c) 2025 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#include "algorithms/simulation/simple_simulation.hpp"

#include "core/circuit.hpp"
#include "core/gate.hpp"
#include "core/properties.hpp"
#include "core/utils/timer.hpp"

#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <cstddef>
#include <iostream>

namespace syrec {

    void coreGateSimulation(const Gate& g, boost::dynamic_bitset<>& input) {
        if (g.type == Gate::Types::Toffoli) {
            boost::dynamic_bitset<> cMask(input.size());
            for (const auto& c: g.controls) {
                cMask.set(c);
            }

            if (cMask.none() || ((input & cMask) == cMask)) {
                input.flip(*g.targets.begin());
            }
        } else if (g.type == Gate::Types::Fredkin) {
            boost::dynamic_bitset<> cMask(input.size());
            for (const auto& c: g.controls) {
                cMask.set(c);
            }

            if (cMask.none() || ((input & cMask) == cMask)) {
                // get both positions and values
                auto              it = g.targets.begin();
                const std::size_t t1 = *it++;
                const std::size_t t2 = *it;

                const bool t1v = input.test(t1);
                const bool t2v = input.test(t2);

                // only swap when different
                if (t1v != t2v) {
                    input.set(t1, t2v);
                    input.set(t2, t1v);
                }
            }
        } else {
            std::cerr << "Unknown gate: Simulation error\n";
        }
    }

    void simpleSimulation(boost::dynamic_bitset<>& output, const Circuit& circ, const boost::dynamic_bitset<>& input,
                          const Properties::ptr& statistics, const bool reverse) {
        Timer<PropertiesTimer> t;

        if (statistics) {
            const PropertiesTimer rt(statistics);
            t.start(rt);
        }

        output = input;
        if (reverse) {
          for (auto g = circ.crbegin(); g != circ.crend(); ++g) {
            coreGateSimulation(*(*g), output);
          }
        } else {
            for (const auto& g: circ) {
                coreGateSimulation(*g, output);
            }
        }

        if (statistics) {
            t.stop();
        }
    }
} // namespace syrec
