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

    auto DDSynthesizer::pathFromSrcDst(dd::mNode* src, dd::mNode const* dst, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& tempVec) const -> void {
        if (src->v <= dst->v) {
            if (src == dst) {
                sigVec.emplace_back(tempVec);
            }
            return;
        }

        tempVec.emplace_back(false);
        pathFromSrcDst(src->e[0].p, dst, sigVec, tempVec);
        tempVec.pop_back();

        tempVec.emplace_back(true);
        pathFromSrcDst(src->e[3].p, dst, sigVec, tempVec);
        tempVec.pop_back();
    }

    auto DDSynthesizer::pathSignature(dd::mNode* src, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& tempVec) -> void {
        if (src->v == 0) {
            if (src->e[0] == dd::mEdge::one || src->e[2] == dd::mEdge::one) {
                tempVec.emplace_back(false);
                sigVec.emplace_back(tempVec);
                tempVec.pop_back();
            }
            if (src->e[1] == dd::mEdge::one || src->e[3] == dd::mEdge::one) {
                tempVec.emplace_back(true);
                sigVec.emplace_back(tempVec);
                tempVec.pop_back();
            }
            return;
        }

        if (dd::mNode::isTerminal(src)) {
            return;
        }

        for (auto i = 0; i < static_cast<int>(src->e.size()); i++) {
            if (i == 0 || i == 2) {
                tempVec.emplace_back(false);
            } else {
                tempVec.emplace_back(true);
            }
            pathSignature(src->e.at(i).p, sigVec, tempVec);
            tempVec.pop_back();
        }
    }

    auto DDSynthesizer::unifyPath(dd::mEdge& src, dd::mEdge const& current, TruthTable::Cube::Vector const& p1SigVec, TruthTable::Cube::Vector const& p2SigVec, const std::vector<std::size_t>& indices, std::unique_ptr<dd::Package<>>& dd) -> bool {
        auto p2SigVecCpy = p2SigVec;

        TruthTable::Cube::Vector rootSigVec;
        TruthTable::Cube         rootTempVec;

        pathFromSrcDst(src.p, current.p, rootSigVec, rootTempVec);

        dd::Control currentNode;
        currentNode.qubit = current.p->v;
        currentNode.type  = dd::Control::Type::pos;

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
                    ctrlFirst.emplace(dd::Control{static_cast<dd::Qubit>(current.p->v - (p2ObjCpy + 1)), dd::Control::Type::pos});
                } else {
                    ctrlFirst.emplace(dd::Control{static_cast<dd::Qubit>(current.p->v - (p2ObjCpy + 1)), dd::Control::Type::neg});
                }
            }
        }

        if (!rootSigVec.empty()) {
            for (auto const& i: rootSigVec) {
                dd::Controls ctrlFinal;
                for (std::size_t j = 0; j < i.size(); j++) {
                    if (i[j].has_value()) {
                        if (*(i[j])) {
                            ctrlFinal.emplace(dd::Control{static_cast<dd::Qubit>((i.size() + current.p->v) - j), dd::Control::Type::pos});
                        } else {
                            ctrlFinal.emplace(dd::Control{static_cast<dd::Qubit>((i.size() + current.p->v) - j), dd::Control::Type::neg});
                        }
                    }
                }

                ctrlFinal.emplace(currentNode);
                ctrlFinal.insert(ctrlFirst.begin(), ctrlFirst.end());

                for (std::size_t p2Obj = 0; p2Obj < target.size(); p2Obj++) {
                    if (target[p2Obj].has_value() && *(target[p2Obj])) {
                        auto op = qc::StandardOperation((src.p->v) + 1U, ctrlFinal, static_cast<dd::Qubit>(static_cast<std::size_t>(current.p->v) - (p2Obj + 1U)), qc::X);
                        qc.x(static_cast<dd::Qubit>(static_cast<std::size_t>(current.p->v) - (p2Obj + 1U)), ctrlFinal);

                        auto srcSaved = src;
                        src           = dd->multiply(src, dd::getDD(&op, dd));
                        dd->incRef(src);
                        dd->decRef(srcSaved);
                        dd->garbageCollect();
                    }
                }
            }

        } else {
            dd::Controls ctrlFinal;
            ctrlFinal.emplace(currentNode);
            ctrlFinal.insert(ctrlFirst.begin(), ctrlFirst.end());

            for (std::size_t p2Obj = 0; p2Obj < target.size(); p2Obj++) {
                if (target[p2Obj].has_value() && *(target[p2Obj])) {
                    auto op = qc::StandardOperation((src.p->v) + 1U, ctrlFinal, static_cast<dd::Qubit>(static_cast<std::size_t>(current.p->v) - (p2Obj + 1U)), qc::X);
                    qc.x(static_cast<dd::Qubit>(static_cast<std::size_t>(current.p->v) - (p2Obj + 1U)), ctrlFinal);

                    auto srcSaved = src;
                    src           = dd->multiply(src, dd::getDD(&op, dd));
                    dd->incRef(src);
                    dd->decRef(srcSaved);
                    dd->garbageCollect();
                }
            }
        }
        synthesize(src, dd);
        return true;
    }

    auto DDSynthesizer::terminate(dd::mEdge const& current) -> bool {
        if (!(dd::mNode::isTerminal(current.p))) {
            return current.p->e[1].isZeroTerminal();
        }
        return false;
    }

    auto DDSynthesizer::swapPaths(dd::mEdge& src, dd::mEdge const& current, std::unique_ptr<dd::Package<>>& dd) -> bool {
        if (dd::mNode::isTerminal(current.p)) {
            return false;
        }
        TruthTable::Cube::Vector p1SigVec;
        TruthTable::Cube         p1TempVec;

        TruthTable::Cube::Vector p2SigVec;
        TruthTable::Cube         p2TempVec;

        pathSignature(current.p->e[0].p, p1SigVec, p1TempVec);
        pathSignature(current.p->e[1].p, p2SigVec, p2TempVec);

        if ((current.p->e[0].isZeroTerminal() && !current.p->e[1].isZeroTerminal()) || (p2SigVec.size() > p1SigVec.size())) {
            TruthTable::Cube::Vector rootSigVec;
            TruthTable::Cube         rootTempVec;

            pathFromSrcDst(src.p, current.p, rootSigVec, rootTempVec);

            dd::Controls ctrl;

            for (auto const& i: rootSigVec) {
                for (std::size_t j = 0; j < i.size(); j++) {
                    if (*(i[j])) {
                        ctrl.emplace(dd::Control{static_cast<dd::Qubit>((i.size() + current.p->v) - j), dd::Control::Type::pos});
                    } else {
                        ctrl.emplace(dd::Control{static_cast<dd::Qubit>((i.size() + current.p->v) - j), dd::Control::Type::neg});
                    }
                }

                auto op = qc::StandardOperation((src.p->v) + 1U, ctrl, current.p->v, qc::X);
                qc.x(current.p->v, ctrl);
                ctrl.clear();

                auto srcSaved = src;
                src           = dd->multiply(src, dd::getDD(&op, dd));
                dd->incRef(src);
                dd->decRef(srcSaved);
                dd->garbageCollect();
            }
            synthesize(src, dd);
            return true;
        }
        return false;
    }

    auto DDSynthesizer::shiftUniquePaths(dd::mEdge& src, dd::mEdge const& current, std::unique_ptr<dd::Package<>>& dd) -> bool {
        if (dd::mNode::isTerminal(current.p)) {
            return false;
        }
        TruthTable::Cube::Vector p1SigVec;
        TruthTable::Cube         p1TempVec;

        TruthTable::Cube::Vector p2SigVec;
        TruthTable::Cube         p2TempVec;

        std::vector<std::size_t> indices;

        pathSignature(current.p->e[0].p, p1SigVec, p1TempVec);
        pathSignature(current.p->e[1].p, p2SigVec, p2TempVec);

        for (std::size_t index = 0; index < p2SigVec.size(); index++) {
            auto it = std::find(p1SigVec.begin(), p1SigVec.end(), p2SigVec[index]);
            if (it != p1SigVec.end()) {
                indices.emplace_back(index);
            } else {
                TruthTable::Cube::Vector rootSigVec;
                TruthTable::Cube         rootTempVec;

                pathFromSrcDst(src.p, current.p, rootSigVec, rootTempVec);

                dd::Controls ctrl;

                for (auto const& i: rootSigVec) {
                    for (std::size_t j = 0; j < i.size(); j++) {
                        if (*(i[j])) {
                            ctrl.emplace(dd::Control{static_cast<dd::Qubit>((i.size() + current.p->v) - j), dd::Control::Type::pos});
                        } else {
                            ctrl.emplace(dd::Control{static_cast<dd::Qubit>((i.size() + current.p->v) - j), dd::Control::Type::neg});
                        }
                    }
                    for (std::size_t pj = 0; pj < p2SigVec[index].size(); pj++) {
                        if (*(p2SigVec[index][pj])) {
                            ctrl.emplace(dd::Control{static_cast<dd::Qubit>(current.p->v - (pj + 1)), dd::Control::Type::pos});
                        } else {
                            ctrl.emplace(dd::Control{static_cast<dd::Qubit>(current.p->v - (pj + 1)), dd::Control::Type::neg});
                        }
                    }
                    auto op = qc::StandardOperation((src.p->v) + 1U, ctrl, current.p->v, qc::X);
                    qc.x(current.p->v, ctrl);
                    ctrl.clear();

                    auto srcSaved = src;
                    src           = dd->multiply(src, dd::getDD(&op, dd));
                    dd->incRef(src);
                    dd->decRef(srcSaved);
                    dd->garbageCollect();
                }

                synthesize(src, dd);
                return true;
            }
        }
        return unifyPath(src, current, p1SigVec, p2SigVec, indices, dd);
    }

    auto DDSynthesizer::shiftingPaths(dd::mEdge& src, dd::mEdge const& current, std::unique_ptr<dd::Package<>>& dd) -> bool {
        return terminate(current) || swapPaths(src, current, dd) || shiftUniquePaths(src, current, dd);
    }

    auto DDSynthesizer::synthesize(dd::mEdge& src, std::unique_ptr<dd::Package<>>& dd) -> void {
        auto                   start = std::chrono::steady_clock::now();
        std::queue<dd::mEdge>  q;
        std::vector<dd::mEdge> visited;

        shiftingPaths(src, src, dd);

        auto currEdge = src;

        while (!src.p->isIdentity()) {
            for (auto& edge: currEdge.p->e) {
                if (edge.isTerminal()) {
                    continue;
                }
                auto it = std::find(visited.begin(), visited.end(), edge);

                if (it == visited.end()) {
                    shiftingPaths(src, edge, dd);
                    q.emplace(edge);
                    visited.emplace_back(edge);
                }
            }
            currEdge = q.front();
            q.pop();
        }

        time = (std::chrono::steady_clock::now() - start);
    }

    auto DDSynthesizer::buildFunctionality(std::unique_ptr<dd::Package<>>& dd) const -> dd::mEdge {
        return dd::buildFunctionality(&qc, dd);
    }

} // namespace syrec
