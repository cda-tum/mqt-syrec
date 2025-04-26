/*
 * Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
 * Copyright (c) 2025 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#include "algorithms/synthesis/syrec_line_aware_synthesis.hpp"

#include "algorithms/synthesis/syrec_synthesis.hpp"
#include "core/circuit.hpp"
#include "core/properties.hpp"
#include "core/syrec/expression.hpp"
#include "core/syrec/program.hpp"
#include "core/syrec/statement.hpp"

#include <algorithm>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/properties.hpp>
#include <vector>

namespace syrec {
    /// checking the entire statement
    bool LineAwareSynthesis::fullStatement(const Statement::ptr& statement) {
        bool okay = false;
        if (auto const* stat = dynamic_cast<AssignStatement*>(statement.get())) {
            okay = fullStatement(*stat);
        } else {
            return false;
        }

        return okay;
    }

    bool LineAwareSynthesis::fullStatement(const AssignStatement& statement) {
        std::vector<unsigned> d;
        std::vector<unsigned> statLhs;
        std::vector<unsigned> ddd;
        getVariables(statement.lhs, statLhs);

        opRhsLhsExpression(statement.rhs, d);

        if (opVec.empty()) {
            return false;
        }
        flow(statement.rhs, ddd);

        /// Only when the rhs input signals are repeated (since the results are stored in the rhs)

        if (checkRepeats()) {
            std::vector<unsigned> dd;
            flow(statement.rhs, dd);

            if (expOpVector.size() == 1) {
                if (expOpVector.at(0) == 1 || expOpVector.at(0) == 2) {
                    /// cancel out the signals

                    expOpVector.clear();
                    assignOpVector.clear();
                    expLhsVector.clear();
                    expRhsVector.clear();
                    opVec.clear();
                } else {
                    if (statement.op == 1) {
                        expressionSingleOp(1, expLhsVector.at(0), statLhs);
                        expressionSingleOp(1, expRhsVector.at(0), statLhs);
                        expOpVector.clear();
                        assignOpVector.clear();
                        expLhsVector.clear();
                        expRhsVector.clear();
                        opVec.clear();
                    } else {
                        expressionSingleOp(statement.op, expLhsVector.at(0), statLhs);
                        expressionSingleOp(expOpVector.at(0), expRhsVector.at(0), statLhs);
                        expOpVector.clear();
                        assignOpVector.clear();
                        expLhsVector.clear();
                        expRhsVector.clear();
                        opVec.clear();
                    }
                }

            } else {
                std::vector<unsigned> lines;
                if (expLhsVector.at(0) == expRhsVector.at(0)) {
                    if (expOpVector.at(0) == 1 || expOpVector.at(0) == 2) {
                        /// cancel out the signals
                    } else if (expOpVector.at(0) != 1 || expOpVector.at(0) != 2) {
                        expressionSingleOp(statement.op, expLhsVector.at(0), statLhs);
                        expressionSingleOp(expOpVector.at(0), expRhsVector.at(0), statLhs);
                    }
                } else {
                    solver(statLhs, statement.op, expLhsVector.at(0), expOpVector.at(0), expRhsVector.at(0));
                }

                unsigned              j = 0;
                unsigned              z = 0;
                std::vector<unsigned> statAssignOp;
                if ((expOpVector.size() % 2) == 0) {
                    z = (static_cast<int>(expOpVector.size()) / 2);
                } else {
                    z = (static_cast<int>((expOpVector.size()) - 1) / 2);
                }

                for (unsigned k = 0; k <= z - 1; k++) {
                    statAssignOp.push_back(assignOpVector.at(k));
                }

                /// Assignment operations

                std::reverse(statAssignOp.begin(), statAssignOp.end());

                /// If reversible assignment is "-", the assignment operations must negated appropriately

                if (statement.op == 1) {
                    for (unsigned int& i: statAssignOp) {
                        if (i == 0) {
                            i = 1;
                        } else if (i == 1) {
                            i = 0;
                        } else {
                            continue;
                        }
                    }
                }

                for (unsigned i = 1; i <= expOpVector.size() - 1; i++) {
                    /// when both rhs and lhs exist
                    if ((!expLhsVector.at(i).empty()) && (!expRhsVector.at(i).empty())) {
                        if (expLhsVector.at(i) == expRhsVector.at(i)) {
                            if (expOpVector.at(i) == 1 || expOpVector.at(i) == 2) {
                                /// cancel out the signals
                                j = j + 1;
                            } else if (expOpVector.at(i) != 1 || expOpVector.at(i) != 2) {
                                if (statAssignOp.at(j) == 1) {
                                    expressionSingleOp(1, expLhsVector.at(i), statLhs);
                                    expressionSingleOp(1, expRhsVector.at(i), statLhs);
                                    j = j + 1;
                                } else {
                                    expressionSingleOp(statAssignOp.at(j), expLhsVector.at(i), statLhs);
                                    expressionSingleOp(expOpVector.at(i), expRhsVector.at(i), statLhs);
                                    j = j + 1;
                                }
                            }
                        } else {
                            solver(statLhs, statAssignOp.at(j), expLhsVector.at(i), expOpVector.at(i), expRhsVector.at(i));
                            j = j + 1;
                        }
                    }

                    /// when only lhs exists o rhs exists
                    else if (((expLhsVector.at(i).empty()) && !(expRhsVector.at(i).empty())) || ((!expLhsVector.at(i).empty()) && (expRhsVector.at(i).empty()))) {
                        expEvaluate(lines, statAssignOp.at(j), expRhsVector.at(i), statLhs);

                        j = j + 1;
                    }
                }
                expOpVector.clear();
                assignOpVector.clear();
                expLhsVector.clear();
                expRhsVector.clear();
                opVec.clear();
            }
        } else {
            expOpVector.clear();
            assignOpVector.clear();
            expLhsVector.clear();
            expRhsVector.clear();
            opVec.clear();
            return false;
        }

        expOpVector.clear();
        assignOpVector.clear();
        expLhsVector.clear();
        expRhsVector.clear();
        opVec.clear();
        return true;
    }

    bool LineAwareSynthesis::flow(const Expression::ptr& expression, std::vector<unsigned>& v) {
        if (auto const* binary = dynamic_cast<BinaryExpression*>(expression.get())) {
            return flow(*binary, v);
        }
        if (auto const* var = dynamic_cast<VariableExpression*>(expression.get())) {
            return flow(*var, v);
        }
        return false;
    }

    bool LineAwareSynthesis::flow(const VariableExpression& expression, std::vector<unsigned>& v) {
        getVariables(expression.var, v);
        return true;
    }

    /// generating LHS and RHS (can be whole expressions as well)
    bool LineAwareSynthesis::flow(const BinaryExpression& expression, const std::vector<unsigned>& v [[maybe_unused]]) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;
        std::vector<unsigned> comp;
        assignOpVector.push_back(expression.op);

        if (!flow(expression.lhs, lhs) || !flow(expression.rhs, rhs)) {
            return false;
        }

        expLhsVector.push_back(lhs);
        expRhsVector.push_back(rhs);
        expOpVector.push_back(expression.op);
        return true;
    }

    bool LineAwareSynthesis::solver(const std::vector<unsigned>& expRhs, unsigned statOp, const std::vector<unsigned>& expLhs, unsigned expOp, const std::vector<unsigned>& statLhs) {
        if (statOp == expOp) {
            if (expOp == 1) {
                expressionSingleOp(1, expLhs, expRhs);
                expressionSingleOp(0, statLhs, expRhs);
            } else {
                expressionSingleOp(statOp, expLhs, expRhs);
                expressionSingleOp(statOp, statLhs, expRhs);
            }
        } else {
            std::vector<unsigned> lines;
            subFlag = true;
            expEvaluate(lines, expOp, expLhs, statLhs);
            subFlag = false;
            expEvaluate(lines, statOp, lines, expRhs);
            subFlag = true;
            if (expOp < 3) {
                expressionOpInverse(expOp, expLhs, statLhs);
            }
        }
        subFlag = false;
        return true;
    }

    bool LineAwareSynthesis::opRhsLhsExpression(const Expression::ptr& expression, std::vector<unsigned>& v) {
        if (auto const* binary = dynamic_cast<BinaryExpression*>(expression.get())) {
            return opRhsLhsExpression(*binary, v);
        }
        if (auto const* var = dynamic_cast<VariableExpression*>(expression.get())) {
            return opRhsLhsExpression(*var, v);
        }
        return false;
    }

    bool LineAwareSynthesis::opRhsLhsExpression(const VariableExpression& expression, std::vector<unsigned>& v) {
        getVariables(expression.var, v);
        return true;
    }

    bool LineAwareSynthesis::opRhsLhsExpression(const BinaryExpression& expression, std::vector<unsigned>& v) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;

        if (!opRhsLhsExpression(expression.lhs, lhs) || !opRhsLhsExpression(expression.rhs, rhs)) {
            return false;
        }

        v = rhs;
        opVec.push_back(expression.op);
        return true;
    }

    void LineAwareSynthesis::popExp() {
        SyrecSynthesis::expOpp.pop();
        SyrecSynthesis::expLhss.pop();
        SyrecSynthesis::expRhss.pop();
    }

    void LineAwareSynthesis::inverse() {
        expressionOpInverse(SyrecSynthesis::expOpp.top(), SyrecSynthesis::expLhss.top(), SyrecSynthesis::expRhss.top());
        SyrecSynthesis::subFlag = false;
        popExp();
    }

    void LineAwareSynthesis::assignAdd(bool& status, std::vector<unsigned>& rhs, std::vector<unsigned>& lhs, const unsigned& op) {
        if (!SyrecSynthesis::expOpp.empty() && SyrecSynthesis::expOpp.top() == op) {
            SyrecSynthesis::increase(rhs, SyrecSynthesis::expLhss.top()); //status = bitwiseCnot(lhs, expLhss.top())
            status = SyrecSynthesis::increase(rhs, SyrecSynthesis::expRhss.top());
            popExp();
        } else {
            status = SyrecSynthesis::increase(rhs, lhs);
        }
        while (!SyrecSynthesis::expOpp.empty()) {
            inverse();
        }
    }

    void LineAwareSynthesis::assignSubtract(bool& status, std::vector<unsigned>& rhs, std::vector<unsigned>& lhs, const unsigned& op) {
        if (!SyrecSynthesis::expOpp.empty() && SyrecSynthesis::expOpp.top() == op) {
            SyrecSynthesis::decrease(rhs, SyrecSynthesis::expLhss.top()); //status = bitwiseCnot(lhs, expLhss.top())
            status = SyrecSynthesis::increase(rhs, SyrecSynthesis::expRhss.top());
            popExp();
        } else {
            status = SyrecSynthesis::decrease(rhs, lhs);
        }
        while (!SyrecSynthesis::expOpp.empty()) {
            inverse();
        }
    }

    void LineAwareSynthesis::assignExor(bool& status, std::vector<unsigned>& lhs, std::vector<unsigned>& rhs, const unsigned& op) {
        if (!SyrecSynthesis::expOpp.empty() && SyrecSynthesis::expOpp.top() == op) {
            SyrecSynthesis::bitwiseCnot(lhs, SyrecSynthesis::expLhss.top()); //status = bitwiseCnot(lhs, expLhss.top())
            status = SyrecSynthesis::bitwiseCnot(lhs, SyrecSynthesis::expRhss.top());
            popExp();
        } else {
            status = SyrecSynthesis::bitwiseCnot(lhs, rhs);
        }
        while (!SyrecSynthesis::expOpp.empty()) {
            inverse();
        }
    }
    /// This function is used when input signals (rhs) are equal (just to solve statements individually)
    bool LineAwareSynthesis::expEvaluate(std::vector<unsigned>& lines, unsigned op, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) {
        switch (op) {
            case BinaryExpression::Add: // +
                increase(rhs, lhs);
                lines = rhs;
                break;
            case BinaryExpression::Subtract: // -
                if (subFlag) {
                    decreaseNewAssign(rhs, lhs);
                    lines = rhs;
                } else {
                    decrease(rhs, lhs);
                    lines = rhs;
                }
                break;
            case BinaryExpression::Exor: // ^
                bitwiseCnot(rhs, lhs);   // duplicate lhs
                lines = rhs;
                break;
            default:
                return false;
        }

        return true;
    }

    bool LineAwareSynthesis::decreaseNewAssign(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs) {
        for (const auto lh: lhs) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(lh);
        }

        increase(rhs, lhs);

        for (const auto lh: lhs) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(lh);
        }

        for (unsigned i = 0U; i < lhs.size(); ++i) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(rhs.at(i));
        }
        return true;
    }

    bool LineAwareSynthesis::expressionSingleOp(const unsigned op, const std::vector<unsigned>& expLhs, const std::vector<unsigned>& expRhs) {
        switch (op) {
            case BinaryExpression::Add: // +
                increase(expRhs, expLhs);
                break;
            case BinaryExpression::Subtract: // -
                if (subFlag) {
                    decreaseNewAssign(expRhs, expLhs);
                } else {
                    decrease(expRhs, expLhs);
                }
                break;
            case BinaryExpression::Exor: // ^
                bitwiseCnot(expRhs, expLhs);
                break;
            default:
                return false;
        }
        return true;
    }

    bool LineAwareSynthesis::expressionOpInverse(const unsigned op, const std::vector<unsigned>& expLhs, const std::vector<unsigned>& expRhs) {
        switch (op) {
            case BinaryExpression::Add: // +
                decrease(expRhs, expLhs);
                break;
            case BinaryExpression::Subtract: // -
                decreaseNewAssign(expRhs, expLhs);
                break;
            case BinaryExpression::Exor: // ^
                bitwiseCnot(expRhs, expLhs);
                break;
            default:
                return false;
        }
        return true;
    }

    bool LineAwareSynthesis::synthesize(Circuit& circ, const Program& program, const Properties::ptr& settings, const Properties::ptr& statistics) {
        LineAwareSynthesis synthesizer(circ);
        return SyrecSynthesis::synthesize(&synthesizer, circ, program, settings, statistics);
    }
} // namespace syrec
