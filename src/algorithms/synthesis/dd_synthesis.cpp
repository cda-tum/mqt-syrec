#include "algorithms/synthesis/dd_synthesis.hpp"

#include "algorithms/synthesis/encoding.hpp"

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

                const auto in  = *input[0];
                const auto out = *output[0];

                if (output[0].has_value()) {
                    const auto index = (static_cast<std::size_t>(out) * 2U) + static_cast<std::size_t>(in);
                    edges.at(index)  = dd::mEdge::one;
                } else {
                    const auto offset     = in ? 1U : 0U;
                    edges.at(0U + offset) = dd::mEdge::one;
                    edges.at(2U + offset) = dd::mEdge::one;
                }
            }
            return dd->makeDDNode(0, edges);
        }

        // generate sub-tables
        std::array<TruthTable, 4U> subTables{};
        for (const auto& [input, output]: tt) {
            // truth table has to be completely specified
            assert(input[0].has_value());

            const auto in  = *input[0];
            const auto out = *output[0];

            TruthTable::Cube reducedInput(input.begin() + 1, input.end());
            TruthTable::Cube reducedOutput(output.begin() + 1, output.end());

            if (output[0].has_value()) {
                const auto index = static_cast<std::size_t>(out) * 2U + static_cast<std::size_t>(in);
                subTables.at(index).try_emplace(std::move(reducedInput), std::move(reducedOutput));
            } else {
                const auto offset = in ? 1U : 0U;
                subTables.at(0 + offset).try_emplace(reducedInput, reducedOutput);
                subTables.at(2 + offset).try_emplace(reducedInput, reducedOutput);
            }
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
    auto DDSynthesizer::pathFromSrcDst(dd::mEdge const& src, dd::mNode* const& dst, TruthTable::Cube::Set& sigVec) const -> void {
        if (src.p->v <= dst->v) {
            if (src.p == dst) {
                sigVec.emplace();
            }
            return;
        }
        TruthTable::Cube cube{};
        const auto       pathLength = static_cast<std::size_t>(src.p->v - dst->v);
        cube.reserve(pathLength);
        pathFromSrcDst(src, dst, sigVec, cube);
    }

    auto DDSynthesizer::pathFromSrcDst(dd::mEdge const& src, dd::mNode* const& dst, TruthTable::Cube::Set& sigVec, TruthTable::Cube& cube) const -> void {
        if (src.p->v <= dst->v) {
            if (src.p == dst) {
                sigVec.emplace(cube);
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
    auto DDSynthesizer::finalSrcPathSignature(dd::mEdge const& src, dd::mEdge const& current, TruthTable::Cube::Set const& p1SigVec, TruthTable::Cube::Set const& p2SigVec, bool const& changePaths, std::unique_ptr<dd::Package<>>& dd) const -> TruthTable::Cube::Set {
        if (current.p->v == src.p->v || current.p->v == 0U) {
            TruthTable::Cube::Set rootSigVec;
            pathFromSrcDst(src, current.p, rootSigVec);
            return rootSigVec;
        }

        TruthTable::Cube::Set rootSigVec;
        pathFromSrcDst(src, current.p, rootSigVec);

        const auto tables = dd->mUniqueTable.getTables();

        auto const& table = tables[current.p->v];

        for (auto* p: table) {
            if (p != nullptr && p != current.p && p->ref > 0) {
                TruthTable::Cube::Set p1Vec;
                TruthTable::Cube::Set p2Vec;
                if (changePaths) {
                    pathSignature(p->e[3], p1Vec);
                    pathSignature(p->e[2], p2Vec);
                } else {
                    pathSignature(p->e[0], p1Vec);
                    pathSignature(p->e[1], p2Vec);
                }

                if (p1SigVec == p1Vec && p2SigVec == p2Vec) {
                    TruthTable::Cube::Set rootSigVecTmp;
                    pathFromSrcDst(src, p, rootSigVecTmp);
                    rootSigVec.merge(rootSigVecTmp);
                }
            }
        }
        return rootSigVec;
    }

    // This algorithm provides all the paths with their signatures for the `src` node.
    // Refer to signature path section of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf
    auto DDSynthesizer::pathSignature(dd::mEdge const& src, TruthTable::Cube::Set& sigVec) const -> void {
        TruthTable::Cube cube{};
        const auto       pathLength = static_cast<std::size_t>(src.p->v + 1);
        cube.reserve(pathLength);
        pathSignature(src, sigVec, cube);
    }

    auto DDSynthesizer::pathSignature(dd::mEdge const& src, TruthTable::Cube::Set& sigVec, TruthTable::Cube& cube) const -> void {
        const auto nEdges = src.p->e.size();
        if (src.p->v == 0) {
            for (auto i = 0U; i < nEdges; ++i) {
                if (src.p->e.at(i) == dd::mEdge::one) {
                    cube.emplace_back((i == 1U || i == 3U));
                    sigVec.emplace(cube);
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

    auto DDSynthesizer::completeUniCubes(TruthTable::Cube::Set const& p1SigVec, TruthTable::Cube::Set const& p2SigVec, TruthTable::Cube::Set& uniqueCubeVec) -> void {
        for (const auto& p2Cube: p2SigVec) {
            const auto it = std::find(p1SigVec.begin(), p1SigVec.end(), p2Cube);
            if (it == p1SigVec.end()) {
                uniqueCubeVec.emplace(p2Cube);
            }
        }
    }

    // This function stores all the controls of the `current` node not concerning the root/src of the DD.
    auto DDSynthesizer::controlNonRoot(dd::mEdge const& current, dd::Controls& ctrl, TruthTable::Cube const& ctrlCube) -> void {
        const auto cubeSize = ctrlCube.size();
        for (auto i = 0U; i < cubeSize; ++i) {
            if (ctrlCube[i].has_value()) {
                const auto idx      = static_cast<dd::Qubit>(static_cast<std::size_t>(current.p->v) - i - 1U);
                const auto ctrlType = *ctrlCube[i] ? dd::Control::Type::pos : dd::Control::Type::neg;
                ctrl.emplace(dd::Control{idx, ctrlType});
            }
        }
    }

    // This function stores all the controls of the `current` node concerning the root/src of the DD.
    auto DDSynthesizer::controlRoot(dd::mEdge const& current, dd::Controls& ctrl, TruthTable::Cube const& ctrlCube) -> void {
        const auto cubeSize = ctrlCube.size();
        for (auto i = 0U; i < cubeSize; ++i) {
            if (ctrlCube[i].has_value()) {
                const auto idx      = static_cast<dd::Qubit>((cubeSize - i) + static_cast<std::size_t>(current.p->v));
                const auto ctrlType = *ctrlCube[i] ? dd::Control::Type::pos : dd::Control::Type::neg;
                ctrl.emplace(dd::Control{idx, ctrlType});
            }
        }
    }

    // Check whether all the edges of the current node are pointing to the same node (indicating a don't care node).
    auto DDSynthesizer::dcNodeCondition(dd::mEdge const& current) -> bool {
        if (!(dd::mNode::isTerminal(current.p))) {
            if (current.p->v == 0U) {
                return std::all_of(current.p->e.begin(), current.p->e.end(), [](const auto& e) { return e == dd::mEdge::one; });
            }

            return std::all_of(current.p->e.begin(), current.p->e.end(), [&current](const auto& e) { return e.p == current.p->e[0].p; });
        }
        return false;
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

        qc->emplace_back(op);
        ++numGates;
    }

    // This algorithm swaps the paths present in the p' edge to the n edge and vice versa.
    // If n' and p paths exists, we move on to P2 algorithm
    // Refer to the P1 algorithm of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf
    auto DDSynthesizer::swapPaths(dd::mEdge src, dd::mEdge const& current, TruthTable::Cube::Set const& p1SigVec, TruthTable::Cube::Set const& p2SigVec, TruthTable::Cube::Set const& p3SigVec, TruthTable::Cube::Set const& p4SigVec, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge {
        if (p2SigVec.size() > p1SigVec.size() || (p2SigVec.empty() && p1SigVec.empty())) {
            if (p2SigVec.empty()) {
                if (p3SigVec.empty() && p4SigVec.empty() && ((!current.p->e[0].isZeroTerminal() && current.p->e[1].isZeroTerminal()) || (!current.p->e[3].isZeroTerminal() && current.p->e[2].isZeroTerminal()))) {
                    return src;
                }
                if (!p3SigVec.empty() || !p4SigVec.empty()) {
                    return src;
                }
            }

            if (!p2SigVec.empty() && p1SigVec == p3SigVec && p2SigVec == p4SigVec) {
                return src;
            }

            auto       rootSigVec   = finalSrcPathSignature(src, current, p1SigVec, p2SigVec, false, dd);
            const auto rootSolution = minbool::minimizeBoolean(rootSigVec);

            const auto nQubits = static_cast<dd::QubitCount>(src.p->v + 1);

            for (auto const& rootVec: rootSolution) {
                dd::Controls ctrlFinal;
                controlRoot(current, ctrlFinal, rootVec);
                applyOperation(nQubits, current.p->v, src, ctrlFinal, dd);
            }
        }
        return src;
    }

    // This algorithm moves the unique paths present in the p' edge to the n edge.
    // If there are no unique paths in p' edge, the unique paths present in the n' edge are moved to the p edge if required.
    // Refer to the P2 algorithm of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf
    auto DDSynthesizer::shiftUniquePaths(dd::mEdge src, dd::mEdge const& current, TruthTable::Cube::Set const& p1SigVec, TruthTable::Cube::Set const& p2SigVec, TruthTable::Cube::Set const& p3SigVec, TruthTable::Cube::Set const& p4SigVec, bool& changePaths, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge {
        if (p2SigVec.empty()) {
            if (p3SigVec.empty() || (p1SigVec == p3SigVec && p2SigVec == p4SigVec)) {
                return src;
            }
            changePaths = true;
        }

        if (p1SigVec == p3SigVec && p2SigVec == p4SigVec) {
            return src;
        }

        TruthTable::Cube::Set uniqueCubeVec;
        // Collect all the unique paths.
        if (changePaths) {
            completeUniCubes(p4SigVec, p3SigVec, uniqueCubeVec);
        } else {
            completeUniCubes(p1SigVec, p2SigVec, uniqueCubeVec);
        }

        if (uniqueCubeVec.empty()) {
            return src;
        }

        TruthTable::Cube::Set rootSigVec;
        if (changePaths) {
            rootSigVec = finalSrcPathSignature(src, current, p4SigVec, p3SigVec, changePaths, dd);
        } else {
            rootSigVec = finalSrcPathSignature(src, current, p1SigVec, p2SigVec, changePaths, dd);
        }

        const auto rootSolution = minbool::minimizeBoolean(rootSigVec);
        const auto uniSolution  = minbool::minimizeBoolean(uniqueCubeVec);

        const auto nQubits = static_cast<dd::QubitCount>(src.p->v + 1);
        for (auto const& uniCube: uniSolution) {
            dd::Controls ctrlNonRoot;
            controlNonRoot(current, ctrlNonRoot, uniCube);

            for (auto const& rootVec: rootSolution) {
                dd::Controls ctrlFinal = ctrlNonRoot;
                controlRoot(current, ctrlFinal, rootVec);
                applyOperation(nQubits, current.p->v, src, ctrlFinal, dd);
            }
        }
        return src;
    }

    // This algorithm checks whether the p' edge is pointing to zero terminal node.
    // This algorithm also checks if n paths == n' paths and p' paths == p paths.
    // Refer to P3 algorithm of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf
    auto DDSynthesizer::terminate(dd::mEdge const& current, TruthTable::Cube::Set const& p1SigVec, TruthTable::Cube::Set const& p2SigVec, TruthTable::Cube::Set const& p3SigVec, TruthTable::Cube::Set const& p4SigVec) -> bool {
        if (!(dd::mNode::isTerminal(current.p))) {
            return ((p1SigVec == p3SigVec && p2SigVec == p4SigVec) || (current.p->e[1].isZeroTerminal() && current.p->e[2].isZeroTerminal()));
        }
        return false;
    }

    // This algorithm modifies the non-unique paths present in the p' or n (based on changePaths) edge to unique paths.
    // Refer to P4 algorithm of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf
    auto DDSynthesizer::unifyPath(dd::mEdge src, dd::mEdge const& current, TruthTable::Cube::Set const& p1SigVec, TruthTable::Cube::Set const& p2SigVec, bool const& changePaths, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge {
        TruthTable::Cube repeatedCube;
        for (auto const& p2Obj: p2SigVec) {
            if (const auto it = std::find(p1SigVec.begin(), p1SigVec.end(), p2Obj); it != p1SigVec.end()) {
                repeatedCube = p2Obj;
            }
        }

        // return one of the missing cubes.
        const auto missCube = TruthTable::Cube::findMissingCube(p1SigVec);

        TruthTable::Cube ctrlVec;
        ctrlVec.resize(repeatedCube.size());

        TruthTable::Cube targetVec;
        targetVec.resize(repeatedCube.size());

        // accordingly store the controls and targets.
        const auto sigLength = repeatedCube.size();
        for (std::size_t p2Obj = 0; p2Obj < sigLength; ++p2Obj) {
            if (repeatedCube[p2Obj] == missCube[p2Obj]) {
                ctrlVec[p2Obj] = missCube[p2Obj];
            } else {
                targetVec[p2Obj] = true;
            }
        }

        dd::Controls ctrlNonRoot;
        controlNonRoot(current, ctrlNonRoot, ctrlVec);

        auto rootSigVec = finalSrcPathSignature(src, current, p1SigVec, p2SigVec, changePaths, dd);

        const auto nQubits = static_cast<dd::QubitCount>(src.p->v + 1);

        const auto rootSolution = minbool::minimizeBoolean(rootSigVec);

        const auto targetSize = targetVec.size();

        for (auto const& rootVec: rootSolution) {
            dd::Controls ctrlFinal;
            controlRoot(current, ctrlFinal, rootVec);

            const auto ctrlType = changePaths ? dd::Control::Type::neg : dd::Control::Type::pos;
            ctrlFinal.emplace(dd::Control{current.p->v, ctrlType});

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

        TruthTable::Cube::Set p1SigVec;
        pathSignature(current.p->e[0], p1SigVec);

        TruthTable::Cube::Set p2SigVec;
        pathSignature(current.p->e[1], p2SigVec);

        TruthTable::Cube::Set p3SigVec;
        pathSignature(current.p->e[2], p3SigVec);

        TruthTable::Cube::Set p4SigVec;
        pathSignature(current.p->e[3], p4SigVec);

        auto changePaths = false;

        // P1 algorithm.
        if (const auto srcSwapped = swapPaths(src, current, p1SigVec, p2SigVec, p3SigVec, p4SigVec, dd); srcSwapped != src) {
            return srcSwapped;
        }

        // P2 algorithm.
        // If there are no unique paths in p' edge, the unique paths present in the n' edge are moved to the p edge if required. changePaths flag is set accordingly.
        if (const auto srcUnique = shiftUniquePaths(src, current, p1SigVec, p2SigVec, p3SigVec, p4SigVec, changePaths, dd); srcUnique != src) {
            return srcUnique;
        }

        // P3 algorithm.
        if (terminate(current, p1SigVec, p2SigVec, p3SigVec, p4SigVec)) {
            return src;
        }

        // P4 algorithm.
        // if changePaths flag is set, p and n' edges are considered instead of n and p' edges.
        if (changePaths) {
            return unifyPath(src, current, p4SigVec, p3SigVec, changePaths, dd);
        }
        return unifyPath(src, current, p1SigVec, p2SigVec, changePaths, dd);
    }

    // Refer to the decoder algorithm of https://www.cda.cit.tum.de/files/eda/2018_aspdac_coding_techniques_in_synthesis.pdf.
    auto DDSynthesizer::decoder(TruthTable::CubeMap const& codewords) -> void {
        const auto codeLength = codewords.begin()->second.size();

        // decode the r most significant bits of the original output pattern.
        if (r != 0U) {
            for (const auto& [pattern, code]: codewords) {
                TruthTable::Cube targetCube(pattern.begin(), pattern.begin() + static_cast<int>(r));

                dd::Controls ctrl;
                for (auto i = 0U; i < codeLength; i++) {
                    if (code[i].has_value()) {
                        const auto ctrlType = *code[i] ? dd::Control::Type::pos : dd::Control::Type::neg;
                        ctrl.emplace(dd::Control{static_cast<dd::Qubit>((codeLength - 1U) - i), ctrlType});
                    }
                }

                const auto targetSize = targetCube.size();

                for (std::size_t i = 0U; i < targetSize; ++i) {
                    if (targetCube[i].has_value() && *(targetCube[i])) {
                        const auto targetBit = static_cast<dd::Qubit>((totalNoBits - 1U) - i);
                        qc->x(targetBit, ctrl);
                        ++numGates;
                    }
                }
            }
        }

        // decode the remaining (m − r) primary outputs.
        const auto correctionBits = m - r;

        if (correctionBits == 0U) {
            return;
        }

        TruthTable ttCorrection{};

        for (const auto& [pattern, code]: codewords) {
            TruthTable::Cube outCube(pattern);
            outCube.resize(totalNoBits);

            TruthTable::Cube inCube(pattern.begin(), pattern.begin() + static_cast<int>(r));
            for (auto i = 0U; i < codeLength; i++) {
                inCube.emplace_back(code[i]);
            }

            // Extend the dc in the inputs.
            const auto completeInputs = inCube.completeCubes();
            for (auto const& completeInput: completeInputs) {
                ttCorrection.try_emplace(completeInput, outCube);
            }
        }

        const auto ttCorrectionDD = buildDD(ttCorrection, ddSynth);
        garbageFlag               = true;
        synthesize(ttCorrectionDD, ddSynth);
    }

    // This function returns the operations required to synthesize the DD.
    auto DDSynthesizer::synthesize(dd::mEdge src, std::unique_ptr<dd::Package<>>& dd) -> std::shared_ptr<qc::QuantumComputation> {
        if (src.p == nullptr || src.p->isIdentity() || dcNodeCondition(src)) {
            return qc;
        }

        totalNoBits = static_cast<std::size_t>(src.p->v + 1);

        // construct qc only if it is pointing to null
        if (qc == nullptr) {
            qc = std::make_shared<qc::QuantumComputation>(totalNoBits);
        }

        // The threshold after which the outputs are considered to be garbage.
        const auto garbageThreshold = static_cast<dd::Qubit>(totalNoBits) - static_cast<dd::Qubit>(m);

        // This following ensures that the `src` node resembles an identity structure.
        // Refer to algorithm Q of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf.

        // to preserve the `src` DD throughout the synthesis, its reference count has to be at least 2.
        while (src.p->ref < 2U) {
            dd->incRef(src);
        }

        // queue for the nodes to be processed in a breadth-first manner.
        std::queue<dd::mEdge> queue{};
        queue.emplace(src);

        // set of nodes that have already been processed.
        std::unordered_set<dd::mEdge> visited{};

        const auto start = std::chrono::steady_clock::now();

        // while there are nodes left to process.
        while (!queue.empty()) {
            const auto current = queue.front();

            // if the garbageFlag is true, the synthesis is terminated once the garbage threshold is reached.
            if (garbageFlag && current.p->v == garbageThreshold - 1) {
                break;
            }

            queue.pop();

            if (current.p == nullptr || current.p->isIdentity() || dcNodeCondition(current)) {
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
                if (srcShifted.p->isIdentity() && dcNodeCondition(srcShifted)) {
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
                if (!e.isTerminal() && !dcNodeCondition(e) && visited.find(e) == visited.end()) {
                    queue.emplace(e);
                    visited.emplace(e);
                }
            }
        }
        runtime = static_cast<double>((std::chrono::steady_clock::now() - start).count());
        return qc;
    }

    auto DDSynthesizer::synthesizeTT(TruthTable tt, bool onePass) -> std::shared_ptr<qc::QuantumComputation> {
        runtime     = 0.;
        numGates    = 0U;
        garbageFlag = false;

        n = tt.nInputs();
        m = tt.nOutputs();

        extend(tt);

        // k1 -> Minimum no. of additional lines required.
        const auto k1 = tt.minimumAdditionalLinesRequired();

        totalNoBits = std::max(n, m + k1);

        // codewords -> Output patterns with the respective codewords.
        TruthTable::CubeMap codewords;

        // construct ddSynth only if it is pointing to null
        if (ddSynth == nullptr) {
            ddSynth = std::make_unique<dd::Package<>>(totalNoBits);
        }

        // construct qc only if it is pointing to null
        if (qc == nullptr) {
            qc = std::make_shared<qc::QuantumComputation>(totalNoBits);
        }

        // Refer to the one-pass synthesis  algorithm of https://www.cda.cit.tum.de/files/eda/2017_tcad_one_pass_synthesis_reversible_circuits.pdf.
        if (onePass) {
            if (m > n) {
                for (auto i = 0U; i < m - n; i++) {
                    // corresponding bits are considered as ancillary bits.
                    qc->setLogicalQubitAncillary(static_cast<dd::Qubit>((totalNoBits - 1) - i));
                }
                // zeros are inserted to match the length of the output patterns.
                augmentWithConstants(tt, m);
            }

            r = (m + k1) - std::max(n, m);

            for (auto i = 0U; i < r; i++) {
                // corresponding bits are considered as ancillary and garbage bits.
                qc->setLogicalQubitAncillary(static_cast<dd::Qubit>(i));
                qc->setLogicalQubitGarbage(static_cast<dd::Qubit>(i));
            }

            // based on the totalNoBits, zeros are appended to the inputs and the outputs.
            augmentWithConstants(tt, totalNoBits, true);
        } else {
            codewords = encodeHuffman(tt);
            r         = totalNoBits - tt.nOutputs();
            augmentWithConstants(tt, totalNoBits);

            const auto nAncillaBits = totalNoBits - n;
            const auto nGarbageBits = totalNoBits - m;

            for (auto i = 0U; i < nAncillaBits; i++) {
                // corresponding bits are considered as ancillary bits.
                qc->setLogicalQubitAncillary(static_cast<dd::Qubit>((totalNoBits - 1) - i));
            }
            for (auto i = 0U; i < nGarbageBits; i++) {
                // corresponding bits are considered as garbage bits.
                qc->setLogicalQubitGarbage(static_cast<dd::Qubit>(i));
            }
        }

        const auto src = buildDD(tt, ddSynth);

        const auto start = std::chrono::steady_clock::now();

        // If the one-pass synthesis is selected, the appended garbage bits need not be considered during the synthesis process.
        garbageFlag = onePass;
        synthesize(src, ddSynth);

        // if codeword is not empty, the above synthesized encoded function should be decoded.
        if (!codewords.empty()) {
            // synthesizing the corresponding decoder circuit.
            decoder(codewords);
        }

        runtime = static_cast<double>((std::chrono::steady_clock::now() - start).count());
        return qc;
    }

} // namespace syrec
