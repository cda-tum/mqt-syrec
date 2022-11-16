#pragma once

#include "QuantumComputation.hpp"
#include "algorithms/optimization/esop_minimization.hpp"
#include "core/truthTable/truth_table.hpp"

#include <chrono>

namespace syrec {

    auto buildDD(const TruthTable& tt, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;

    class DDSynthesizer {
    public:
        DDSynthesizer() = default;

        explicit DDSynthesizer(const std::size_t nqubits):
            qc(nqubits) {}

        auto synthesize(dd::mEdge src, std::unique_ptr<dd::Package<>>& dd) -> qc::QuantumComputation&;

        [[nodiscard]] auto numGate() const -> std::size_t {
            return numGates;
        }

        [[nodiscard]] auto getExecutionTime() const -> double {
            return runtime;
        }

    private:
        double                 runtime  = 0.;
        std::size_t            numGates = 0U;
        qc::QuantumComputation qc;

        auto pathFromSrcDst(dd::mEdge const& src, dd::mNode* const& dst, TruthTable::Cube::Vector& sigVec) const -> void;
        auto pathFromSrcDst(dd::mEdge const& src, dd::mNode* const& dst, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& cube) const -> void;

        auto pathSignature(dd::mEdge const& src, TruthTable::Cube::Vector& sigVec) const -> void;
        auto pathSignature(dd::mEdge const& src, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& cube) const -> void;

        auto        applyOperation(dd::QubitCount const& totalBits, dd::Qubit const& targetBit, dd::mEdge& to, dd::Controls const& ctrl, std::unique_ptr<dd::Package<>>& dd) -> void;
        static auto controlNonRoot(dd::mEdge const& current, dd::Controls& ctrl, TruthTable::Cube const& ctrlCube) -> void;
        static auto controlNonRootEsop(dd::mEdge const& current, dd::Controls& ctrl, minbool::MinTerm const& ctrlCube, std::size_t const& n2) -> void;
        static auto controlRootEsop(dd::mEdge const& current, dd::Controls& ctrl, minbool::MinTerm const& ctrlCube, std::size_t const& n2) -> void;

        static auto esopOptimization(TruthTable::Cube::Vector const& sigVec) -> std::vector<minbool::MinTerm>;

        auto swapPaths(dd::mEdge src, dd::mEdge const& current, TruthTable::Cube::Vector const& p1SigVec, TruthTable::Cube::Vector const& p2SigVec, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;

        auto shiftUniquePaths(dd::mEdge src, dd::mEdge const& current, TruthTable::Cube::Vector const& p1SigVec, TruthTable::Cube::Vector const& p2SigVec, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;

        static auto terminate(dd::mEdge const& current) -> bool;

        auto unifyPath(dd::mEdge src, dd::mEdge const& current, TruthTable::Cube::Vector const& p1SigVec, TruthTable::Cube::Vector const& p2SigVec, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;

        auto shiftingPaths(dd::mEdge const& src, dd::mEdge const& current, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;
    };

} // namespace syrec
