/*
 * Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
 * Copyright (c) 2025 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#include "algorithms/synthesis/syrec_synthesis.hpp"

#include "core/circuit.hpp"
#include "core/gate.hpp"
#include "core/syrec/expression.hpp"
#include "core/syrec/program.hpp"
#include "core/syrec/statement.hpp"
#include "core/syrec/variable.hpp"
#include "core/utils/timer.hpp"

#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <numeric>
#include <stack>

namespace syrec {

    struct Annotater {
        explicit Annotater(Circuit& circ, const std::stack<Statement::ptr>& stmts):
            circ(circ),
            stmts(stmts) {}

        // Operator needs this signature to work
        void operator()(Gate const& g) const {
            if (!stmts.empty()) {
                circ.annotate(g, "lno", std::to_string(stmts.top()->lineNumber));
            }
        }

    private:
        Circuit&                          circ;
        const std::stack<Statement::ptr>& stmts;
    };

    // Helper Functions for the synthesis methods
    SyrecSynthesis::SyrecSynthesis(Circuit& circ):
        circ(circ) {
        freeConstLinesMap.try_emplace(false /* emplacing a default constructed object */);
        freeConstLinesMap.try_emplace(true /* emplacing a default constructed object */);

        // root anlegen
        cctMan.current = add_vertex(cctMan.tree);
        cctMan.root    = cctMan.current;
        // Blatt anlegen
        cctMan.current                                            = add_vertex(cctMan.tree);
        get(boost::vertex_name, cctMan.tree)[cctMan.current].circ = std::make_shared<Circuit>();
        get(boost::vertex_name, cctMan.tree)[cctMan.current].circ->gateAdded.connect(Annotater(*get(boost::vertex_name, cctMan.tree)[cctMan.current].circ, stmts));
        add_edge(cctMan.root, cctMan.current, cctMan.tree);
    }

    void SyrecSynthesis::setMainModule(const Module::ptr& mainModule) {
        assert(modules.empty());
        modules.push(mainModule);
    }

    bool SyrecSynthesis::onModule(const Module::ptr& main) {
        for (const auto& stat: main->statements) {
            if (processStatement(stat)) {
                return false;
            }
        }
        return assembleCircuit(cctMan.root);
    }

    /// If the input signals are repeated (i.e., rhs input signals are repeated)
    bool SyrecSynthesis::checkRepeats() {
        std::vector checkLhsVec(expLhsVector.cbegin(), expLhsVector.cend());
        std::vector checkRhsVec(expRhsVector.cbegin(), expRhsVector.cend());

        for (unsigned k = 0; k < checkLhsVec.size(); k++) {
            if (checkLhsVec.at(k).empty()) {
                checkLhsVec.erase(checkLhsVec.begin() + k);
            }
        }

        for (unsigned k = 0; k < checkRhsVec.size(); k++) {
            if (checkRhsVec.at(k).empty()) {
                checkRhsVec.erase(checkRhsVec.begin() + k);
            }
        }

        for (int i = 0; i < static_cast<int>(checkRhsVec.size()); i++) {
            for (int j = 0; j < static_cast<int>(checkRhsVec.size()); j++) {
                if ((j != i) && (checkRhsVec.at(i) == checkRhsVec.at(j))) {
                    expOpVector.clear();
                    expLhsVector.clear();
                    expRhsVector.clear();
                    return true;
                }
            }
        }

        for (auto const& i: checkLhsVec) {
            for (auto const& j: checkRhsVec) {
                if (i == j) {
                    expOpVector.clear();
                    expLhsVector.clear();
                    expRhsVector.clear();
                    return true;
                }
            }
        }

        expOpVector.clear();
        expLhsVector.clear();
        expRhsVector.clear();
        return false;
    }

    bool SyrecSynthesis::opRhsLhsExpression([[maybe_unused]] const Expression::ptr& expression, [[maybe_unused]] std::vector<unsigned>& v) {
        return true;
    }
    bool SyrecSynthesis::opRhsLhsExpression([[maybe_unused]] const VariableExpression& expression, [[maybe_unused]] std::vector<unsigned>& v) {
        return true;
    }
    bool SyrecSynthesis::opRhsLhsExpression([[maybe_unused]] const BinaryExpression& expression, [[maybe_unused]] std::vector<unsigned>& v) {
        return true;
    }

    bool SyrecSynthesis::onStatement(const Statement::ptr& statement) {
        stmts.push(statement);
        bool okay = false;
        if (auto const* swapStat = dynamic_cast<SwapStatement*>(statement.get())) {
            okay = onStatement(*swapStat);
        } else if (auto const* unaryStat = dynamic_cast<UnaryStatement*>(statement.get())) {
            okay = onStatement(*unaryStat);
        } else if (auto const* assignStat = dynamic_cast<AssignStatement*>(statement.get())) {
            okay = onStatement(*assignStat);
        } else if (auto const* ifStat = dynamic_cast<IfStatement*>(statement.get())) {
            okay = onStatement(*ifStat);
        } else if (auto const* forStat = dynamic_cast<ForStatement*>(statement.get())) {
            okay = onStatement(*forStat);
        } else if (auto const* callStat = dynamic_cast<CallStatement*>(statement.get())) {
            okay = onStatement(*callStat);
        } else if (auto const* uncallStat = dynamic_cast<UncallStatement*>(statement.get())) {
            okay = onStatement(*uncallStat);
        } else if (auto const* skipStat = statement.get()) {
            okay = onStatement(*skipStat);
        } else {
            return false;
        }

        stmts.pop();
        return okay;
    }

    bool SyrecSynthesis::onStatement(const SwapStatement& statement) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;

        getVariables(statement.lhs, lhs);
        getVariables(statement.rhs, rhs);

        assert(lhs.size() == rhs.size());

        swap(lhs, rhs);

        return true;
    }

    bool SyrecSynthesis::onStatement(const UnaryStatement& statement) {
        // load variable
        std::vector<unsigned> var;
        getVariables(statement.var, var);

        switch (statement.op) {
            case UnaryStatement::Invert:
                bitwiseNegation(var);
                break;
            case UnaryStatement::Increment:
                increment(var);
                break;
            case UnaryStatement::Decrement:
                decrement(var);
                break;
            default:
                return false;
        }
        return true;
    }

    bool SyrecSynthesis::onStatement(const AssignStatement& statement) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;
        std::vector<unsigned> d;

        getVariables(statement.lhs, lhs);

        opRhsLhsExpression(statement.rhs, d);
        SyrecSynthesis::onExpression(statement.rhs, rhs, lhs, statement.op);
        opVec.clear();

        bool status = false;

        switch (statement.op) {
            case AssignStatement::Add: {
                assignAdd(status, lhs, rhs, statement.op);
            } break;

            case AssignStatement::Subtract: {
                assignSubtract(status, lhs, rhs, statement.op);
            } break;

            case AssignStatement::Exor: {
                assignExor(status, lhs, rhs, statement.op);
            } break;

            default:
                return false;
        }

        return status;
    }

    bool SyrecSynthesis::onStatement(const IfStatement& statement) {
        // calculate expression
        std::vector<unsigned> expressionResult;
        onExpression(statement.condition, expressionResult, {}, 0U);
        assert(expressionResult.size() == 1U);

        // add new helper line
        unsigned helperLine = expressionResult.front();

        // activate this line
        // NOLINTNEXTLINE warning stems from Boost itself
        addActiveControl(helperLine);

        for (const Statement::ptr& stat: statement.thenStatements) {
            if (processStatement(stat)) {
                return false;
            }
        }

        // toggle helper line
        removeActiveControl(helperLine);
        ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(helperLine);
        addActiveControl(helperLine);

        for (const Statement::ptr& stat: statement.elseStatements) {
            if (processStatement(stat)) {
                return false;
            }
        }

        // de-active helper line
        removeActiveControl(helperLine);
        ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(helperLine);

        return true;
    }

    bool SyrecSynthesis::onStatement(const ForStatement& statement) {
        const auto& [nfrom, nTo] = statement.range;

        const unsigned     from         = nfrom ? nfrom->evaluate(loopMap) : 1U; // default value is 1u
        const unsigned     to           = nTo->evaluate(loopMap);
        const unsigned     step         = statement.step ? statement.step->evaluate(loopMap) : 1U; // default step is +1
        const std::string& loopVariable = statement.loopVariable;

        if (from <= to) {
            for (unsigned i = from; i <= to; i += step) {
                // adjust loop variable if necessary

                if (!loopVariable.empty()) {
                    loopMap[loopVariable] = i;
                }

                for (const auto& stat: statement.statements) {
                    if (processStatement(stat)) {
                        return false;
                    }
                }
            }
        }

        else if (from > to) {
            for (auto i = static_cast<int>(from); i >= static_cast<int>(to); i -= static_cast<int>(step)) {
                // adjust loop variable if necessary

                if (!loopVariable.empty()) {
                    loopMap[loopVariable] = i;
                }

                for (const auto& stat: statement.statements) {
                    if (processStatement(stat)) {
                        return false;
                    }
                }
            }
        }
        // clear loop variable if necessary
        if (!loopVariable.empty()) {
            assert(loopMap.erase(loopVariable) == 1U);
        }

        return true;
    }

    bool SyrecSynthesis::onStatement(const CallStatement& statement) {
        // 1. Adjust the references module's parameters to the call arguments
        for (unsigned i = 0U; i < statement.parameters.size(); ++i) {
            const std::string&   parameter       = statement.parameters.at(i);
            const Variable::ptr& moduleParameter = statement.target->parameters.at(i);

            moduleParameter->setReference(modules.top()->findParameterOrVariable(parameter));
        }

        // 2. Create new lines for the module's variables
        addVariables(circ, statement.target->variables);

        modules.push(statement.target);
        for (const Statement::ptr& stat: statement.target->statements) {
            if (processStatement(stat)) {
                return false;
            }
        }
        modules.pop();

        return true;
    }

    bool SyrecSynthesis::onStatement(const UncallStatement& statement) {
        // 1. Adjust the references module's parameters to the call arguments
        for (unsigned i = 0U; i < statement.parameters.size(); ++i) {
            const std::string& parameter       = statement.parameters.at(i);
            const auto&        moduleParameter = statement.target->parameters.at(i);

            moduleParameter->setReference(modules.top()->findParameterOrVariable(parameter));
        }

        // 2. Create new lines for the module's variables
        addVariables(circ, statement.target->variables);

        modules.push(statement.target);

        const auto statements = statement.target->statements;
        for (auto it = statements.rbegin(); it != statements.rend(); ++it) {
            const auto reverseStatement = (*it)->reverse();
            if (processStatement(reverseStatement)) {
                return false;
            }
        }

        modules.pop();

        return true;
    }

    bool SyrecSynthesis::onStatement(const SkipStatement& statement [[maybe_unused]]) {
        return true;
    }

    bool SyrecSynthesis::onExpression(const Expression::ptr& expression, std::vector<unsigned>& lines, std::vector<unsigned> const& lhsStat, unsigned op) {
        if (auto const* numeric = dynamic_cast<NumericExpression*>(expression.get())) {
            return onExpression(*numeric, lines);
        }
        if (auto const* variable = dynamic_cast<VariableExpression*>(expression.get())) {
            return onExpression(*variable, lines);
        }
        if (auto const* binary = dynamic_cast<BinaryExpression*>(expression.get())) {
            return onExpression(*binary, lines, lhsStat, op);
        }
        if (auto const* shift = dynamic_cast<ShiftExpression*>(expression.get())) {
            return onExpression(*shift, lines, lhsStat, op);
        }
        return false;
    }

    bool SyrecSynthesis::onExpression(const ShiftExpression& expression, std::vector<unsigned>& lines, std::vector<unsigned> const& lhsStat, unsigned op) {
        std::vector<unsigned> lhs;
        if (!onExpression(expression.lhs, lhs, lhsStat, op)) {
            return false;
        }

        unsigned rhs = expression.rhs->evaluate(loopMap);

        switch (expression.op) {
            case ShiftExpression::Left: // <<
                getConstantLines(expression.bitwidth(), 0U, lines);
                leftShift(lines, lhs, rhs);
                break;
            case ShiftExpression::Right: // <<
                getConstantLines(expression.bitwidth(), 0U, lines);
                rightShift(lines, lhs, rhs);
                break;
            default:
                return false;
        }

        return true;
    }

    bool SyrecSynthesis::onExpression(const NumericExpression& expression, std::vector<unsigned>& lines) {
        getConstantLines(expression.bitwidth(), expression.value->evaluate(loopMap), lines);
        return true;
    }

    bool SyrecSynthesis::onExpression(const VariableExpression& expression, std::vector<unsigned>& lines) {
        getVariables(expression.var, lines);
        return true;
    }

    bool SyrecSynthesis::onExpression(const BinaryExpression& expression, std::vector<unsigned>& lines, std::vector<unsigned> const& lhsStat, unsigned op) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;

        if (!onExpression(expression.lhs, lhs, lhsStat, op) || !onExpression(expression.rhs, rhs, lhsStat, op)) {
            return false;
        }

        expLhss.push(lhs);
        expRhss.push(rhs);
        expOpp.push(expression.op);

        if ((expOpp.size() == opVec.size()) && (expOpp.top() == op)) {
            return true;
        }

        switch (expression.op) {
            case BinaryExpression::Add: // +
                expAdd(expression.bitwidth(), lines, lhs, rhs);
                break;
            case BinaryExpression::Subtract: // -
                expSubtract(expression.bitwidth(), lines, lhs, rhs);
                break;
            case BinaryExpression::Exor: // ^
                expExor(expression.bitwidth(), lines, lhs, rhs);
                break;
            case BinaryExpression::Multiply: // *
                getConstantLines(expression.bitwidth(), 0U, lines);
                multiplication(lines, lhs, rhs);
                break;
            case BinaryExpression::Divide: // /
                getConstantLines(expression.bitwidth(), 0U, lines);
                division(lines, lhs, rhs);
                break;
            case BinaryExpression::Modulo: { // %
                getConstantLines(expression.bitwidth(), 0U, lines);
                std::vector<unsigned> quot;
                getConstantLines(expression.bitwidth(), 0U, quot);

                bitwiseCnot(lines, lhs); // duplicate lhs
                modulo(quot, lines, rhs);
            } break;
            case BinaryExpression::LogicalAnd: // &&
                lines.emplace_back(getConstantLine(false));
                conjunction(lines.at(0), lhs.at(0), rhs.at(0));
                break;
            case BinaryExpression::LogicalOr: // ||
                lines.emplace_back(getConstantLine(false));
                disjunction(lines.at(0), lhs.at(0), rhs.at(0));
                break;
            case BinaryExpression::BitwiseAnd: // &
                getConstantLines(expression.bitwidth(), 0U, lines);
                bitwiseAnd(lines, lhs, rhs);
                break;
            case BinaryExpression::BitwiseOr: // |
                getConstantLines(expression.bitwidth(), 0U, lines);
                bitwiseOr(lines, lhs, rhs);
                break;
            case BinaryExpression::LessThan: // <
                lines.emplace_back(getConstantLine(false));
                lessThan(lines.at(0), lhs, rhs);
                break;
            case BinaryExpression::GreaterThan: // >
                lines.emplace_back(getConstantLine(false));
                greaterThan(lines.at(0), lhs, rhs);
                break;
            case BinaryExpression::Equals: // =
                lines.emplace_back(getConstantLine(false));
                equals(lines.at(0), lhs, rhs);
                break;
            case BinaryExpression::NotEquals: // !=
                lines.emplace_back(getConstantLine(false));
                notEquals(lines.at(0), lhs, rhs);
                break;
            case BinaryExpression::LessEquals: // <=
                lines.emplace_back(getConstantLine(false));
                lessEquals(lines.at(0), lhs, rhs);
                break;
            case BinaryExpression::GreaterEquals: // >=
                lines.emplace_back(getConstantLine(false));
                greaterEquals(lines.at(0), lhs, rhs);
                break;
            default:
                return false;
        }

        return true;
    }

    /// Function when the assignment statements consist of binary expressions and does not include repeated input signals

    //**********************************************************************
    //*****                      Unary Operations                      *****
    //**********************************************************************

    bool SyrecSynthesis::bitwiseNegation(const std::vector<unsigned>& dest) {
        for (unsigned idx: dest) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(idx);
        }
        return true;
    }

    bool SyrecSynthesis::decrement(const std::vector<unsigned>& dest) {
        for (unsigned int i: dest) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(i);
            addActiveControl(i);
        }

        for (unsigned int i: dest) {
            removeActiveControl(i);
        }

        return true;
    }

    bool SyrecSynthesis::increment(const std::vector<unsigned>& dest) {
        for (unsigned int i: dest) {
            addActiveControl(i);
        }

        for (int i = static_cast<int>(dest.size()) - 1; i >= 0; --i) {
            removeActiveControl(dest.at(i));
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(dest.at(i));
        }

        return true;
    }

    //**********************************************************************
    //*****                     Binary Operations                      *****
    //**********************************************************************

    bool SyrecSynthesis::bitwiseAnd(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        bool ok = true;
        for (unsigned i = 0U; i < dest.size(); ++i) {
            ok &= conjunction(dest.at(i), src1.at(i), src2.at(i));
        }
        return ok;
    }

    bool SyrecSynthesis::bitwiseCnot(const std::vector<unsigned>& dest, const std::vector<unsigned>& src) {
        for (unsigned i = 0U; i < src.size(); ++i) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(src.at(i), dest.at(i));
        }
        return true;
    }

    bool SyrecSynthesis::bitwiseOr(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        bool ok = true;
        for (unsigned i = 0U; i < dest.size(); ++i) {
            ok &= disjunction(dest.at(i), src1.at(i), src2.at(i));
        }
        return ok;
    }

    bool SyrecSynthesis::conjunction(unsigned dest, unsigned src1, unsigned src2) {
        ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendToffoli(src1, src2, dest);

        return true;
    }

    bool SyrecSynthesis::decreaseWithCarry(const std::vector<unsigned>& dest, const std::vector<unsigned>& src, unsigned carry) {
        for (unsigned i = 0U; i < src.size(); ++i) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(dest.at(i));
        }

        increaseWithCarry(dest, src, carry);

        for (unsigned i = 0U; i < src.size(); ++i) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(dest.at(i));
        }

        return true;
    }

    bool SyrecSynthesis::disjunction(const unsigned dest, const unsigned src1, const unsigned src2) {
        ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(src1, dest);
        ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(src2, dest);
        ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendToffoli(src1, src2, dest);

        return true;
    }

    bool SyrecSynthesis::division(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if (!modulo(dest, src1, src2)) {
            return false;
        }

        std::vector<unsigned> sum;
        std::vector<unsigned> partial;

        for (unsigned i = 1U; i < src1.size(); ++i) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(src2.at(i));
        }

        for (unsigned i = 1U; i < src1.size(); ++i) {
            addActiveControl(src2.at(i));
        }

        for (int i = static_cast<int>(src1.size()) - 1; i >= 0; --i) {
            partial.push_back(src2.at(src1.size() - 1U - i));
            sum.insert(sum.begin(), src1.at(i));
            addActiveControl(dest.at(i));
            increase(sum, partial);
            removeActiveControl(dest.at(i));
            if (i > 0) {
                for (unsigned j = (static_cast<int>(src1.size()) - i); j < src1.size(); ++j) {
                    removeActiveControl(src2.at(j));
                }
                ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(src2.at(src1.size() - i));
                for (unsigned j = (static_cast<int>(src1.size()) + 1U - i); j < src1.size(); ++j) {
                    addActiveControl(src2.at(j));
                }
            }
        }

        return true;
    }

    bool SyrecSynthesis::equals(const unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        for (unsigned i = 0U; i < src1.size(); ++i) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(src2.at(i), src1.at(i));
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(src1.at(i));
        }

        Gate::line_container controls(src1.begin(), src1.end());
        ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendMultiControlToffoli(controls, dest);

        for (unsigned i = 0U; i < src1.size(); ++i) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(src2.at(i), src1.at(i));
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(src1.at(i));
        }

        return true;
    }

    bool SyrecSynthesis::greaterEquals(const unsigned dest, const std::vector<unsigned>& srcTwo, const std::vector<unsigned>& srcOne) {
        if (!greaterThan(dest, srcOne, srcTwo)) {
            return false;
        }
        ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(dest);

        return true;
    }

    bool SyrecSynthesis::greaterThan(const unsigned dest, const std::vector<unsigned>& src2, const std::vector<unsigned>& src1) {
        return lessThan(dest, src1, src2);
    }

    bool SyrecSynthesis::increase(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs) {
        if (auto bitwidth = static_cast<int>(rhs.size()); bitwidth == 1) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(lhs.at(0), rhs.at(0));
        } else {
            for (int i = 1; i <= bitwidth - 1; ++i) {
                ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(lhs.at(i), rhs.at(i));
            }
            for (int i = bitwidth - 2; i >= 1; --i) {
                ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(lhs.at(i), lhs.at(i + 1));
            }
            for (int i = 0; i <= bitwidth - 2; ++i) {
                ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendToffoli(rhs.at(i), lhs.at(i), lhs.at(i + 1));
            }

            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(lhs.at(bitwidth - 1), rhs.at(bitwidth - 1));

            for (int i = bitwidth - 2; i >= 1; --i) {
                ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendToffoli(lhs.at(i), rhs.at(i), lhs.at(i + 1));
                ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(lhs.at(i), rhs.at(i));
            }
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendToffoli(lhs.at(0), rhs.at(0), lhs.at(1));
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(lhs.at(0), rhs.at(0));

            for (int i = 1; i <= bitwidth - 2; ++i) {
                ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(lhs.at(i), lhs.at(i + 1));
            }
            for (int i = 1; i <= bitwidth - 1; ++i) {
                ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(lhs.at(i), rhs.at(i));
            }
        }

        return true;
    }

    bool SyrecSynthesis::decrease(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs) {
        for (unsigned int rh: rhs) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(rh);
        }

        increase(rhs, lhs);

        for (unsigned int rh: rhs) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(rh);
        }
        return true;
    }

    bool SyrecSynthesis::increaseWithCarry(const std::vector<unsigned>& dest, const std::vector<unsigned>& src, unsigned carry) {
        auto bitwidth = static_cast<int>(src.size());

        if (bitwidth == 0) {
            return true;
        }

        for (int i = 1U; i < bitwidth; ++i) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(src.at(i), dest.at(i));
        }

        if (bitwidth > 1) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(src.at(bitwidth - 1), carry);
        }
        for (int i = bitwidth - 2; i > 0; --i) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(src.at(i), src.at(i + 1));
        }

        for (int i = 0U; i < bitwidth - 1; ++i) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendToffoli(src.at(i), dest.at(i), src.at(i + 1));
        }
        ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendToffoli(src.at(bitwidth - 1), dest.at(bitwidth - 1), carry);

        for (int i = bitwidth - 1; i > 0; --i) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(src.at(i), dest.at(i));
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendToffoli(dest.at(i - 1), src.at(i - 1), src.at(i));
        }

        for (int i = 1U; i < bitwidth - 1; ++i) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(src.at(i), src.at(i + 1));
        }

        for (int i = 0U; i < bitwidth; ++i) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(src.at(i), dest.at(i));
        }

        return true;
    }

    bool SyrecSynthesis::lessEquals(unsigned dest, const std::vector<unsigned>& src2, const std::vector<unsigned>& src1) {
        if (!lessThan(dest, src1, src2)) {
            return false;
        }
        ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(dest);

        return true;
    }

    bool SyrecSynthesis::lessThan(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        return (decreaseWithCarry(src1, src2, dest) && increase(src1, src2));
    }

    bool SyrecSynthesis::modulo(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        std::vector<unsigned> sum;
        std::vector<unsigned> partial;

        for (unsigned i = 1U; i < src1.size(); ++i) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(src2.at(i));
        }

        for (unsigned i = 1U; i < src1.size(); ++i) {
            addActiveControl(src2.at(i));
        }

        for (int i = static_cast<int>(src1.size()) - 1; i >= 0; --i) {
            partial.push_back(src2.at(src1.size() - 1U - i));
            sum.insert(sum.begin(), src1.at(i));
            decreaseWithCarry(sum, partial, dest.at(i));
            addActiveControl(dest.at(i));
            increase(sum, partial);
            removeActiveControl(dest.at(i));
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(dest.at(i));
            if (i > 0) {
                for (unsigned j = (static_cast<int>(src1.size()) - i); j < src1.size(); ++j) {
                    removeActiveControl(src2.at(j));
                }
                ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(src2.at(src1.size() - i));
                for (unsigned j = (static_cast<int>(src1.size()) + 1U - i); j < src1.size(); ++j) {
                    addActiveControl(src2.at(j));
                }
            }
        }
        return true;
    }

    bool SyrecSynthesis::multiplication(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if ((src1.empty()) || (dest.empty())) {
            return true;
        }

        std::vector<unsigned> sum     = dest;
        std::vector<unsigned> partial = src2;

        bool ok = true;

        addActiveControl(src1.at(0));
        ok = ok && bitwiseCnot(sum, partial);
        removeActiveControl(src1.at(0));

        for (unsigned i = 1; i < dest.size(); ++i) {
            sum.erase(sum.begin());
            partial.pop_back();
            addActiveControl(src1.at(i));
            ok = ok && increase(sum, partial);
            removeActiveControl(src1.at(i));
        }

        return ok;
    }

    bool SyrecSynthesis::notEquals(const unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if (!equals(dest, src1, src2)) {
            return false;
        }
        ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(dest);
        return true;
    }

    void SyrecSynthesis::swap(const std::vector<unsigned>& dest1, const std::vector<unsigned>& dest2) {
        for (unsigned i = 0U; i < dest1.size(); ++i) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendFredkin(dest1.at(i), dest2.at(i));
        }
    }

    //**********************************************************************
    //*****                      Shift Operations                      *****
    //**********************************************************************

    void SyrecSynthesis::leftShift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2) {
        for (unsigned i = 0U; (i + src2) < dest.size(); ++i) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(src1.at(i), dest.at(i + src2));
        }
    }

    void SyrecSynthesis::rightShift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2) {
        for (unsigned i = src2; i < dest.size(); ++i) {
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendCnot(src1.at(i), dest.at(i - src2));
        }
    }

    bool SyrecSynthesis::expressionOpInverse([[maybe_unused]] unsigned op, [[maybe_unused]] const std::vector<unsigned>& expLhs, [[maybe_unused]] const std::vector<unsigned>& expRhs) {
        return true;
    }
    //**********************************************************************
    //*****                     Efficient Controls                     *****
    //**********************************************************************

    void SyrecSynthesis::addActiveControl(unsigned control) {
        // aktuelles Blatt vollendet, zurueck zum parent
        cctMan.current = source(*(in_edges(cctMan.current, cctMan.tree).first), cctMan.tree);

        // child fuer neuen control anlegen
        cct_node child                                       = add_vertex(cctMan.tree);
        get(boost::vertex_name, cctMan.tree)[child].control  = control;
        get(boost::vertex_name, cctMan.tree)[child].controls = get(boost::vertex_name, cctMan.tree)[cctMan.current].controls;
        get(boost::vertex_name, cctMan.tree)[child].controls.insert(control);
        add_edge(cctMan.current, child, cctMan.tree);
        cctMan.current = child;

        // neues Blatt anlegen
        cct_node leaf                                       = add_vertex(cctMan.tree);
        get(boost::vertex_name, cctMan.tree)[leaf].controls = get(boost::vertex_name, cctMan.tree)[cctMan.current].controls;
        get(boost::vertex_name, cctMan.tree)[leaf].circ     = std::make_shared<Circuit>();
        get(boost::vertex_name, cctMan.tree)[leaf].circ->gateAdded.connect(Annotater(*get(boost::vertex_name, cctMan.tree)[leaf].circ, stmts));
        add_edge(cctMan.current, leaf, cctMan.tree);
        cctMan.current = leaf;
    }

    void SyrecSynthesis::removeActiveControl(unsigned control [[maybe_unused]]) {
        // aktuelles Blatt vollendet, zurueck zum parent
        cctMan.current = source(*(in_edges(cctMan.current, cctMan.tree).first), cctMan.tree);

        // aktueller Knoten abgeschlossen, zurueck zum parent
        cctMan.current = source(*(in_edges(cctMan.current, cctMan.tree).first), cctMan.tree);

        // neues Blatt anlegen
        cct_node leaf                                       = add_vertex(cctMan.tree);
        get(boost::vertex_name, cctMan.tree)[leaf].controls = get(boost::vertex_name, cctMan.tree)[cctMan.current].controls;
        get(boost::vertex_name, cctMan.tree)[leaf].circ     = std::make_shared<Circuit>();
        get(boost::vertex_name, cctMan.tree)[leaf].circ->gateAdded.connect(Annotater(*get(boost::vertex_name, cctMan.tree)[leaf].circ, stmts));
        add_edge(cctMan.current, leaf, cctMan.tree);
        cctMan.current = leaf;
    }

    bool SyrecSynthesis::assembleCircuit(const cct_node& current) {
        // leaf
        if (out_edges(current, cctMan.tree).first == out_edges(current, cctMan.tree).second /*get( boost::vertex_name, cctMan.tree )[current].circ.get()->num_gates() > 0u*/) {
            circ.insertCircuit(circ.numGates(), *(get(boost::vertex_name, cctMan.tree)[current].circ), get(boost::vertex_name, cctMan.tree)[current].controls);
            return true;
        }
        // assemble optimized circuits of successors
        for (auto edgeIt = out_edges(current, cctMan.tree).first; edgeIt != out_edges(current, cctMan.tree).second; ++edgeIt) {
            if (!assembleCircuit(target(*edgeIt, cctMan.tree))) {
                return false;
            }
        }
        return true;
    }

    void SyrecSynthesis::getVariables(const VariableAccess::ptr& var, std::vector<unsigned>& lines) {
        unsigned offset = varLines[var->getVar()];

        if (!var->indexes.empty()) {
            // check if it is all numeric_expressions
            unsigned n = static_cast<int>(var->getVar()->dimensions.size()); // dimensions
            if (static_cast<unsigned>(std::count_if(var->indexes.cbegin(), var->indexes.cend(), [&](const auto& p) { return dynamic_cast<NumericExpression*>(p.get()); })) == n) {
                for (unsigned i = 0U; i < n; ++i) {
                    offset += dynamic_cast<NumericExpression*>(var->indexes.at(i).get())->value->evaluate(loopMap) *
                              std::accumulate(var->getVar()->dimensions.begin() + i + 1U, var->getVar()->dimensions.end(), 1U, std::multiplies<>()) *
                              var->getVar()->bitwidth;
                }
            }
        }

        if (var->range) {
            auto [nfirst, nsecond] = *var->range;

            unsigned first  = nfirst->evaluate(loopMap);
            unsigned second = nsecond->evaluate(loopMap);

            if (first < second) {
                for (unsigned i = first; i <= second; ++i) {
                    lines.emplace_back(offset + i);
                }
            } else {
                for (auto i = static_cast<int>(first); i >= static_cast<int>(second); --i) {
                    lines.emplace_back(offset + i);
                }
            }
        } else {
            for (unsigned i = 0U; i < var->getVar()->bitwidth; ++i) {
                lines.emplace_back(offset + i);
            }
        }
    }

    /**
   * Function to access array variables
   *
   * The array variable that corresponds to the given indexes is exchanged (via swap operations) with some given helper lines
   *
   * \param offset is the first line number associated to the array
   * \param dimensions is the dimensions of the array
   * \param indexes is the indexes of the array
   * \param bitwidth is the bitwidth of the variables within the array
   * \param lines is the destination, where
   */
    unsigned SyrecSynthesis::getConstantLine(bool value) {
        unsigned constLine = 0U;

        if (!freeConstLinesMap[value].empty()) {
            constLine = freeConstLinesMap[value].back();
            freeConstLinesMap[value].pop_back();
        } else if (!freeConstLinesMap[!value].empty()) {
            constLine = freeConstLinesMap[!value].back();
            freeConstLinesMap[!value].pop_back();
            ((get(boost::vertex_name, cctMan.tree)[cctMan.current].circ))->appendNot(constLine);
        } else {
            constLine = circ.addLine((std::string("const_") + std::to_string(static_cast<int>(value))), "garbage", value, true);
        }

        return constLine;
    }

    void SyrecSynthesis::getConstantLines(unsigned bitwidth, unsigned value, std::vector<unsigned>& lines) {
        boost::dynamic_bitset<> number(bitwidth, value);

        for (unsigned i = 0U; i < bitwidth; ++i) {
            lines.emplace_back(getConstantLine(number.test(i)));
        }
    }

    void SyrecSynthesis::addVariable(Circuit& circ, const std::vector<unsigned>& dimensions, const Variable::ptr& var,
                                     const constant constant, bool garbage, const std::string& arraystr) {
        if (dimensions.empty()) {
            for (unsigned i = 0U; i < var->bitwidth; ++i) {
                std::string name = var->name + arraystr + "." + std::to_string(i);
                circ.addLine(name, name, constant, garbage);
            }
        } else {
            unsigned              len = dimensions.front();
            std::vector<unsigned> newDimensions(dimensions.begin() + 1U, dimensions.end());

            for (unsigned i = 0U; i < len; ++i) {
                addVariable(circ, newDimensions, var, constant, garbage, arraystr + "[" + std::to_string(i) + "]");
            }
        }
    }

    void SyrecSynthesis::addVariables(Circuit& circVar, const Variable::vec& variables) {
        for (const auto& var: variables) {
            // entry in var lines map
            varLines.try_emplace(var, circVar.getLines());

            // types of constant and garbage
            constant constVar = (var->type == Variable::Out || var->type == Variable::Wire) ? constant(false) : constant();
            bool     garbage  = (var->type == Variable::In || var->type == Variable::Wire);

            addVariable(circVar, var->dimensions, var, constVar, garbage, std::string());
        }
    }

    bool SyrecSynthesis::synthesize(SyrecSynthesis* synthesizer, Circuit& circ, const Program& program, const Properties::ptr& settings, const Properties::ptr& statistics) {
        // Settings parsing
        auto variableNameFormat = get<std::string>(settings, "variable_name_format", "%1$s%3$s.%2$d");
        auto mainModule         = get<std::string>(settings, "main_module", std::string());
        // Run-time measuring
        Timer<PropertiesTimer> t;

        if (statistics) {
            PropertiesTimer rt(statistics);
            t.start(rt);
        }

        // get the main module
        Module::ptr main;

        if (!mainModule.empty()) {
            main = program.findModule(mainModule);
            if (!main) {
                std::cerr << "Program has no module: " << mainModule << std::endl;
                return false;
            }
        } else {
            main = program.findModule("main");
            if (!main) {
                main = program.modules().front();
            }
        }

        // declare as top module
        synthesizer->setMainModule(main);

        // create lines for global variables
        synthesizer->addVariables(circ, main->parameters);
        synthesizer->addVariables(circ, main->variables);

        // synthesize the statements
        const auto& statements = synthesizer->onModule(main);

        if (statistics) {
            t.stop();
        }

        return statements;
    }

} // namespace syrec
