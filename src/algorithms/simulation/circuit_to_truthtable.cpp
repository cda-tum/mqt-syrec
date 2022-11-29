#include "algorithms/simulation/circuit_to_truthtable.hpp"

namespace syrec {

    auto buildTruthTable(const qc::QuantumComputation& qc) -> TruthTable {
        TruthTable ttSimOut{};
        const auto nBits = qc.getNqubits();
        auto       dd    = std::make_unique<dd::Package<>>(nBits);

        const auto totalInputs = 1U << nBits;

        std::uint64_t n = 0U;

        while (n < totalInputs) {
            const auto inCube = TruthTable::Cube::fromInteger(n, nBits);

            auto const inEdge    = dd->makeBasisState(nBits, inCube.toBoolVec());
            const auto out       = dd::simulate(&qc, inEdge, dd, 1);
            const auto outString = out.begin()->first;

            ttSimOut.try_emplace(inCube, TruthTable::Cube::fromString(outString));
            ++n;
        }

        return ttSimOut;
    }

} // namespace syrec
