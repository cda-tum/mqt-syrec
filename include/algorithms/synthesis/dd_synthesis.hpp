#pragma once

#include "QuantumComputation.hpp"
#include "core/truthTable/truth_table.hpp"
#include "dd/FunctionalityConstruction.hpp"

#include <chrono>

namespace syrec {

    auto buildDD(const TruthTable& tt, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;

    class DDSynthesizer {
    public:
        DDSynthesizer() = default;

        explicit DDSynthesizer(std::size_t nqubits):
            qc(nqubits) {}

        auto pathFromSrcDst(dd::mNode* src, dd::mNode const* dst, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& tempVec) const -> void;

        auto pathSignature(dd::mNode* src, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& tempVec) -> void;

        static auto terminate(dd::mEdge const& current) -> bool;

        auto unifyPath(dd::mEdge& src, dd::mEdge const& current, TruthTable::Cube::Vector const& p1SigVec, TruthTable::Cube::Vector const& p2SigVec, const std::vector<std::size_t>& indices, std::unique_ptr<dd::Package<>>& dd) -> bool;

        auto swapPaths(dd::mEdge& src, dd::mEdge const& current, std::unique_ptr<dd::Package<>>& dd) -> bool;

        auto shiftUniquePaths(dd::mEdge& src, dd::mEdge const& current, std::unique_ptr<dd::Package<>>& dd) -> bool;

        auto shiftingPaths(dd::mEdge& src, dd::mEdge const& current, std::unique_ptr<dd::Package<>>& dd) -> bool;

        auto synthesize(dd::mEdge& src, std::unique_ptr<dd::Package<>>& dd) -> void;

        auto               begin() noexcept { return qc.begin(); }
        [[nodiscard]] auto begin() const noexcept { return qc.begin(); }
        auto               end() noexcept { return qc.end(); }
        [[nodiscard]] auto end() const noexcept { return qc.end(); }

        auto buildFunctionality(std::unique_ptr<dd::Package<>>& dd) const -> dd::mEdge;

        [[nodiscard]] auto numGate() const -> std::size_t {
            return qc.getNops();
        }

        [[nodiscard]] auto getExecutionTime() const -> double {
            return time.count();
        }

    private:
        qc::QuantumComputation        qc;
        std::chrono::duration<double> time = static_cast<std::chrono::duration<double>>(0);
    };

} // namespace syrec
