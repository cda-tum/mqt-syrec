#pragma once

#include "QuantumComputation.hpp"
#include "algorithms/optimization/esop_minimization.hpp"
#include "algorithms/synthesis/encoding.hpp"
#include "core/truthTable/truth_table.hpp"

#include <chrono>

namespace syrec {

    auto buildDD(const TruthTable& tt, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;

    class DDSynthesizer {
    public:
        static auto synthesizeCodingTechniques(const TruthTable& tt, const bool encodeWithoutAdditionalLine = true) -> std::shared_ptr<qc::QuantumComputation> {
            DDSynthesizer synthesizer{};
            return synthesizer.synthesizeCodingTechniquesTT(tt, encodeWithoutAdditionalLine);
        }

        static auto synthesizeOnePass(const TruthTable& tt) -> std::shared_ptr<qc::QuantumComputation> {
            DDSynthesizer synthesizer{};
            return synthesizer.synthesizeOnePassTT(tt);
        }

        auto synthesize(dd::mEdge src, std::unique_ptr<dd::Package<>>& dd) -> std::shared_ptr<qc::QuantumComputation>;

        [[nodiscard]] auto numGate() const -> std::size_t {
            return numGates;
        }

        auto reset() -> void {
            runtime     = 0.;
            numGates    = 0U;
            n           = 0U;
            m           = 0U;
            totalNoBits = 0U;
            r           = 0U;
            garbageFlag = false;
        }

        [[nodiscard]] auto getExecutionTime() const -> double {
            return runtime;
        }

    private:
        double                                  runtime  = 0.;
        std::size_t                             numGates = 0U;
        std::unique_ptr<dd::Package<>>          ddSynth;
        std::shared_ptr<qc::QuantumComputation> qc;

        // n -> No. of primary inputs.
        // m -> No. of primary outputs.
        // totalNoBits -> Total no. of bits required to create the circuit
        // r -> Additional variables/bits required to decode the output patterns.

        std::size_t n           = 0U;
        std::size_t m           = 0U;
        std::size_t totalNoBits = 0U;
        std::size_t r           = 0U;
        bool        garbageFlag = false;

        auto pathFromSrcDst(dd::mEdge const& src, dd::mNode* const& dst, TruthTable::Cube::Set& sigVec) const -> void;
        auto pathFromSrcDst(dd::mEdge const& src, dd::mNode* const& dst, TruthTable::Cube::Set& sigVec, TruthTable::Cube& cube) const -> void;

        [[nodiscard]] auto finalSrcPathSignature(dd::mEdge const& src, dd::mEdge const& current, TruthTable::Cube::Set const& p1SigVec, TruthTable::Cube::Set const& p2SigVec, bool const& changePaths, std::unique_ptr<dd::Package<>>& dd) const -> TruthTable::Cube::Set;

        auto pathSignature(dd::mEdge const& src, TruthTable::Cube::Set& sigVec) const -> void;
        auto pathSignature(dd::mEdge const& src, TruthTable::Cube::Set& sigVec, TruthTable::Cube& cube) const -> void;

        static auto completeUniCubes(TruthTable::Cube::Set const& p1SigVec, TruthTable::Cube::Set const& p2SigVec, TruthTable::Cube::Set& uniqueCubeVec) -> void;

        auto applyOperation(dd::QubitCount const& totalBits, dd::Qubit const& targetBit, dd::mEdge& to, dd::Controls const& ctrl, std::unique_ptr<dd::Package<>>& dd) -> void;

        static auto controlRoot(dd::mEdge const& current, dd::Controls& ctrl, TruthTable::Cube const& ctrlCube) -> void;
        static auto controlNonRoot(dd::mEdge const& current, dd::Controls& ctrl, TruthTable::Cube const& ctrlCube) -> void;

        static auto dcNodeCondition(dd::mEdge const& current) -> bool;

        auto swapPaths(dd::mEdge src, dd::mEdge const& current, TruthTable::Cube::Set const& p1SigVec, TruthTable::Cube::Set const& p2SigVec, TruthTable::Cube::Set const& p3SigVec, TruthTable::Cube::Set const& p4SigVec, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;

        auto        shiftUniquePaths(dd::mEdge src, dd::mEdge const& current, TruthTable::Cube::Set const& p1SigVec, TruthTable::Cube::Set const& p2SigVec, TruthTable::Cube::Set const& p3SigVec, TruthTable::Cube::Set const& p4SigVec, bool& changePaths, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;
        static auto terminate(dd::mEdge const& current, TruthTable::Cube::Set const& p1SigVec, TruthTable::Cube::Set const& p2SigVec, TruthTable::Cube::Set const& p3SigVec, TruthTable::Cube::Set const& p4SigVec) -> bool;

        auto unifyPath(dd::mEdge src, dd::mEdge const& current, TruthTable::Cube::Set const& p1SigVec, TruthTable::Cube::Set const& p2SigVec, bool const& changePaths, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;
        auto shiftingPaths(dd::mEdge const& src, dd::mEdge const& current, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge;

        template<class T>
        auto decoder(T const& codewords) -> void;

        auto initializeSynthesizer(TruthTable const& tt) -> void;

        auto buildAndSynthesize(TruthTable const& tt) -> void;

        auto synthesizeOnePassTT(TruthTable tt) -> std::shared_ptr<qc::QuantumComputation>;

        auto synthesizeCodingTechniquesTT(TruthTable tt, bool encodeWithoutAdditionalLine) -> std::shared_ptr<qc::QuantumComputation>;
    };

} // namespace syrec
