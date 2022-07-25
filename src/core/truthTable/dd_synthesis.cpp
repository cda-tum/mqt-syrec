#include "core/truthTable/dd_synthesis.hpp"

#include "dd/FunctionalityConstruction.hpp"
using namespace dd::literals;

namespace syrec {

    // as of now the function can be used to synthesize completely reversible truthtables (without don't care conditions).

    void rootPath(dd::mNode* src, dd::mNode* dst, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& tempVec) {
        if (bool bitMatch = (src->v > dst->v); !bitMatch) {
            if (src == dst) {
                sigVec.push_back(tempVec);
                return;
            } else {
                return;
            }
        }

        auto n = src->e[0].p;
        tempVec.emplace_back(false);
        rootPath(n, dst, sigVec, tempVec);
        tempVec.pop_back();

        auto p = src->e[3].p;
        tempVec.emplace_back(true);
        rootPath(p, dst, sigVec, tempVec);
        tempVec.pop_back();

        return;
    }

    void sigPath(dd::mNode* src, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& tempVec) {
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

        auto p1 = src->e[0].p;
        tempVec.emplace_back(false);
        sigPath(p1, sigVec, tempVec);
        tempVec.pop_back();

        auto p2 = src->e[1].p;
        tempVec.emplace_back(true);
        sigPath(p2, sigVec, tempVec);
        tempVec.pop_back();

        auto p3 = src->e[2].p;
        tempVec.emplace_back(false);
        sigPath(p3, sigVec, tempVec);
        tempVec.pop_back();

        auto p4 = src->e[3].p;
        tempVec.emplace_back(true);
        sigPath(p4, sigVec, tempVec);
        tempVec.pop_back();

        return;
    }

    void algoP4(dd::mEdge& src, dd::mNode* current, syrec::TruthTable::Cube::Vector& p1SigVec, syrec::TruthTable::Cube::Vector& p2SigVec, const std::vector<std::size_t>& indices, std::unique_ptr<dd::Package<>>& dd) {
        std::cout << "inside algoP4" << std::endl;
        syrec::TruthTable::Cube::Vector p2SigVecCpy = p2SigVec;

        syrec::TruthTable::Cube::Vector rootSigVec;
        syrec::TruthTable::Cube         rootTempVec;

        rootPath(src.p, current, rootSigVec, rootTempVec);

        dd::Controls Ctrlroot;

        for (auto const& i: rootSigVec) {
            for (std::size_t j = 0; j < i.size(); j++) {
                if (i[j].has_value()) {
                    if (*(i[j])) {
                        std::cout << "root" << std::endl;
                        std::cout << "pos" << std::endl;
                        dd::Control obj;
                        obj.qubit = static_cast<dd::Qubit>((i.size() + current->v) - j);
                        obj.type  = dd::Control::Type::pos;
                        Ctrlroot.insert(obj);
                    } else {
                        std::cout << "root" << std::endl;
                        std::cout << "neg" << std::endl;
                        dd::Control obj;
                        obj.qubit = static_cast<dd::Qubit>((i.size() + current->v) - j);
                        obj.type  = dd::Control::Type::neg;
                        Ctrlroot.insert(obj);
                    }
                }
            }
        }

        dd::Control currentNode;
        currentNode.qubit = current->v;
        currentNode.type  = dd::Control::Type::pos;

        for (auto ind: indices) {
            for (std::size_t p2Obj = 0; p2Obj < p2SigVecCpy[ind].size(); p2Obj++) {
                dd::Controls Ctrl;
                if (*(p2SigVecCpy[ind][p2Obj])) {
                    std::cout << "True -> False" << std::endl;
                    p2SigVecCpy[ind].at(p2Obj) = false;
                } else {
                    std::cout << "False -> True" << std::endl;
                    p2SigVecCpy[ind].at(p2Obj) = true;
                }

                auto it = std::find(p1SigVec.begin(), p1SigVec.end(), p2SigVecCpy[ind]);
                if (it != p1SigVec.end()) {
                    std::cout << "not changed to unique signature" << std::endl;
                    p2SigVecCpy = p2SigVec;
                    continue;
                } else {
                    std::cout << "changed to unique signature" << std::endl;
                    for (std::size_t p2ObjCpy = 0; p2ObjCpy < p2SigVec[ind].size(); p2ObjCpy++) {
                        if (p2ObjCpy != p2Obj) {
                            if (*(p2SigVec[ind][p2ObjCpy])) {
                                std::cout << "not root" << std::endl;
                                std::cout << "pos" << std::endl;
                                dd::Control objP;
                                objP.qubit = static_cast<dd::Qubit>(current->v - (p2ObjCpy + 1));
                                objP.type  = dd::Control::Type::pos;
                                Ctrl.insert(objP);
                            } else {
                                std::cout << "not root" << std::endl;
                                std::cout << "neg" << std::endl;
                                dd::Control objN;
                                objN.qubit = static_cast<dd::Qubit>(current->v - (p2ObjCpy + 1));
                                objN.type  = dd::Control::Type::neg;
                                Ctrl.insert(objN);
                            }
                        }
                    }

                    if (!Ctrlroot.empty()) {
                        for (auto i: Ctrlroot) {
                            dd::Controls ctrlFinal;
                            ctrlFinal.insert(i);
                            ctrlFinal.insert(currentNode);
                            ctrlFinal.insert(Ctrl.begin(), Ctrl.end());

                            auto q = qc::QuantumComputation(dd->qubits());
                            q.x(static_cast<dd::Qubit>((std::size_t)current->v - (p2Obj + 1U)), ctrlFinal);
                            ctrlFinal.clear();
                            auto ddTest = dd::buildFunctionality(&q, dd);
                            src         = dd->multiply(src, ddTest);
                        }
                    } else {
                        dd::Controls ctrlFinal;
                        ctrlFinal.insert(currentNode);
                        ctrlFinal.insert(Ctrl.begin(), Ctrl.end());

                        auto q = qc::QuantumComputation(dd->qubits());
                        q.x(static_cast<dd::Qubit>((std::size_t)current->v - (p2Obj + 1U)), ctrlFinal);
                        ctrlFinal.clear();
                        auto ddTest = dd::buildFunctionality(&q, dd);
                        src         = dd->multiply(src, ddTest);
                    }

                    break;
                }
            }

            algoQ(src, dd);
        }
    }

