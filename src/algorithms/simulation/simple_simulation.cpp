#include "algorithms/simulation/simple_simulation.hpp"

#include "core/gate.hpp"
#include "core/utils/timer.hpp"

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
                auto        it = g.targets.begin();
                std::size_t t1 = *it++;
                std::size_t t2 = *it;

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

    void simpleSimulation(boost::dynamic_bitset<>& output, const Circuit& circ, const boost::dynamic_bitset<>& input,
                          const Properties::ptr& statistics) {
        Timer<PropertiesTimer> t;

        if (statistics) {
            PropertiesTimer rt(statistics);
            t.start(rt);
        }

        output = input;
        for (const auto& g: circ) {
            coreGateSimulation(*g, output);
        }

        if (statistics) {
            t.stop();
        }
    }

    auto buildTruthTable(const qc::QuantumComputation& qc, TruthTable const& tt, std::unique_ptr<dd::Package<>>& dd) -> TruthTable {
        TruthTable ttSimOut{};

        for (auto const& [input, _]: tt) {
            auto const inEdge    = dd->makeBasisState(static_cast<dd::Qubit>(tt.nInputs()), input.toBoolVec());
            const auto out       = dd::simulate(&qc, inEdge, dd, 1);
            const auto outString = out.begin()->first;
            ttSimOut.try_emplace(input, TruthTable::Cube::fromString(outString));
        }

        return ttSimOut;
    }

} // namespace syrec
