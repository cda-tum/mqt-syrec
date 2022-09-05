#include "algorithms/synthesis/dd_synthesis.hpp"

using namespace dd::literals;

namespace syrec {

    auto buildDD(const TruthTable& tt, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge {
        // truth tables has to have the same number of inputs and outputs
        assert(tt.nInputs() == tt.nOutputs());

        if (tt.nInputs() == 0U) {
            return dd::mEdge::zero;
        }

        auto edges = std::array<dd::mEdge, 4U>{dd::mEdge::zero, dd::mEdge::zero, dd::mEdge::zero, dd::mEdge::zero};

        // base case
        if (tt.nInputs() == 1U) {
            for (const auto& [input, output]: tt) {
                // truth table has to be completely specified
                assert(input[0].has_value());
                assert(output[0].has_value());
                const auto in    = *input[0];
                const auto out   = *output[0];
                const auto index = static_cast<std::size_t>(out) * 2U + static_cast<std::size_t>(in);
                edges[index]     = dd::mEdge::one;
            }
            return dd->makeDDNode(0, edges);
        }

        // generate sub-tables
        std::array<TruthTable, 4U> subTables{};
        for (const auto& [input, output]: tt) {
            // truth table has to be completely specified
            assert(input[0].has_value());
            assert(output[0].has_value());
            const auto       in    = *input[0];
            const auto       out   = *output[0];
            const auto       index = static_cast<std::size_t>(out) * 2U + static_cast<std::size_t>(in);
            TruthTable::Cube reducedInput(input.begin() + 1, input.end());
            TruthTable::Cube reducedOutput(output.begin() + 1, output.end());
            subTables[index].try_emplace(std::move(reducedInput), std::move(reducedOutput));
        }

        // recursively build the DD for each sub-table
        for (std::size_t i = 0U; i < 4U; i++) {
            edges[i] = buildDD(subTables[i], dd);
            // free up the memory used by the sub-table as fast as possible.
            subTables[i].clear();
        }

        const auto label = static_cast<dd::Qubit>(tt.nInputs() - 1U);
        return dd->makeDDNode(label, edges);
    }
} // namespace syrec