    bool algoP3(dd::mNode* current) {
        std::cout << "inside algoP3" << std::endl;
        if (!(dd::mNode::isTerminal(current))) {
            if (!(current->e[0].isZeroTerminal()) && current->e[1].isZeroTerminal())
                return true;
            else
                return false;
        }

        return false;
    }

    void algoP1(dd::mEdge& src, dd::mNode* current, std::unique_ptr<dd::Package<>>& dd) {
        std::cout << "inside algoP1" << std::endl;
        if (!(dd::mNode::isTerminal(current))) {
            syrec::TruthTable::Cube::Vector p1SigVec;
            syrec::TruthTable::Cube         p1TempVec;

            syrec::TruthTable::Cube::Vector p2SigVec;
            syrec::TruthTable::Cube         p2TempVec;

            sigPath(current->e[0].p, p1SigVec, p1TempVec);
            sigPath(current->e[1].p, p2SigVec, p2TempVec);

            if ((current->e[0].isZeroTerminal() && !current->e[1].isZeroTerminal()) || (p2SigVec.size() > p1SigVec.size())) {
                syrec::TruthTable::Cube::Vector rootSigVec;
                syrec::TruthTable::Cube         rootTempVec;

                rootPath(src.p, current, rootSigVec, rootTempVec);

                dd::Controls Ctrl;

                for (auto const& i: rootSigVec) {
                    for (std::size_t j = 0; j < i.size(); j++) {
                        if (*(i[j])) {
                            std::cout << "root" << std::endl;
                            std::cout << "pos" << std::endl;
                            dd::Control obj;
                            obj.qubit = static_cast<dd::Qubit>((i.size() + current->v) - j);
                            obj.type  = dd::Control::Type::pos;
                            Ctrl.insert(obj);
                        } else {
                            std::cout << "root" << std::endl;
                            std::cout << "neg" << std::endl;
                            dd::Control obj;
                            obj.qubit = static_cast<dd::Qubit>((i.size() + current->v) - j);
                            obj.type  = dd::Control::Type::neg;
                            Ctrl.insert(obj);
                        }
                    }

                    auto q = qc::QuantumComputation(dd->qubits());
                    q.x(current->v, Ctrl);
                    Ctrl.clear();
                    auto ddTest = dd::buildFunctionality(&q, dd);
                    src         = dd->multiply(src, ddTest);
                }
            }
        }
    }

