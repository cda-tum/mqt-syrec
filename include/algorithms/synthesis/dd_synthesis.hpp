#pragma once

#include "QuantumComputation.hpp"
#include "core/truthTable/truth_table.hpp"

#include <chrono>

namespace syrec {

    auto buildDD(const TruthTable& tt, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;

    class DDSynthesizer {
    public:
        DDSynthesizer() = default;

        explicit DDSynthesizer(std::size_t nqubits):
            qc(nqubits), qcSingleOp(nqubits) {}

        auto synthesize(dd::mEdge const& src, std::unique_ptr<dd::Package<>>& dd) -> qc::QuantumComputation&;

        [[nodiscard]] auto numGate() const -> std::size_t {
            return numGates;
        }

        [[nodiscard]] auto getExecutionTime() const -> double {
            return time;
        }

        qc::QuantumComputation qc;

    private:
        qc::QuantumComputation qcSingleOp;
        double                 time     = 0;
        std::size_t            numGates = 0U;
        dd::mEdge              srcGlobal{};

        auto pathFromSrcDst(dd::mEdge const& src, dd::mEdge const& dst, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& tempVec) const -> void;

        auto pathSignature(dd::mEdge const& src, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& tempVec) const -> void;

        static auto terminate(dd::mEdge const& current) -> bool;

        auto unifyPath(dd::mEdge const& src, dd::mEdge const& current, TruthTable::Cube::Vector const& p1SigVec, TruthTable::Cube::Vector const& p2SigVec, const std::vector<std::size_t>& indices, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;

        auto swapPaths(dd::mEdge const& src, dd::mEdge const& current, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;

        auto shiftUniquePaths(dd::mEdge const& src, dd::mEdge const& current, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;

        auto shiftingPaths(dd::mEdge const& src, dd::mEdge const& current, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;

        auto synthesizeRec(dd::mEdge const& src, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;
    };

} // namespace syrec
