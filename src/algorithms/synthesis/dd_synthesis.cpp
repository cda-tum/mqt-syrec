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

    auto DDSynthesis::pathFromRoot(dd::mNode* src, dd::mNode* dst, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& tempVec) -> void {
        if (bool bitMatch = (src->v > dst->v); !bitMatch) {
            if (src == dst) {
                sigVec.push_back(tempVec);
                return;
            }
            return;
        }

        auto* n = src->e[0].p;
        tempVec.emplace_back(false);
        pathFromRoot(n, dst, sigVec, tempVec);
        tempVec.pop_back();

        auto* p = src->e[3].p;
        tempVec.emplace_back(true);
        pathFromRoot(p, dst, sigVec, tempVec);
        tempVec.pop_back();
        //return
    }

    auto DDSynthesis::pathSignature(dd::mNode* src, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& tempVec) -> void {
        if (src->v == 0) {
            if (src->e[0] == dd::mEdge::one) {
                tempVec.emplace_back(false);
                sigVec.push_back(tempVec);
                tempVec.pop_back();
            }
            if (src->e[1] == dd::mEdge::one) {
                tempVec.emplace_back(true);
                sigVec.push_back(tempVec);
                tempVec.pop_back();
            }
            if (src->e[2] == dd::mEdge::one) {
                tempVec.emplace_back(false);
                sigVec.push_back(tempVec);
                tempVec.pop_back();
            }
            if (src->e[3] == dd::mEdge::one) {
                tempVec.emplace_back(true);
                sigVec.push_back(tempVec);
                tempVec.pop_back();
            }

            return;
        }

        if (dd::mNode::isTerminal(src)) {
            return;
        }

        auto* p1 = src->e[0].p;
        tempVec.emplace_back(false);
        pathSignature(p1, sigVec, tempVec);
        tempVec.pop_back();

        auto* p2 = src->e[1].p;
        tempVec.emplace_back(true);
        pathSignature(p2, sigVec, tempVec);
        tempVec.pop_back();

        auto* p3 = src->e[2].p;
        tempVec.emplace_back(false);
        pathSignature(p3, sigVec, tempVec);
        tempVec.pop_back();

        auto* p4 = src->e[3].p;
        tempVec.emplace_back(true);
        pathSignature(p4, sigVec, tempVec);
        tempVec.pop_back();
        //return
    }

    auto DDSynthesis::unifyPath(dd::mEdge& src, dd::mNode* current, syrec::TruthTable::Cube::Vector& p1SigVec, syrec::TruthTable::Cube::Vector& p2SigVec, const std::vector<std::size_t>& indices, std::unique_ptr<dd::Package<>>& dd) -> void {
        syrec::TruthTable::Cube::Vector p2SigVecCpy = p2SigVec;

        syrec::TruthTable::Cube::Vector rootSigVec;
        syrec::TruthTable::Cube         rootTempVec;

        pathFromRoot(src.p, current, rootSigVec, rootTempVec);

        dd::Controls ctrlroot;

        for (auto const& i: rootSigVec) {
            for (std::size_t j = 0; j < i.size(); j++) {
                if (i[j].has_value()) {
                    if (*(i[j])) {
                        dd::Control obj;
                        obj.qubit = static_cast<dd::Qubit>((i.size() + current->v) - j);
                        obj.type  = dd::Control::Type::pos;
                        ctrlroot.insert(obj);
                    } else {
                        dd::Control obj;
                        obj.qubit = static_cast<dd::Qubit>((i.size() + current->v) - j);
                        obj.type  = dd::Control::Type::neg;
                        ctrlroot.insert(obj);
                    }
                }
            }
        }

        dd::Control currentNode;
        currentNode.qubit = current->v;
        currentNode.type  = dd::Control::Type::pos;

        for (auto ind: indices) {
            for (std::size_t p2Obj = 0; p2Obj < p2SigVecCpy[ind].size(); p2Obj++) {
                dd::Controls ctrl;
                if (*(p2SigVecCpy[ind][p2Obj])) {
                    p2SigVecCpy[ind][p2Obj] = false;
                } else {
                    p2SigVecCpy[ind][p2Obj] = true;
                }

                auto it1 = std::find(p1SigVec.begin(), p1SigVec.end(), p2SigVecCpy[ind]);
                auto it2 = std::find(p2SigVec.begin(), p2SigVec.end(), p2SigVecCpy[ind]);

                if (it1 != p1SigVec.end() && it2 != p2SigVec.end()) {
                    p2SigVecCpy = p2SigVec;
                    continue;
                }
                for (std::size_t p2ObjCpy = 0; p2ObjCpy < p2SigVec[ind].size(); p2ObjCpy++) {
                    if (p2ObjCpy != p2Obj) {
                        if (*(p2SigVec[ind][p2ObjCpy])) {
                            dd::Control objP;
                            objP.qubit = static_cast<dd::Qubit>(current->v - (p2ObjCpy + 1));
                            objP.type  = dd::Control::Type::pos;
                            ctrl.insert(objP);
                        } else {
                            dd::Control objN;
                            objN.qubit = static_cast<dd::Qubit>(current->v - (p2ObjCpy + 1));
                            objN.type  = dd::Control::Type::neg;
                            ctrl.insert(objN);
                        }
                    }
                }

                if (!ctrlroot.empty()) {
                    for (auto i: ctrlroot) {
                        dd::Controls ctrlFinal;
                        ctrlFinal.insert(i);
                        ctrlFinal.insert(currentNode);
                        ctrlFinal.insert(ctrl.begin(), ctrl.end());

                        auto q = qc::QuantumComputation((src.p->v) + 1U);
                        q.x(static_cast<dd::Qubit>(static_cast<std::size_t>(current->v) - (p2Obj + 1U)), ctrlFinal);
                        qc.x(static_cast<dd::Qubit>(static_cast<std::size_t>(current->v) - (p2Obj + 1U)), ctrlFinal);
                        ctrlFinal.clear();
                        auto ddTest = dd::buildFunctionality(&q, dd);
                        src         = dd->multiply(src, ddTest);
                    }
                } else {
                    dd::Controls ctrlFinal;
                    ctrlFinal.insert(currentNode);
                    ctrlFinal.insert(ctrl.begin(), ctrl.end());

                    auto q = qc::QuantumComputation((src.p->v) + 1U);
                    q.x(static_cast<dd::Qubit>(static_cast<std::size_t>(current->v) - (p2Obj + 1U)), ctrlFinal);
                    qc.x(static_cast<dd::Qubit>(static_cast<std::size_t>(current->v) - (p2Obj + 1U)), ctrlFinal);
                    ctrlFinal.clear();
                    auto ddTest = dd::buildFunctionality(&q, dd);
                    src         = dd->multiply(src, ddTest);
                }

                break;
            }
        }
        synthesize(src, dd);
    }

    auto DDSynthesis::terminate(dd::mNode* current) -> bool {
        if (!(dd::mNode::isTerminal(current))) {
            return !(current->e[0].isZeroTerminal()) && current->e[1].isZeroTerminal();
        }

        return false;
    }

    auto DDSynthesis::swapPaths(dd::mEdge& src, dd::mNode* current, std::unique_ptr<dd::Package<>>& dd) -> void {
        if (!(dd::mNode::isTerminal(current))) {
            syrec::TruthTable::Cube::Vector p1SigVec;
            syrec::TruthTable::Cube         p1TempVec;

            syrec::TruthTable::Cube::Vector p2SigVec;
            syrec::TruthTable::Cube         p2TempVec;

            pathSignature(current->e[0].p, p1SigVec, p1TempVec);
            pathSignature(current->e[1].p, p2SigVec, p2TempVec);

            if ((current->e[0].isZeroTerminal() && !current->e[1].isZeroTerminal()) || (p2SigVec.size() > p1SigVec.size())) {
                syrec::TruthTable::Cube::Vector rootSigVec;
                syrec::TruthTable::Cube         rootTempVec;

                pathFromRoot(src.p, current, rootSigVec, rootTempVec);

                dd::Controls ctrl;

                for (auto const& i: rootSigVec) {
                    for (std::size_t j = 0; j < i.size(); j++) {
                        if (*(i[j])) {
                            dd::Control obj;
                            obj.qubit = static_cast<dd::Qubit>((i.size() + current->v) - j);
                            obj.type  = dd::Control::Type::pos;
                            ctrl.insert(obj);
                        } else {
                            dd::Control obj;
                            obj.qubit = static_cast<dd::Qubit>((i.size() + current->v) - j);
                            obj.type  = dd::Control::Type::neg;
                            ctrl.insert(obj);
                        }
                    }

                    auto q = qc::QuantumComputation((src.p->v) + 1U);
                    q.x(current->v, ctrl);
                    qc.x(current->v, ctrl);
                    ctrl.clear();
                    auto ddTest = dd::buildFunctionality(&q, dd);
                    src         = dd->multiply(src, ddTest);
                }

                synthesize(src, dd);
            }
        }
    }

    auto DDSynthesis::shiftUniquePaths(dd::mEdge& src, dd::mNode* current, std::unique_ptr<dd::Package<>>& dd) -> void {
        if (!(dd::mNode::isTerminal(current))) {
            syrec::TruthTable::Cube::Vector p1SigVec;
            syrec::TruthTable::Cube         p1TempVec;

            syrec::TruthTable::Cube::Vector p2SigVec;
            syrec::TruthTable::Cube         p2TempVec;

            std::vector<std::size_t> indices;

            pathSignature(current->e[0].p, p1SigVec, p1TempVec);
            pathSignature(current->e[1].p, p2SigVec, p2TempVec);

            std::size_t flag = 0;

            for (std::size_t i = 0; i < p2SigVec.size(); i++) {
                auto it = std::find(p1SigVec.begin(), p1SigVec.end(), p2SigVec[i]);
                if (it != p1SigVec.end()) {
                    indices.push_back(i);
                } else {
                    flag = flag + 1;
                }
            }

            if (flag == p2SigVec.size() && flag != 0) {
                syrec::TruthTable::Cube::Vector rootSigVec;
                syrec::TruthTable::Cube         rootTempVec;

                pathFromRoot(src.p, current, rootSigVec, rootTempVec);

                dd::Controls ctrl;

                for (auto const& i: rootSigVec) {
                    for (std::size_t j = 0; j < i.size(); j++) {
                        if (*(i[j])) {
                            dd::Control obj;
                            obj.qubit = static_cast<dd::Qubit>((i.size() + current->v) - j);
                            obj.type  = dd::Control::Type::pos;
                            ctrl.insert(obj);
                        } else {
                            dd::Control obj;
                            obj.qubit = static_cast<dd::Qubit>((i.size() + current->v) - j);
                            obj.type  = dd::Control::Type::neg;
                            ctrl.insert(obj);
                        }
                    }

                    for (auto const& p2: p2SigVec) {
                        for (std::size_t pj = 0; pj < p2.size(); pj++) {
                            if (*(p2[pj])) {
                                dd::Control obj;
                                obj.qubit = static_cast<dd::Qubit>(current->v - (pj + 1));
                                obj.type  = dd::Control::Type::pos;
                                ctrl.insert(obj);
                            } else {
                                dd::Control obj;
                                obj.qubit = static_cast<dd::Qubit>(current->v - (pj + 1));
                                obj.type  = dd::Control::Type::neg;
                                ctrl.insert(obj);
                            }
                        }

                        auto q = qc::QuantumComputation((src.p->v) + 1U);
                        q.x(current->v, ctrl);
                        qc.x(current->v, ctrl);
                        ctrl.clear();
                        auto ddTest = dd::buildFunctionality(&q, dd);
                        src         = dd->multiply(src, ddTest);
                    }
                }

                synthesize(src, dd);

            }

            else {
                unifyPath(src, current, p1SigVec, p2SigVec, indices, dd);
            }
        }
    }

    auto DDSynthesis::shiftingPaths(dd::mEdge& src, dd::mNode* current, std::unique_ptr<dd::Package<>>& dd) -> void {
        auto srcCpy = src;

        if (terminate(current)) {
            return;
        }

        swapPaths(src, current, dd);

        if (srcCpy == src) {
            shiftUniquePaths(src, current, dd);
        }
    }

    auto DDSynthesis::synthesize(dd::mEdge& src, std::unique_ptr<dd::Package<>>& dd) -> qc::QuantumComputation& {
        std::vector<dd::mNode*> q;
        std::vector<dd::mNode*> visited;

        shiftingPaths(src, src.p, dd);

        auto const* currNode = src.p;

        while (src != dd->makeIdent(static_cast<dd::Qubit>((src.p->v) + 1U))) {
            for (const auto& edge: currNode->e) {
                if (!edge.isTerminal()) {
                    auto it = std::find(visited.begin(), visited.end(), edge.p);

                    if (it == visited.end()) {
                        shiftingPaths(src, edge.p, dd);
                        q.emplace_back(edge.p);
                        visited.emplace_back(edge.p);
                    }
                }
            }
            currNode = q.front();
            q.erase(q.begin());
        }
        return qc;
    }
} // namespace syrec
