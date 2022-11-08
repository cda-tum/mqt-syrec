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
            qc(nqubits) {}

        auto synthesize(dd::mEdge const& src, std::unique_ptr<dd::Package<>>& dd) -> qc::QuantumComputation&;

        [[nodiscard]] auto numGate() const -> std::size_t {
            return numGates;
        }

        [[nodiscard]] auto getExecutionTime() const -> double {
            return runtime;
        }

    private:
        double                 runtime  = 0.;
        std::size_t            numGates = 0U;
        dd::mEdge              srcGlobal{};
        qc::QuantumComputation qc;

        auto pathFromSrcDst(dd::mEdge const& src, dd::mEdge const& dst, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& tempVec) const -> void;

        auto pathSignature(dd::mEdge const& src, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& tempVec) const -> void;

        auto        operation(dd::Qubit const& totalBits, dd::Qubit const& targetBit, dd::mEdge& modifySrc, dd::Controls const& ctrl, std::unique_ptr<dd::Package<>>& dd) -> void;
        static auto controlNonRoot(dd::mEdge const& current, dd::Controls& ctrl, TruthTable::Cube const& ctrlCube) -> void;

        static auto controlRoot(dd::mEdge const& current, dd::Controls& ctrl, TruthTable::Cube const& ctrlCube) -> void;

        auto swapPaths(dd::mEdge const& src, dd::mEdge const& current, TruthTable::Cube::Vector const& p1SigVec, TruthTable::Cube::Vector const& p2SigVec, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;

        auto shiftUniquePaths(dd::mEdge const& src, dd::mEdge const& current, TruthTable::Cube::Vector const& p1SigVec, TruthTable::Cube::Vector const& p2SigVec, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;

        static auto terminate(dd::mEdge const& current) -> bool;

        auto unifyPath(dd::mEdge const& src, dd::mEdge const& current, TruthTable::Cube::Vector const& p1SigVec, TruthTable::Cube::Vector const& p2SigVec, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;

        auto shiftingPaths(dd::mEdge const& src, dd::mEdge const& current, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;

        auto synthesizeRec(dd::mEdge const& src, std::unique_ptr<dd::Package<>>& dd) -> void;
    };

} // namespace syrec
