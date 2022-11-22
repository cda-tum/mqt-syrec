#include "algorithms/synthesis/dd_synthesis.hpp"

using namespace dd::literals;

namespace syrec {

    auto buildDD(const TruthTable& tt, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge {
        // truth table has to have the same number of inputs and outputs
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
                const auto index = (static_cast<std::size_t>(out) * 2U) + static_cast<std::size_t>(in);
                edges.at(index)  = dd::mEdge::one;
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
            subTables.at(index).try_emplace(std::move(reducedInput), std::move(reducedOutput));
        }

        // recursively build the DD for each sub-table
        for (std::size_t i = 0U; i < 4U; ++i) {
            edges.at(i) = buildDD(subTables.at(i), dd);
            // free up the memory used by the sub-table as fast as possible.
            subTables.at(i).clear();
        }

        const auto label = static_cast<dd::Qubit>(tt.nInputs() - 1U);
        return dd->makeDDNode(label, edges);
    }

    // This algorithm provides all paths with their signatures from the `src` node to the `current` node.
    // Refer to the control path section of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf
    auto DDSynthesizer::pathFromSrcDst(dd::mEdge const& src, dd::mNode* const& dst, TruthTable::Cube::Vector& sigVec) const -> void {
        if (src.p->v <= dst->v) {
            if (src.p == dst) {
                sigVec.emplace_back();
            }
            return;
        }
        TruthTable::Cube cube{};
        const auto       pathLength = static_cast<std::size_t>(src.p->v - dst->v);
        cube.reserve(pathLength);
        pathFromSrcDst(src, dst, sigVec, cube);
    }

    auto DDSynthesizer::pathFromSrcDst(dd::mEdge const& src, dd::mNode* const& dst, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& cube) const -> void {
        if (src.p->v <= dst->v) {
            if (src.p == dst) {
                sigVec.emplace_back(cube);
            }
            return;
        }

        cube.emplace_back(false);
        pathFromSrcDst(src.p->e[0], dst, sigVec, cube);
        cube.pop_back();

        cube.emplace_back(true);
        pathFromSrcDst(src.p->e[3], dst, sigVec, cube);
        cube.pop_back();
    }

    //please refer to the second optimization approach introduced in https://www.cda.cit.tum.de/files/eda/2017_rc_improving_qmdd_synthesis_of_reversible_circuits.pdf
    auto DDSynthesizer::finalSrcPathSignature(dd::mEdge const& src, dd::mEdge const& current, TruthTable::Cube::Vector const& p1SigVec, TruthTable::Cube::Vector const& p2SigVec, std::unique_ptr<dd::Package<>>& dd) const -> TruthTable::Cube::Vector {
        if (current.p->v == src.p->v || current.p->v == 0U) {
            TruthTable::Cube::Vector rootSigVec;
            pathFromSrcDst(src, current.p, rootSigVec);
            return rootSigVec;
        }

        TruthTable::Cube::Vector rootSigVec;
        pathFromSrcDst(src, current.p, rootSigVec);

        const auto tables = dd->mUniqueTable.getTables();

        auto const& table = tables[current.p->v];

        for (auto* p: table) {
            if (p != nullptr && p != current.p) {
                TruthTable::Cube::Vector p1Vec;
                pathSignature(p->e[0], p1Vec);

                TruthTable::Cube::Vector p2Vec;
                pathSignature(p->e[1], p2Vec);

                if ((p1SigVec.size() == p1Vec.size() && p2SigVec.size() == p2Vec.size()) && std::is_permutation(p1SigVec.begin(), p1SigVec.end(), p1Vec.begin()) && std::is_permutation(p2SigVec.begin(), p2SigVec.end(), p2Vec.begin())) {
                    TruthTable::Cube::Vector rootSigVecTmp;
                    pathFromSrcDst(src, p, rootSigVecTmp);
                    std::move(rootSigVecTmp.begin(), rootSigVecTmp.end(), std::back_inserter(rootSigVec));
                }
            }
        }
        return rootSigVec;
    }

    // This algorithm provides all the paths with their signatures for the `src` node.
    // Refer to signature path section of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf
    auto DDSynthesizer::pathSignature(dd::mEdge const& src, TruthTable::Cube::Vector& sigVec) const -> void {
        TruthTable::Cube cube{};
        const auto       pathLength = static_cast<std::size_t>(src.p->v + 1);
        cube.reserve(pathLength);
        pathSignature(src, sigVec, cube);
    }

