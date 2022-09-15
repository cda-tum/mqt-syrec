#pragma once

#include "core/truthTable/truth_table.hpp"

namespace syrec {

    auto buildDD(const TruthTable& tt, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;

    struct DDSynthesis {
        DDSynthesis() = default;

        explicit DDSynthesis(std::size_t nqubits):
            qc(nqubits) {}

        auto pathFromRoot(dd::mNode* src, dd::mNode* dst, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& tempVec) -> void;

        auto pathSignature(dd::mNode* src, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& tempVec) -> void;

        static auto terminate(dd::mNode* current) -> bool;

        auto unifyPath(dd::mEdge& src, dd::mNode* current, syrec::TruthTable::Cube::Vector& p1SigVec, syrec::TruthTable::Cube::Vector& p2SigVec, const std::vector<std::size_t>& indices, std::unique_ptr<dd::Package<>>& dd) -> void;

        auto swapPaths(dd::mEdge& src, dd::mNode* current, std::unique_ptr<dd::Package<>>& dd) -> void;

        auto shiftUniquePaths(dd::mEdge& src, dd::mNode* current, std::unique_ptr<dd::Package<>>& dd) -> void;

        auto shiftingPaths(dd::mEdge& src, dd::mNode* current, std::unique_ptr<dd::Package<>>& dd) -> void;

        auto synthesize(dd::mEdge& src, std::unique_ptr<dd::Package<>>& dd) -> qc::QuantumComputation&;

        qc::QuantumComputation qc;
    };

} // namespace syrec
