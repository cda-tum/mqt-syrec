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
        for (std::size_t i = 0U; i < 4U; i++) {
            edges.at(i) = buildDD(subTables.at(i), dd);
            // free up the memory used by the sub-table as fast as possible.
            subTables.at(i).clear();
        }

        const auto label = static_cast<dd::Qubit>(tt.nInputs() - 1U);
        return dd->makeDDNode(label, edges);
    }

    //This algorithm provides the all the paths with their signatures from the node pointed by src to the node pointed by current (refer to control path section of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf)
    auto DDSynthesizer::pathFromSrcDst(dd::mEdge const& src, dd::mEdge const& dst, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& tempVec) const -> void {
        if (src.p->v <= dst.p->v) {
            if (src == dst) {
                sigVec.emplace_back(tempVec);
            }
            return;
        }

        tempVec.emplace_back(false);
        pathFromSrcDst(src.p->e[0], dst, sigVec, tempVec);
        tempVec.pop_back();

        tempVec.emplace_back(true);
        pathFromSrcDst(src.p->e[3], dst, sigVec, tempVec);
        tempVec.pop_back();
    }

    //This algorithm provides the all the paths with their signatures for the node pointed by src (refer to signature path section of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf)
    auto DDSynthesizer::pathSignature(dd::mEdge const& src, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& tempVec) const -> void {
        if (static_cast<std::size_t>(src.p->v) == 0U) {
            for (auto i = 0; i < 4; i++) {
                if (src.p->e.at(i) == dd::mEdge::one) {
                    tempVec.emplace_back((i == 1 || i == 3));
                    sigVec.emplace_back(tempVec);
                    tempVec.pop_back();
                }
            }
            return;
        }

        if (dd::mNode::isTerminal(src.p)) {
            return;
        }

        for (auto i = 0; i < static_cast<int>(src.p->e.size()); i++) {
            tempVec.emplace_back((i == 1 || i == 3));
            pathSignature(src.p->e.at(i), sigVec, tempVec);
            tempVec.pop_back();
        }
    }

    //This function stores all the ctrls of the node pointed by current not concerning root/src of the dd.
    auto DDSynthesizer::controlNonRoot(dd::mEdge const& current, dd::Controls& ctrl, TruthTable::Cube ctrlCube) -> void {
        for (std::size_t i = 0; i < ctrlCube.size(); i++) {
            if (ctrlCube[i].has_value() && *(ctrlCube[i])) {
                ctrl.emplace(dd::Control{static_cast<dd::Qubit>(static_cast<std::size_t>(current.p->v) - (i + 1)), dd::Control::Type::pos});
            } else if (ctrlCube[i].has_value() && !*(ctrlCube[i])) {
                ctrl.emplace(dd::Control{static_cast<dd::Qubit>(static_cast<std::size_t>(current.p->v) - (i + 1)), dd::Control::Type::neg});
            }
        }
    }

    //This function stores all the ctrls of the node pointed by current concerning the root/src of the dd.
    auto DDSynthesizer::controlRoot(dd::mEdge const& current, dd::Controls& ctrl, TruthTable::Cube ctrlCube) -> void {
        for (std::size_t j = 0; j < ctrlCube.size(); j++) {
            if (ctrlCube[j].has_value() && *(ctrlCube[j])) {
                ctrl.emplace(dd::Control{static_cast<dd::Qubit>((ctrlCube.size() + static_cast<std::size_t>(current.p->v)) - j), dd::Control::Type::pos});
            } else if (ctrlCube[j].has_value() && !*(ctrlCube[j])) {
                ctrl.emplace(dd::Control{static_cast<dd::Qubit>((ctrlCube.size() + static_cast<std::size_t>(current.p->v)) - j), dd::Control::Type::neg});
            }
        }
    }

    //This function performs the multi-control (if any) X operation. First, the dd is modified accordingly. Later on, the qc is emplaced with the same operation.
    auto DDSynthesizer::operation(dd::Qubit const& totalBits, dd::Qubit const& targetBit, dd::mEdge& modifySrc, dd::Controls const& ctrl, std::unique_ptr<dd::Package<>>& dd) -> void {
        auto op       = qc::StandardOperation(totalBits, ctrl, targetBit, qc::X);
        auto srcSaved = modifySrc;
        modifySrc     = dd->multiply(modifySrc, dd::getDD(&op, dd));

        //The below debug statements should always return "circuit is going as planned in operation function". However, for the "urf3" benchmark, it returns "circuit is not going as planned".
        /*if (srcSaved == dd->multiply(modifySrc , dd::getDD(&op, dd))){
                std::cout << "circuit is going as planned operation function"<< std::endl;
        }*/

        dd->incRef(modifySrc);
        dd->decRef(srcSaved);
        dd->garbageCollect();

        qc.x(targetBit, ctrl);
        numGates += 1;
    }

    //This algorithm swaps the paths present in the p' edge to n edge and vice versa (refer to P1 algorithm of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf)
    auto DDSynthesizer::swapPaths(dd::mEdge const& src, dd::mEdge const& current, TruthTable::Cube::Vector const& p1SigVec, TruthTable::Cube::Vector const& p2SigVec, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge {
        if ((current.p->e[0].isZeroTerminal() && !current.p->e[1].isZeroTerminal()) || (p2SigVec.size() > p1SigVec.size())) {
            auto swapPathsSrc = src;

            TruthTable::Cube::Vector rootSigVec;
            TruthTable::Cube         rootTempVec;

            pathFromSrcDst(src, current, rootSigVec, rootTempVec);

            for (auto const& rootVec: rootSigVec) {
                dd::Controls ctrlFinal;

                controlRoot(current, ctrlFinal, rootVec);
                operation(static_cast<dd::Qubit>(src.p->v + 1U), current.p->v, swapPathsSrc, ctrlFinal, dd);
            }

            return swapPathsSrc;
        }

        return src;
    }

    //This algorithm moves the unique paths present in the p' edge to n edge (refer to P2 algorithm of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf)
    auto DDSynthesizer::shiftUniquePaths(dd::mEdge const& src, dd::mEdge const& current, TruthTable::Cube::Vector const& p1SigVec, TruthTable::Cube::Vector const& p2SigVec, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge {
        TruthTable::Cube::Vector uniqueCubeVec;

        //Collect all the unique p' paths.
        for (const auto& p2Cube: p2SigVec) {
            auto it = std::find(p1SigVec.begin(), p1SigVec.end(), p2Cube);
            if (it == p1SigVec.end()) {
                uniqueCubeVec.emplace_back(p2Cube);
            }
        }

        if (uniqueCubeVec.empty()) {
            return src;
        }

        auto shiftUniquePathsSrc = src;

        TruthTable::Cube::Vector rootSigVec;
        TruthTable::Cube         rootTempVec;

        pathFromSrcDst(src, current, rootSigVec, rootTempVec);

        for (auto const& uniCube: uniqueCubeVec) {
            dd::Controls ctrlNonRoot;

            controlNonRoot(current, ctrlNonRoot, uniCube);

            for (auto const& rootVec: rootSigVec) {
                dd::Controls ctrlFinal;

                controlRoot(current, ctrlFinal, rootVec);

                ctrlFinal.insert(ctrlNonRoot.begin(), ctrlNonRoot.end());

                operation(static_cast<dd::Qubit>(src.p->v + 1U), current.p->v, shiftUniquePathsSrc, ctrlFinal, dd);
            }
        }
        return shiftUniquePathsSrc;
    }

    //This algorithm checks whether the p' edge is pointing to zero terminal node (refer to P3 algorithm of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf)
    auto DDSynthesizer::terminate(dd::mEdge const& current) -> bool {
        if (!(dd::mNode::isTerminal(current.p))) {
            return current.p->e[1].isZeroTerminal();
        }
        return false;
    }

    //This algorithm modifies the non-unique paths present in the p' edge to unique paths (refer to P4 algorithm of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf)
    auto DDSynthesizer::unifyPath(dd::mEdge const& src, dd::mEdge const& current, TruthTable::Cube::Vector const& p1SigVec, TruthTable::Cube::Vector const& p2SigVec, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge {
        std::vector<std::size_t> indices;

        for (std::size_t index = 0; index < p2SigVec.size(); index++) {
            auto it = std::find(p1SigVec.begin(), p1SigVec.end(), p2SigVec[index]);
            if (it != p1SigVec.end()) {
                indices.emplace_back(index);
            }
        }

        // return one of the missing cubes.
        TruthTable::Cube missCube{TruthTable::Cube::findMissingCube(p1SigVec)};

        TruthTable::Cube ctrlVec;
        TruthTable::Cube targetVec;

        ctrlVec.resize(p2SigVec[indices[0]].size());
        targetVec.resize(p2SigVec[indices[0]].size());

        // accordingly store the controls and targets.
        for (std::size_t p2Obj = 0; p2Obj < p2SigVec[indices[0]].size(); p2Obj++) {
            if (p2SigVec[indices[0]][p2Obj] == missCube[p2Obj]) {
                ctrlVec[p2Obj] = missCube[p2Obj];
            } else {
                targetVec[p2Obj] = true;
            }
        }

        dd::Controls ctrlNonRoot;

        controlNonRoot(current, ctrlNonRoot, ctrlVec);

        auto unifyPathSrc = src;

        TruthTable::Cube::Vector rootSigVec;
        TruthTable::Cube         rootTempVec;

        pathFromSrcDst(src, current, rootSigVec, rootTempVec);

        for (auto const& rootVec: rootSigVec) {
            dd::Controls ctrlFinal;

            controlRoot(current, ctrlFinal, rootVec);

            ctrlFinal.emplace(dd::Control{current.p->v, dd::Control::Type::pos});

            ctrlFinal.insert(ctrlNonRoot.begin(), ctrlNonRoot.end());

            for (std::size_t i = 0; i < targetVec.size(); i++) {
                if (targetVec[i].has_value() && *(targetVec[i])) {
                    operation(static_cast<dd::Qubit>(src.p->v + 1U), static_cast<dd::Qubit>(static_cast<std::size_t>(current.p->v) - (i + 1U)), unifyPathSrc, ctrlFinal, dd);
                }
            }
        }

        return unifyPathSrc;
    }

    //This algorithm ensures that the node pointed by current is an identity structure (refer to algorithm P of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf).
    auto DDSynthesizer::shiftingPaths(dd::mEdge const& src, dd::mEdge const& current, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge {
        dd::mEdge shiftUniquePathsSrc;

        if (dd::mNode::isTerminal(current.p)) {
            return src;
        }

        TruthTable::Cube::Vector p1SigVec;
        TruthTable::Cube         p1TempVec;

        TruthTable::Cube::Vector p2SigVec;
        TruthTable::Cube         p2TempVec;

        pathSignature(current.p->e[0], p1SigVec, p1TempVec);
        pathSignature(current.p->e[1], p2SigVec, p2TempVec);

        // P1 algorithm.
        if (auto swapPathsSrc = swapPaths(src, current, p1SigVec, p2SigVec, dd); swapPathsSrc == src) {
            // P2 algorithm.
            shiftUniquePathsSrc = shiftUniquePaths(src, current, p1SigVec, p2SigVec, dd);

        } else {
            return swapPathsSrc;
        }

        if (shiftUniquePathsSrc == src) {
            // P3 algorithm.
            if (terminate(current)) {
                return src;
            }

            // P4 algorithm.
            return unifyPath(src, current, p1SigVec, p2SigVec, dd);
        }
        return shiftUniquePathsSrc;
    }

    //This algorithm ensures that the whole dd node is an identity structure (refer to algorithm Q of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf).
    auto DDSynthesizer::synthesizeRec(dd::mEdge const& src, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge {
        auto start = std::chrono::steady_clock::now();

        if (src.p->ref == 0U) {
            dd->incRef(src);
        }

        std::queue<dd::mEdge>         q;
        std::unordered_set<dd::mEdge> visited;

        if (src.p->isIdentity()) {
            time = static_cast<double>((std::chrono::steady_clock::now() - start).count());
            return src;
        }

        q.emplace(src);
        visited.emplace(src);

        //BFS traversal.

        while (!q.empty()) {
            auto newEdge = q.front();
            q.pop();

            //The below debug statements should always return "circuit is going as planned after identity". However, for the "urf3" benchmark, it returns "circuit is not going as planned after identity".
            if (auto shiftingPathsSrc = shiftingPaths(src, newEdge, dd); shiftingPathsSrc != src) {
                if (shiftingPathsSrc.p->isIdentity()) {
                    /*if (srcGlobal == dd->multiply(shiftingPathsSrc , dd::buildFunctionality(&qc, dd))){
                        std::cout << "circuit is going as planned after identity"<< std::endl;
                    }
                    else{
                        std::cout << "circuit is not going as planned after identity"<< std::endl;
                    }*/
                    time = static_cast<double>((std::chrono::steady_clock::now() - start).count());
                    return shiftingPathsSrc;
                }

                //The below debug statements should always return "circuit is going as planned". However, for the "urf3" benchmark, it returns "circuit is not going as planned".
                /*if (srcGlobal == dd->multiply(shiftingPathsSrc , dd::buildFunctionality(&qc, dd))){
                    std::cout << "circuit is going as planned"<< std::endl;
                }
                else{
                    std::cout << "circuit is not going as planned"<< std::endl;
                }*/

                return synthesizeRec(shiftingPathsSrc, dd);
            }

            for (auto edge = 0U; edge < 4U; edge++) {
                if (newEdge.p->e.at(edge).isTerminal()) {
                    continue;
                }

                auto it = visited.find(newEdge.p->e.at(edge));

                if (it == visited.end()) {
                    q.emplace(newEdge.p->e.at(edge));
                    visited.emplace(newEdge.p->e.at(edge));
                }
            }
        }

        time = static_cast<double>((std::chrono::steady_clock::now() - start).count());
        return src;
    }

    //This function returns the operations required to synthesize the dd.
    auto DDSynthesizer::synthesize(dd::mEdge const& src, std::unique_ptr<dd::Package<>>& dd) -> qc::QuantumComputation& {
        qc.clear();
        time     = 0;
        numGates = 0U;

        srcGlobal = src;

        synthesizeRec(src, dd);
        return qc;
    }

} // namespace syrec