    auto DDSynthesizer::pathSignature(dd::mEdge const& src, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& cube) const -> void {
        const auto nEdges = src.p->e.size();
        if (src.p->v == 0) {
            for (auto i = 0U; i < nEdges; ++i) {
                if (src.p->e.at(i) == dd::mEdge::one) {
                    cube.emplace_back((i == 1U || i == 3U));
                    sigVec.emplace_back(cube);
                    cube.pop_back();
                }
            }
            return;
        }

        if (dd::mNode::isTerminal(src.p)) {
            return;
        }

        for (auto i = 0U; i < nEdges; ++i) {
            cube.emplace_back((i == 1U || i == 3U));
            pathSignature(src.p->e.at(i), sigVec, cube);
            cube.pop_back();
        }
    }

    // This function stores all the controls of the `current` node not concerning the root/src of the DD.
    auto DDSynthesizer::controlNonRoot(dd::mEdge const& current, dd::Controls& ctrl, TruthTable::Cube const& ctrlCube) -> void {
        const auto cubeSize = ctrlCube.size();
        for (auto i = 0U; i < cubeSize; ++i) {
            if (ctrlCube[i].has_value()) {
                const auto idx = static_cast<dd::Qubit>(static_cast<std::size_t>(current.p->v) - i - 1U);
                if (*ctrlCube[i]) {
                    ctrl.emplace(dd::Control{idx, dd::Control::Type::pos});
                } else {
                    ctrl.emplace(dd::Control{idx, dd::Control::Type::neg});
                }
            }
        }
    }

    // This function stores all the controls of the `current` node concerning the root/src of the DD.
    auto DDSynthesizer::controlRoot(dd::mEdge const& current, dd::Controls& ctrl, TruthTable::Cube const& ctrlCube) -> void {
        const auto cubeSize = ctrlCube.size();
        for (auto i = 0U; i < cubeSize; ++i) {
            if (ctrlCube[i].has_value()) {
                const auto idx = static_cast<dd::Qubit>((cubeSize - i) + static_cast<std::size_t>(current.p->v));
                if (*ctrlCube[i]) {
                    ctrl.emplace(dd::Control{idx, dd::Control::Type::pos});
                } else {
                    ctrl.emplace(dd::Control{idx, dd::Control::Type::neg});
                }
            }
        }
    }

    // This function performs the multi-control (if any) X operation.
    auto DDSynthesizer::applyOperation(dd::QubitCount const& totalBits, dd::Qubit const& targetBit, dd::mEdge& to, dd::Controls const& ctrl, std::unique_ptr<dd::Package<>>& dd) -> void {
        // create operation and corresponding decision diagram
        auto       op   = std::make_unique<qc::StandardOperation>(totalBits, ctrl, targetBit, qc::X);
        const auto opDD = dd::getDD(op.get(), dd);

        const auto tmp = dd->multiply(to, opDD);
        dd->incRef(tmp);
        dd->decRef(to);
        to = tmp;
        dd->garbageCollect();

        qc.emplace_back(op);
        ++numGates;
    }

