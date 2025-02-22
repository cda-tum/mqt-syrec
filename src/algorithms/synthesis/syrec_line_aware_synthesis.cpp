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
        std::vector<unsigned> comp;
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

            // Binaryexpression ADD=0, MINUS=1, EXOR=2
            // AssignOperation ADD=0, MINUS=1, EXOR=2
            if (expOpVector.size() == 1) {
                if (expOpVector.front() == BinaryExpression::BinaryOperation::Subtract || expOpVector.front() == BinaryExpression::BinaryOperation::Exor) {
                    /// cancel out the signals

                    expOpVector.clear();
                    assignOpVector.clear();
                    expLhsVector.clear();
                    expRhsVector.clear();
                    opVec.clear();
                } else {
                    if (statement.assignOperation == AssignStatement::AssignOperation::Subtract) {
                        expressionSingleOp(BinaryExpression::BinaryOperation::Subtract, expLhsVector.at(0), statLhs);
                        expressionSingleOp(BinaryExpression::BinaryOperation::Subtract, expRhsVector.at(0), statLhs);
                        expOpVector.clear();
                        assignOpVector.clear();
                        expLhsVector.clear();
                        expRhsVector.clear();
                        opVec.clear();
                    } else {
                        const std::optional<BinaryExpression::BinaryOperation> mappedToBinaryOperation = tryMapAssignmentToBinaryOperation(statement.assignOperation);
                        if (!mappedToBinaryOperation.has_value())
                            return false;

                        expressionSingleOp(*mappedToBinaryOperation, expLhsVector.at(0), statLhs);
                        expressionSingleOp(expOpVector.front(), expRhsVector.at(0), statLhs);
                        expOpVector.clear();
                        assignOpVector.clear();
                        expLhsVector.clear();
                        expRhsVector.clear();
                        opVec.clear();
                    }
                }

            } else {
                std::vector<unsigned> lines;
                if (expLhsVector.front() == expRhsVector.front()) {
                    if (!(expOpVector.front() == BinaryExpression::BinaryOperation::Subtract || expOpVector.front() == BinaryExpression::BinaryOperation::Exor)) {
                        const std::optional<BinaryExpression::BinaryOperation> mappedToBinaryOperation = tryMapAssignmentToBinaryOperation(statement.assignOperation);
                        if (!mappedToBinaryOperation.has_value())
                            return false;

                        expressionSingleOp(*mappedToBinaryOperation, expLhsVector.at(0), statLhs);
                        expressionSingleOp(expOpVector.front(), expRhsVector.at(0), statLhs);
                    }
                    // Else cancel out signals
                } else {
                    solver(statLhs, statement.assignOperation, expLhsVector.at(0), expOpVector.at(0), expRhsVector.at(0));
                }

                unsigned              j = 0;
                unsigned              z = 0;
                std::vector < AssignStatement::AssignOperation> statAssignOp;
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

                if (statement.assignOperation == AssignStatement::AssignOperation::Subtract) {
                    for (AssignStatement::AssignOperation& i: statAssignOp) {
                        if (i == AssignStatement::AssignOperation::Add) {
                            i = AssignStatement::AssignOperation::Subtract;
                        } else if (i == AssignStatement::AssignOperation::Subtract) {
                            i = AssignStatement::AssignOperation::Add;
                        }
                    }
                }

                for (unsigned i = 1; i <= expOpVector.size() - 1; i++) {
                    /// when both rhs and lhs exist
                    if ((!expLhsVector.at(i).empty()) && (!expRhsVector.at(i).empty())) {
                        if (expLhsVector.at(i) == expRhsVector.at(i)) {
                            if (expOpVector.at(i) == BinaryExpression::BinaryOperation::Subtract || expOpVector.at(i) == BinaryExpression::BinaryOperation::Exor) {
                                /// cancel out the signals
                                j = j + 1;
                            } else if (expOpVector.at(i) != BinaryExpression::BinaryOperation::Subtract || expOpVector.at(i) != BinaryExpression::BinaryOperation::Exor) {
                                if (statAssignOp.at(j) == AssignStatement::AssignOperation::Subtract) {
                                    expressionSingleOp(BinaryExpression::BinaryOperation::Subtract, expLhsVector.at(i), statLhs);
                                    expressionSingleOp(BinaryExpression::BinaryOperation::Subtract, expRhsVector.at(i), statLhs);
                                    j = j + 1;
                                } else {
                                    const std::optional<BinaryExpression::BinaryOperation> mappedToBinaryOperation = tryMapAssignmentToBinaryOperation(statAssignOp.at(j));
                                    if (!mappedToBinaryOperation.has_value())
                                        return false;

                                    expressionSingleOp(*mappedToBinaryOperation, expLhsVector.at(i), statLhs);
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
                        const std::optional<BinaryExpression::BinaryOperation> mappedToBinaryOperation = tryMapAssignmentToBinaryOperation(statAssignOp.at(j));
                        if (!mappedToBinaryOperation.has_value())
                            return false;

                        expEvaluate(lines, *mappedToBinaryOperation, expRhsVector.at(i), statLhs);

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

        if (const std::optional<AssignStatement::AssignOperation> mappedToAssignmentOperation = tryMapBinaryToAssignmentOperation(expression.binaryOperation); mappedToAssignmentOperation.has_value())
            assignOpVector.push_back(*mappedToAssignmentOperation);
        else
            return false;

        if (!flow(expression.lhs, lhs) || !flow(expression.rhs, rhs)) {
            return false;
        }

        expLhsVector.push_back(lhs);
        expRhsVector.push_back(rhs);
        expOpVector.push_back(expression.binaryOperation);
        return true;
    }

    bool LineAwareSynthesis::solver(const std::vector<unsigned>& expRhs, AssignStatement::AssignOperation statOp, const std::vector<unsigned>& expLhs, BinaryExpression::BinaryOperation expOp, const std::vector<unsigned>& statLhs) {
        const std::optional<BinaryExpression::BinaryOperation> mappedToBinaryOperation = tryMapAssignmentToBinaryOperation(statOp);
        if (!mappedToBinaryOperation.has_value()) {
            subFlag = false;
            return true;
        }

        if (*mappedToBinaryOperation == expOp) {
            if (expOp == BinaryExpression::BinaryOperation::Subtract) {
                expressionSingleOp(BinaryExpression::BinaryOperation::Subtract, expLhs, expRhs);
                expressionSingleOp(BinaryExpression::BinaryOperation::Add, statLhs, expRhs);
            } else {
                expressionSingleOp(*mappedToBinaryOperation, expLhs, expRhs);
                expressionSingleOp(*mappedToBinaryOperation, statLhs, expRhs);
            }
        } else {
            std::vector<unsigned> lines;
            subFlag = true;
            expEvaluate(lines, expOp, expLhs, statLhs);
            subFlag = false;
            expEvaluate(lines, *mappedToBinaryOperation, lines, expRhs);
            subFlag = true;
            switch (expOp) {
                case BinaryExpression::BinaryOperation::Add:
                case BinaryExpression::BinaryOperation::Subtract:
                case BinaryExpression::BinaryOperation::Exor:
                    expressionOpInverse(expOp, expLhs, statLhs);
                    break;
                default:
                    break;
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
        opVec.push_back(expression.binaryOperation);
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

    void LineAwareSynthesis::assignAdd(bool& status, std::vector<unsigned>& rhs, std::vector<unsigned>& lhs, const AssignStatement::AssignOperation& op) {
        if (const std::optional<BinaryExpression::BinaryOperation> mappedToBinaryOperation = !expOpp.empty() ? tryMapAssignmentToBinaryOperation(op) : std::nullopt;
            mappedToBinaryOperation.has_value() && *mappedToBinaryOperation == expOpp.top()) {
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

    void LineAwareSynthesis::assignSubtract(bool& status, std::vector<unsigned>& rhs, std::vector<unsigned>& lhs, const AssignStatement::AssignOperation& op) {
        if (const std::optional<BinaryExpression::BinaryOperation> mappedToBinaryOperation = !expOpp.empty() ? tryMapAssignmentToBinaryOperation(op) : std::nullopt;
            mappedToBinaryOperation.has_value() && *mappedToBinaryOperation == expOpp.top()) {
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

    void LineAwareSynthesis::assignExor(bool& status, std::vector<unsigned>& lhs, std::vector<unsigned>& rhs, const AssignStatement::AssignOperation& op) {
        if (const std::optional<BinaryExpression::BinaryOperation> mappedToBinaryOperation = !expOpp.empty() ? tryMapAssignmentToBinaryOperation(op) : std::nullopt;
            mappedToBinaryOperation.has_value() && *mappedToBinaryOperation == expOpp.top()) {
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
    bool LineAwareSynthesis::expEvaluate(std::vector<unsigned>& lines, BinaryExpression::BinaryOperation op, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) {
        switch (op) {
            case BinaryExpression::BinaryOperation::Add: // +
                increase(rhs, lhs);
                lines = rhs;
                break;
            case BinaryExpression::BinaryOperation::Subtract: // -
                if (subFlag) {
                    decreaseNewAssign(rhs, lhs);
                    lines = rhs;
                } else {
                    decrease(rhs, lhs);
                    lines = rhs;
                }
                break;
            case BinaryExpression::BinaryOperation::Exor: // ^
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
            (get(boost::vertex_name, cctMan.tree)[cctMan.current].circ)->appendNot(lh);
        }

        increase(rhs, lhs);

        for (const auto lh: lhs) {
            (get(boost::vertex_name, cctMan.tree)[cctMan.current].circ)->appendNot(lh);
        }

        for (unsigned i = 0U; i < lhs.size(); ++i) {
            (get(boost::vertex_name, cctMan.tree)[cctMan.current].circ)->appendNot(rhs.at(i));
        }
        return true;
    }

    bool LineAwareSynthesis::expressionSingleOp(BinaryExpression::BinaryOperation op, const std::vector<unsigned>& expLhs, const std::vector<unsigned>& expRhs) {
        switch (op) {
            case BinaryExpression::BinaryOperation::Add: // +
                increase(expRhs, expLhs);
                break;
            case BinaryExpression::BinaryOperation::Subtract: // -
                if (subFlag) {
                    decreaseNewAssign(expRhs, expLhs);
                } else {
                    decrease(expRhs, expLhs);
                }
                break;
            case BinaryExpression::BinaryOperation::Exor: // ^
                bitwiseCnot(expRhs, expLhs);
                break;
            default:
                return false;
        }
        return true;
    }

    bool LineAwareSynthesis::expressionOpInverse(BinaryExpression::BinaryOperation op, const std::vector<unsigned>& expLhs, const std::vector<unsigned>& expRhs) {
        switch (op) {
            case BinaryExpression::BinaryOperation::Add: // +
                decrease(expRhs, expLhs);
                break;
            case BinaryExpression::BinaryOperation::Subtract: // -
                decreaseNewAssign(expRhs, expLhs);
                break;
            case BinaryExpression::BinaryOperation::Exor: // ^
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

    std::optional<AssignStatement::AssignOperation> LineAwareSynthesis::tryMapBinaryToAssignmentOperation(BinaryExpression::BinaryOperation binaryOperation) noexcept {
        switch (binaryOperation) {
            case BinaryExpression::BinaryOperation::Add:
                return AssignStatement::AssignOperation::Add;
            case BinaryExpression::BinaryOperation::Subtract:
                return AssignStatement::AssignOperation::Subtract;
            case BinaryExpression::BinaryOperation::Exor:
                return AssignStatement::AssignOperation::Exor;
            default:
                return std::nullopt;
        }
    }

    std::optional<BinaryExpression::BinaryOperation> LineAwareSynthesis::tryMapAssignmentToBinaryOperation(AssignStatement::AssignOperation assignOperation) noexcept {
        switch (assignOperation) {
            case AssignStatement::AssignOperation::Add:
                return BinaryExpression::BinaryOperation::Add;

            case AssignStatement::AssignOperation::Subtract:
                return BinaryExpression::BinaryOperation::Subtract;

            case AssignStatement::AssignOperation::Exor:
                return BinaryExpression::BinaryOperation::Exor;
            default:
                return std::nullopt;
        }
    }
} // namespace syrec