    void algoP2(dd::mEdge& src, dd::mNode* current, std::unique_ptr<dd::Package<>>& dd) {
        std::cout << "inside algoP2" << std::endl;
        if (!(dd::mNode::isTerminal(current))) {
            syrec::TruthTable::Cube::Vector p1SigVec;
            syrec::TruthTable::Cube         p1TempVec;

            syrec::TruthTable::Cube::Vector p2SigVec;
            syrec::TruthTable::Cube         p2TempVec;

            std::vector<std::size_t> indices;

            sigPath(current->e[0].p, p1SigVec, p1TempVec);
            sigPath(current->e[1].p, p2SigVec, p2TempVec);

            std::size_t flag = 0;

            for (std::size_t i = 0; i < p2SigVec.size(); i++) {
                auto it = std::find(p1SigVec.begin(), p1SigVec.end(), p2SigVec[i]);
                if (it != p1SigVec.end()) {
                    indices.push_back(i);
                } else
                    flag = flag + 1;
            }

            if (flag == p2SigVec.size() && flag != 0) {
                syrec::TruthTable::Cube::Vector rootSigVec;
                syrec::TruthTable::Cube         rootTempVec;

                rootPath(src.p, current, rootSigVec, rootTempVec);

                dd::Controls Ctrl;

                for (auto const& i: rootSigVec) {
                    for (std::size_t j = 0; j < i.size(); j++) {
                        if (*(i[j])) {
                            std::cout << "root" << std::endl;
                            std::cout << "pos" << std::endl;
                            dd::Control obj;
                            obj.qubit = static_cast<dd::Qubit>((i.size() + current->v) - j);
                            obj.type  = dd::Control::Type::pos;
                            Ctrl.insert(obj);
                        } else {
                            std::cout << "root" << std::endl;
                            std::cout << "neg" << std::endl;
                            dd::Control obj;
                            obj.qubit = static_cast<dd::Qubit>((i.size() + current->v) - j);
                            obj.type  = dd::Control::Type::neg;
                            Ctrl.insert(obj);
                        }
                    }

                    for (auto const& p2: p2SigVec) {
                        for (std::size_t pj = 0; pj < p2.size(); pj++) {
                            if (*(p2[pj])) {
                                std::cout << "not root" << std::endl;
                                std::cout << "pos" << std::endl;
                                dd::Control obj;
                                obj.qubit = static_cast<dd::Qubit>(current->v - (pj + 1));
                                obj.type  = dd::Control::Type::pos;
                                Ctrl.insert(obj);
                            } else {
                                std::cout << "not root" << std::endl;
                                std::cout << "neg" << std::endl;
                                dd::Control obj;
                                obj.qubit = static_cast<dd::Qubit>(current->v - (pj + 1));
                                obj.type  = dd::Control::Type::neg;
                                Ctrl.insert(obj);
                            }
                        }

                        auto q = qc::QuantumComputation(dd->qubits());
                        q.x(current->v, Ctrl);
                        Ctrl.clear();
                        auto ddTest = dd::buildFunctionality(&q, dd);
                        src         = dd->multiply(src, ddTest);
                    }
                }

            }

            else {
                algoP4(src, current, p1SigVec, p2SigVec, indices, dd);
            }
        }
    }

    void algoP(dd::mEdge& src, dd::mNode* current, std::unique_ptr<dd::Package<>>& dd) {
        std::cout << "inside algoP" << std::endl;
        auto srcCpy = src;

        if (algoP3(current)) {
            return;
        }

        algoP1(src, current, dd);

        if (srcCpy == src) {
            algoP2(src, current, dd);
        }
    }

    void algoQ(dd::mEdge& src, std::unique_ptr<dd::Package<>>& dd) {
        std::cout << "inside algoQ" << std::endl;
        std::vector<dd::mNode*> q;
        std::vector<dd::mNode*> visited;

        algoP(src, src.p, dd);

        auto currNode = src.p;

        while (src != dd->makeIdent(static_cast<dd::Qubit>(dd->qubits()))) {
            std::cout << "while" << std::endl;
            for (const auto& edge: currNode->e) {
                if (!edge.isTerminal()) {
                    auto it = std::find(visited.begin(), visited.end(), edge.p);

                    if (it == visited.end()) {
                        algoP(src, edge.p, dd);
                        q.emplace_back(edge.p);
                        visited.emplace_back(edge.p);
                    }
                }
            }

            currNode = q.front();
            q.erase(q.begin());
        }
    }

} // namespace syrec