    // This algorithm swaps the paths present in the p' edge to the n edge and vice versa.
    // Refer to the P1 algorithm of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf
    auto DDSynthesizer::swapPaths(dd::mEdge src, dd::mEdge const& current, TruthTable::Cube::Vector const& p1SigVec, TruthTable::Cube::Vector const& p2SigVec, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge {
        if ((current.p->e[0].isZeroTerminal() && !current.p->e[1].isZeroTerminal()) || (p2SigVec.size() > p1SigVec.size())) {
            auto rootSigVec = finalSrcPathSignature(src, current, p1SigVec, p2SigVec, dd);

            const auto rootSolution = minbool::minimizeBoolean(rootSigVec);

            const auto nQubits = static_cast<dd::QubitCount>(src.p->v + 1);

            if (rootSolution.empty()) {
                applyOperation(nQubits, current.p->v, src, {}, dd);
            }

            for (auto const& rootVec: rootSolution) {
                dd::Controls ctrlFinal;
                controlRoot(current, ctrlFinal, rootVec);
                applyOperation(nQubits, current.p->v, src, ctrlFinal, dd);
            }
        }
        return src;
    }

    // This algorithm moves the unique paths present in the p' edge to the n edge.
    // Refer to the P2 algorithm of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf
    auto DDSynthesizer::shiftUniquePaths(dd::mEdge src, dd::mEdge const& current, TruthTable::Cube::Vector const& p1SigVec, TruthTable::Cube::Vector const& p2SigVec, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge {
        TruthTable::Cube::Vector uniqueCubeVec;

        // Collect all the unique p' paths.
        for (const auto& p2Cube: p2SigVec) {
            const auto it = std::find(p1SigVec.begin(), p1SigVec.end(), p2Cube);
            if (it == p1SigVec.end()) {
                uniqueCubeVec.emplace_back(p2Cube);
            }
        }

        if (uniqueCubeVec.empty()) {
            return src;
        }

        auto rootSigVec = finalSrcPathSignature(src, current, p1SigVec, p2SigVec, dd);

        const auto rootSolution = minbool::minimizeBoolean(rootSigVec);
        const auto uniSolution  = minbool::minimizeBoolean(uniqueCubeVec);

        const auto nQubits = static_cast<dd::QubitCount>(src.p->v + 1);
        for (auto const& uniCube: uniSolution) {
            dd::Controls ctrlNonRoot;
            controlNonRoot(current, ctrlNonRoot, uniCube);

            if (rootSolution.empty()) {
                applyOperation(nQubits, current.p->v, src, ctrlNonRoot, dd);
            }

            for (auto const& rootVec: rootSolution) {
                dd::Controls ctrlFinal = ctrlNonRoot;
                controlRoot(current, ctrlFinal, rootVec);
                applyOperation(nQubits, current.p->v, src, ctrlFinal, dd);
            }
        }
        return src;
    }

    // This algorithm checks whether the p' edge is pointing to zero terminal node.
    // Refer to P3 algorithm of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf
    auto DDSynthesizer::terminate(dd::mEdge const& current) -> bool {
        if (!(dd::mNode::isTerminal(current.p))) {
            return current.p->e[1].isZeroTerminal();
        }
        return false;
    }

    // This algorithm modifies the non-unique paths present in the p' edge to unique paths.
    // Refer to P4 algorithm of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf
    auto DDSynthesizer::unifyPath(dd::mEdge src, dd::mEdge const& current, TruthTable::Cube::Vector const& p1SigVec, TruthTable::Cube::Vector const& p2SigVec, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge {
        std::vector<std::size_t> indices;
        const auto               sig2Size = p2SigVec.size();
        for (std::size_t index = 0; index < sig2Size; ++index) {
            if (const auto it = std::find(p1SigVec.begin(), p1SigVec.end(), p2SigVec[index]); it != p1SigVec.end()) {
                indices.emplace_back(index);
            }
        }

        // return one of the missing cubes.
        const auto missCube = TruthTable::Cube::findMissingCube(p1SigVec);

        TruthTable::Cube ctrlVec;
        ctrlVec.resize(p2SigVec[indices[0]].size());

        TruthTable::Cube targetVec;
        targetVec.resize(p2SigVec[indices[0]].size());

        // accordingly store the controls and targets.
        const auto sigLength = p2SigVec[indices[0]].size();
        for (std::size_t p2Obj = 0; p2Obj < sigLength; ++p2Obj) {
            if (p2SigVec[indices[0]][p2Obj] == missCube[p2Obj]) {
                ctrlVec[p2Obj] = missCube[p2Obj];
            } else {
                targetVec[p2Obj] = true;
            }
        }

        dd::Controls ctrlNonRoot;
        controlNonRoot(current, ctrlNonRoot, ctrlVec);

        auto rootSigVec = finalSrcPathSignature(src, current, p1SigVec, p2SigVec, dd);

        const auto nQubits = static_cast<dd::QubitCount>(src.p->v + 1);

        const auto rootSolution = minbool::minimizeBoolean(rootSigVec);

        const auto targetSize = targetVec.size();

        if (rootSolution.empty()) {
            ctrlNonRoot.emplace(dd::Control{current.p->v, dd::Control::Type::pos});
            for (std::size_t i = 0; i < targetSize; ++i) {
                if (targetVec[i].has_value() && *(targetVec[i])) {
                    applyOperation(nQubits, static_cast<dd::Qubit>(static_cast<std::size_t>(current.p->v) - (i + 1U)), src, ctrlNonRoot, dd);
                }
            }
        }

        for (auto const& rootVec: rootSolution) {
            dd::Controls ctrlFinal;
            controlRoot(current, ctrlFinal, rootVec);
            ctrlFinal.emplace(dd::Control{current.p->v, dd::Control::Type::pos});
            ctrlFinal.insert(ctrlNonRoot.begin(), ctrlNonRoot.end());

            for (std::size_t i = 0; i < targetSize; ++i) {
                if (targetVec[i].has_value() && *(targetVec[i])) {
                    applyOperation(nQubits, static_cast<dd::Qubit>(static_cast<std::size_t>(current.p->v) - (i + 1U)), src, ctrlFinal, dd);
                }
            }
        }
        return src;
    }

    // This algorithm ensures that the `current` node has the identity structure.
    // Refer to algorithm P of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf)
    auto DDSynthesizer::shiftingPaths(dd::mEdge const& src, dd::mEdge const& current, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge {
        if (dd::mNode::isTerminal(current.p)) {
            return src;
        }

        TruthTable::Cube::Vector p1SigVec;
        pathSignature(current.p->e[0], p1SigVec);

        TruthTable::Cube::Vector p2SigVec;
        pathSignature(current.p->e[1], p2SigVec);

        // P1 algorithm.
        if (const auto srcSwapped = swapPaths(src, current, p1SigVec, p2SigVec, dd); srcSwapped != src) {
            return srcSwapped;
        }

        // P2 algorithm.
        if (const auto srcUnique = shiftUniquePaths(src, current, p1SigVec, p2SigVec, dd); srcUnique != src) {
            return srcUnique;
        }

        // P3 algorithm.
        if (terminate(current)) {
            return src;
        }

        // P4 algorithm.
        return unifyPath(src, current, p1SigVec, p2SigVec, dd);
    }

    // This function returns the operations required to synthesize the DD.
    auto DDSynthesizer::synthesize(dd::mEdge src, std::unique_ptr<dd::Package<>>& dd) -> qc::QuantumComputation& {
        qc.clear();
        runtime  = 0.;
        numGates = 0U;

        if (src.p == nullptr || src.p->isIdentity()) {
            return qc;
        }

        // This following ensures that the `src` node resembles an identity structure.
        // Refer to algorithm Q of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf.

        const auto start = std::chrono::steady_clock::now();

        // to preserve the `src` DD throughout the synthesis, its reference count has to be at least 2.
        while (src.p->ref < 2U) {
            dd->incRef(src);
        }

        // queue for the nodes to be processed in a breadth-first manner.
        std::queue<dd::mEdge> queue{};
        queue.emplace(src);

        // set of nodes that have already been processed.
        std::unordered_set<dd::mEdge> visited{};

        // while there are nodes left to process.
        while (!queue.empty()) {
            const auto current = queue.front();
            queue.pop();

            if (current.p == nullptr || current.p->isIdentity()) {
                continue;
            }

            // shift the paths of the current node.

            // first, increment the reference count of the `src` node,
            // to prevent it from being garbage collected in the `shiftingPaths` call.
            dd->incRef(src);

            // perform the shifting paths algorithm.
            const auto srcShifted   = shiftingPaths(src, current, dd);
            const auto pathsShifted = (srcShifted != src);

            // decrement reference count of `src` node again and trigger garbage collection.
            dd->decRef(src);
            dd->garbageCollect();

            if (pathsShifted) {
                // stopping criterion
                if (srcShifted.p->isIdentity()) {
                    break;
                }

                // if paths were shifted, synthesis starts again from the new `src` node.
                src = srcShifted;
                visited.clear();
                queue = {};
                queue.emplace(src);
                continue;
            }

            // if all paths have been shifted, the children of the current node need to be processed.
            for (const auto& e: current.p->e) {
                if (!e.isTerminal() && visited.find(e) == visited.end()) {
                    queue.emplace(e);
                    visited.emplace(e);
                }
            }
        }
        runtime = static_cast<double>((std::chrono::steady_clock::now() - start).count());

        return qc;
    }

} // namespace syrec
