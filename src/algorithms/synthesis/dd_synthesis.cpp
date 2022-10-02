#include "algorithms/synthesis/dd_synthesis.hpp"

#include "dd/FunctionalityConstruction.hpp"

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

    //This algorithm provides the all the paths with their signatures for node pointed by src to node pointed by current (refer to control path section of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf)
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

    //This algorithm provides the all the paths with their signatures for node pointed by src (refer to signature path section of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf)
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

    //This algorithm modifies the non-unique paths present in the p' edge to unique paths (refer to P4 algorithm of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf)
    auto DDSynthesizer::unifyPath(dd::mEdge const& src, dd::mEdge const& current, TruthTable::Cube::Vector const& p1SigVec, TruthTable::Cube::Vector const& p2SigVec, const std::vector<std::size_t>& indices, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge {
        auto unifyPathSrc = src;

        auto p2SigVecCpy = p2SigVec;

        TruthTable::Cube::Vector rootSigVec;
        TruthTable::Cube         rootTempVec;

        pathFromSrcDst(src, current, rootSigVec, rootTempVec);

        auto currentQubit = current.p->v;
        auto sourceQubit  = src.p->v;

        dd::Control currentNode{currentQubit, dd::Control::Type::pos};

        TruthTable::Cube missCube{TruthTable::Cube::findMissingCube(p1SigVec)};
        TruthTable::Cube ctrl;
        dd::Controls     ctrlFirst;
        ctrl.resize(p2SigVecCpy[indices[0]].size());
        TruthTable::Cube target;
        target.resize(p2SigVecCpy[indices[0]].size());

        for (std::size_t p2Obj = 0; p2Obj < p2SigVecCpy[indices[0]].size(); p2Obj++) {
            if (p2SigVecCpy[indices[0]][p2Obj] == missCube[p2Obj]) {
                ctrl[p2Obj] = missCube[p2Obj];
            } else {
                target[p2Obj] = true;
            }
        }

        for (std::size_t p2ObjCpy = 0; p2ObjCpy < ctrl.size(); p2ObjCpy++) {
            if (ctrl[p2ObjCpy].has_value()) {
                if (*(ctrl[p2ObjCpy])) {
                    ctrlFirst.emplace(dd::Control{static_cast<dd::Qubit>(static_cast<std::size_t>(currentQubit) - (p2ObjCpy + 1)), dd::Control::Type::pos});
                } else {
                    ctrlFirst.emplace(dd::Control{static_cast<dd::Qubit>(static_cast<std::size_t>(currentQubit) - (p2ObjCpy + 1)), dd::Control::Type::neg});
                }
            }
        }

        if (!(rootSigVec.empty())) {
            for (auto const& i: rootSigVec) {
                dd::Controls ctrlFinal;
                for (std::size_t j = 0; j < i.size(); j++) {
                    if (i[j].has_value()) {
                        if (*(i[j])) {
                            ctrlFinal.emplace(dd::Control{static_cast<dd::Qubit>((i.size() + static_cast<std::size_t>(currentQubit)) - j), dd::Control::Type::pos});
                        } else {
                            ctrlFinal.emplace(dd::Control{static_cast<dd::Qubit>((i.size() + static_cast<std::size_t>(currentQubit)) - j), dd::Control::Type::neg});
                        }
                    }
                }

                ctrlFinal.emplace(currentNode);
                ctrlFinal.insert(ctrlFirst.begin(), ctrlFirst.end());

                for (std::size_t p2Obj = 0; p2Obj < target.size(); p2Obj++) {
                    if (target[p2Obj].has_value() && *(target[p2Obj])) {
                        auto op       = qc::StandardOperation(sourceQubit + 1U, ctrlFinal, static_cast<dd::Qubit>(static_cast<std::size_t>(currentQubit) - (p2Obj + 1U)), qc::X);
                        auto srcSaved = unifyPathSrc;
                        unifyPathSrc  = dd->multiply(unifyPathSrc, dd::getDD(&op, dd));

                        if (unifyPathSrc.p->ref == 0U) {
                            dd->incRef(unifyPathSrc);
                            dd->decRef(srcSaved);
                            dd->garbageCollect();
                        }

                        qc.x(static_cast<dd::Qubit>(static_cast<std::size_t>(currentQubit) - (p2Obj + 1U)), ctrlFinal);
                        numGates += 1;
                    }
                }

                //return unifyPathSrc;
            }
        }
        return unifyPathSrc;
    }

    //This algorithm checks whether the p' edge is pointing to zero terminal node (refer to P3 algorithm of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf)
    auto DDSynthesizer::terminate(dd::mEdge const& current) -> bool {
        if (!(dd::mNode::isTerminal(current.p))) {
            return current.p->e[1].isZeroTerminal();
        }
        return false;
    }

    //This algorithm swaps the paths present in the p' edge to n edge and vice versa (refer to P1 algorithm of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf)
    auto DDSynthesizer::swapPaths(dd::mEdge const& src, dd::mEdge const& current, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge {
        if (dd::mNode::isTerminal(current.p)) {
            return src;
        }

        auto swapPathsSrc = src;

        TruthTable::Cube::Vector p1SigVec;
        TruthTable::Cube         p1TempVec;

        TruthTable::Cube::Vector p2SigVec;
        TruthTable::Cube         p2TempVec;

        pathSignature(current.p->e[0], p1SigVec, p1TempVec);
        pathSignature(current.p->e[1], p2SigVec, p2TempVec);

        auto currentQubit = current.p->v;
        auto sourceQubit  = src.p->v;

        if ((current.p->e[0].isZeroTerminal() && !current.p->e[1].isZeroTerminal()) || (p2SigVec.size() > p1SigVec.size())) {
            TruthTable::Cube::Vector rootSigVec;
            TruthTable::Cube         rootTempVec;

            pathFromSrcDst(src, current, rootSigVec, rootTempVec);

            for (auto const& i: rootSigVec) {
                dd::Controls ctrl;
                for (std::size_t j = 0; j < i.size(); j++) {
                    if (i[j].has_value()) {
                        if (*(i[j])) {
                            ctrl.emplace(dd::Control{static_cast<dd::Qubit>((i.size() + static_cast<std::size_t>(currentQubit)) - j), dd::Control::Type::pos});
                        } else {
                            ctrl.emplace(dd::Control{static_cast<dd::Qubit>((i.size() + static_cast<std::size_t>(currentQubit)) - j), dd::Control::Type::neg});
                        }
                    }
                }

                auto op       = qc::StandardOperation(sourceQubit + 1U, ctrl, currentQubit, qc::X);
                auto srcSaved = swapPathsSrc;
                swapPathsSrc  = dd->multiply(swapPathsSrc, dd::getDD(&op, dd));

                if (swapPathsSrc.p->ref == 0U) {
                    dd->incRef(swapPathsSrc);
                    dd->decRef(srcSaved);
                    dd->garbageCollect();
                }

                qc.x(currentQubit, ctrl);
                numGates += 1;

                //return swapPathsSrc;
            }
        }

        return swapPathsSrc;
    }

    //This algorithm moves the unique paths present in the p' edge to n edge (refer to P2 algorithm of http://www.informatik.uni-bremen.de/agra/doc/konf/12aspdac_qmdd_synth_rev.pdf)
    auto DDSynthesizer::shiftUniquePaths(dd::mEdge const& src, dd::mEdge const& current, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge {
        if (dd::mNode::isTerminal(current.p)) {
            return src;
        }

        auto shiftUniquePathsSrc = src;

        TruthTable::Cube::Vector p1SigVec;
        TruthTable::Cube         p1TempVec;

        TruthTable::Cube::Vector p2SigVec;
        TruthTable::Cube         p2TempVec;

        TruthTable::Cube::Vector rootSigVec;
        TruthTable::Cube         rootTempVec;

        TruthTable::Cube::Vector uniqueCubeVec;

        std::vector<std::size_t> indices;

        pathSignature(current.p->e[0], p1SigVec, p1TempVec);
        pathSignature(current.p->e[1], p2SigVec, p2TempVec);

        auto currentQubit = current.p->v;
        auto sourceQubit  = src.p->v;

        pathFromSrcDst(src, current, rootSigVec, rootTempVec);

        for (auto& p2Cube: p2SigVec) {
            auto it = std::find(p1SigVec.begin(), p1SigVec.end(), p2Cube);
            if (it == p1SigVec.end()) {
                uniqueCubeVec.emplace_back(p2Cube);
            }
        }

        if (uniqueCubeVec.empty()) {
            return shiftUniquePathsSrc;
        }

        for (auto& uniCube: uniqueCubeVec) {
            dd::Controls ctrl;

            for (std::size_t pj = 0; pj < uniCube.size(); pj++) {
                if (uniCube[pj].has_value()) {
                    if (*(uniCube[pj])) {
                        ctrl.emplace(dd::Control{static_cast<dd::Qubit>(static_cast<std::size_t>(currentQubit) - (pj + 1)), dd::Control::Type::pos});
                    } else {
                        ctrl.emplace(dd::Control{static_cast<dd::Qubit>(static_cast<std::size_t>(currentQubit) - (pj + 1)), dd::Control::Type::neg});
                    }
                }
            }

            for (auto const& i: rootSigVec) {
                dd::Controls ctrlFinal;
                for (std::size_t j = 0; j < i.size(); j++) {
                    if (i[j].has_value()) {
                        if (*(i[j])) {
                            ctrlFinal.emplace(dd::Control{static_cast<dd::Qubit>((i.size() + static_cast<std::size_t>(currentQubit)) - j), dd::Control::Type::pos});
                        } else {
                            ctrlFinal.emplace(dd::Control{static_cast<dd::Qubit>((i.size() + static_cast<std::size_t>(currentQubit)) - j), dd::Control::Type::neg});
                        }
                    }
                }

                ctrlFinal.insert(ctrl.begin(), ctrl.end());

                auto op             = qc::StandardOperation(sourceQubit + 1U, ctrlFinal, currentQubit, qc::X);
                auto srcSaved       = shiftUniquePathsSrc;
                shiftUniquePathsSrc = dd->multiply(shiftUniquePathsSrc, dd::getDD(&op, dd));
                if (shiftUniquePathsSrc.p->ref == 0U) {
                    dd->incRef(shiftUniquePathsSrc);
                    dd->decRef(srcSaved);
                    dd->garbageCollect();
                }
                qc.x(currentQubit, ctrlFinal);
                numGates += 1;

                //return shiftUniquePathsSrc;
            }
        }
        return shiftUniquePathsSrc;
    }

    auto DDSynthesizer::shiftingPaths(dd::mEdge const& src, dd::mEdge const& current, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge {
        dd::mEdge swapPathsSrc;
        dd::mEdge shiftUniquePathsSrc;

        swapPathsSrc = swapPaths(src, current, dd);

        if (swapPathsSrc == src) {
            shiftUniquePathsSrc = shiftUniquePaths(src, current, dd);

        } else {
            return swapPathsSrc;
        }

        if (shiftUniquePathsSrc == src) {
            if (terminate(current)) {
                return src;
            }

            TruthTable::Cube::Vector p1SigVec;
            TruthTable::Cube         p1TempVec;

            TruthTable::Cube::Vector p2SigVec;
            TruthTable::Cube         p2TempVec;

            std::vector<std::size_t> indices;

            pathSignature(current.p->e[0], p1SigVec, p1TempVec);
            pathSignature(current.p->e[1], p2SigVec, p2TempVec);

            for (std::size_t index = 0; index < p2SigVec.size(); index++) {
                auto it = std::find(p1SigVec.begin(), p1SigVec.end(), p2SigVec[index]);
                if (it != p1SigVec.end()) {
                    indices.emplace_back(index);
                }
            }

            return unifyPath(src, current, p1SigVec, p2SigVec, indices, dd);
        }
        return shiftUniquePathsSrc;
    }

    auto DDSynthesizer::synthesizeRec(dd::mEdge const& src, std::unique_ptr<dd::Package<>>& dd) -> dd::mEdge {
        auto start = std::chrono::steady_clock::now();

        qcSingleOp.clear();

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

        while (!q.empty()) {
            auto newEdge = q.front();
            q.pop();

            auto shiftingPathsSrc = shiftingPaths(src, newEdge, dd);

            if (shiftingPathsSrc != src) {
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

    auto DDSynthesizer::synthesize(dd::mEdge const& src, std::unique_ptr<dd::Package<>>& dd) -> qc::QuantumComputation& {
        qc.clear();
        qcSingleOp.clear();
        time     = 0;
        numGates = 0U;

        srcGlobal = src;

        synthesizeRec(src, dd);
        return qc;
    }

} // namespace syrec
