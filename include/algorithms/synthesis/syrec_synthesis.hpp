/*
 * Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
 * Copyright (c) 2025 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#pragma once

#include "core/circuit.hpp"
#include "core/gate.hpp"
#include "core/properties.hpp"
#include "core/syrec/expression.hpp"
#include "core/syrec/module.hpp"
#include "core/syrec/number.hpp"
#include "core/syrec/program.hpp"
#include "core/syrec/statement.hpp"
#include "core/syrec/variable.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_selectors.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/pending/property.hpp>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <vector>

namespace syrec::internal {
    struct NodeProperties {
        NodeProperties() = default;

        unsigned                 control{};
        Gate::line_container     controls;
        std::shared_ptr<Circuit> circ;
    };

    using cct      = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, boost::property<boost::vertex_name_t, NodeProperties>>;
    using cct_node = boost::graph_traits<cct>::vertex_descriptor;

    struct CctManager {
        cct      tree;
        cct_node current;
        cct_node root;
    };
} // namespace syrec::internal

namespace syrec {
    using namespace internal;

    class SyrecSynthesis {
    public:
        std::stack<unsigned>               expOpp;
        std::stack<std::vector<unsigned>>  expLhss;
        std::stack<std::vector<unsigned>>  expRhss;
        bool                               subFlag = false;
        std::vector<unsigned>              opVec;
        std::vector<unsigned>              assignOpVector;
        std::vector<unsigned>              expOpVector;
        std::vector<std::vector<unsigned>> expLhsVector;
        std::vector<std::vector<unsigned>> expRhsVector;

        using var_lines_map = std::map<Variable::ptr, unsigned int>;

        explicit SyrecSynthesis(Circuit& circ);
        virtual ~SyrecSynthesis() = default;

        void addVariables(Circuit& circ, const Variable::vec& variables);
        void setMainModule(const Module::ptr& mainModule);

    protected:
        virtual bool processStatement(const Statement::ptr& statement) = 0;

        virtual bool onModule(const Module::ptr&);

        virtual bool opRhsLhsExpression([[maybe_unused]] const Expression::ptr& expression, [[maybe_unused]] std::vector<unsigned>& v);
        virtual bool opRhsLhsExpression([[maybe_unused]] const VariableExpression& expression, [[maybe_unused]] std::vector<unsigned>& v);
        virtual bool opRhsLhsExpression([[maybe_unused]] const BinaryExpression& expression, [[maybe_unused]] std::vector<unsigned>& v);

        virtual bool              onStatement(const Statement::ptr& statement);
        virtual bool              onStatement(const AssignStatement& statement);
        virtual bool              onStatement(const IfStatement& statement);
        virtual bool              onStatement(const ForStatement& statement);
        virtual bool              onStatement(const CallStatement& statement);
        virtual bool              onStatement(const UncallStatement& statement);
        bool                      onStatement(const SwapStatement& statement);
        bool                      onStatement(const UnaryStatement& statement);
        [[nodiscard]] static bool onStatement(const SkipStatement& statement);

        virtual void assignAdd(bool& status, std::vector<unsigned>& lhs, std::vector<unsigned>& rhs, [[maybe_unused]] const unsigned& op) = 0;

        virtual void assignSubtract(bool& status, std::vector<unsigned>& lhs, std::vector<unsigned>& rhs, [[maybe_unused]] const unsigned& op) = 0;
        virtual void assignExor(bool& status, std::vector<unsigned>& lhs, std::vector<unsigned>& rhs, [[maybe_unused]] const unsigned& op)     = 0;

        virtual bool onExpression(const Expression::ptr& expression, std::vector<unsigned>& lines, std::vector<unsigned> const& lhsStat, unsigned op);
        virtual bool onExpression(const BinaryExpression& expression, std::vector<unsigned>& lines, std::vector<unsigned> const& lhsStat, unsigned op);
        virtual bool onExpression(const ShiftExpression& expression, std::vector<unsigned>& lines, std::vector<unsigned> const& lhsStat, unsigned op);
        virtual bool onExpression(const NumericExpression& expression, std::vector<unsigned>& lines);
        virtual bool onExpression(const VariableExpression& expression, std::vector<unsigned>& lines);

        virtual void expAdd([[maybe_unused]] const unsigned& bitwidth, std::vector<unsigned>& lines, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) = 0;

        virtual void expSubtract([[maybe_unused]] const unsigned& bitwidth, std::vector<unsigned>& lines, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) = 0;

        virtual void expExor([[maybe_unused]] const unsigned& bitwidth, std::vector<unsigned>& lines, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) = 0;

        // unary operations
        bool bitwiseNegation(const std::vector<unsigned>& dest); // ~
        bool decrement(const std::vector<unsigned>& dest);       // --
        bool increment(const std::vector<unsigned>& dest);       // ++

        // binary operations
        bool         bitwiseAnd(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2); // &
        bool         bitwiseCnot(const std::vector<unsigned>& dest, const std::vector<unsigned>& src);                                    // ^=
        bool         bitwiseOr(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);  // &
        bool         conjunction(unsigned dest, unsigned src1, unsigned src2);                                                            // &&// -=
        bool         decreaseWithCarry(const std::vector<unsigned>& dest, const std::vector<unsigned>& src, unsigned carry);
        bool         disjunction(unsigned dest, unsigned src1, unsigned src2);                                                          // ||
        bool         division(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2); // /
        bool         equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                       // =
        bool         greaterEquals(unsigned dest, const std::vector<unsigned>& srcTwo, const std::vector<unsigned>& srcOne);            // >
        bool         greaterThan(unsigned dest, const std::vector<unsigned>& src2, const std::vector<unsigned>& src1);                  // >// +=
        bool         increaseWithCarry(const std::vector<unsigned>& dest, const std::vector<unsigned>& src, unsigned carry);
        bool         lessEquals(unsigned dest, const std::vector<unsigned>& src2, const std::vector<unsigned>& src1);                         // <=
        bool         lessThan(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                           // <
        bool         modulo(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);         // %
        bool         multiplication(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2); // *
        bool         notEquals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                          // !=
        void         swap(const std::vector<unsigned>& dest1, const std::vector<unsigned>& dest2);                                            // <=>
        bool         decrease(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs);
        bool         increase(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs);
        virtual bool expressionOpInverse([[maybe_unused]] unsigned op, [[maybe_unused]] const std::vector<unsigned>& expLhs, [[maybe_unused]] const std::vector<unsigned>& expRhs);
        bool         checkRepeats();

        // shift operations
        void leftShift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2);  // <<
        void rightShift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2); // >>

        void addActiveControl(unsigned);
        void removeActiveControl(unsigned);

        bool assembleCircuit(const cct_node&);

        CctManager cctMan;

        static void addVariable(Circuit& circ, const std::vector<unsigned>& dimensions, const Variable::ptr& var, constant constant, bool garbage, const std::string& arraystr);
        void        getVariables(const VariableAccess::ptr& var, std::vector<unsigned>& lines);

        unsigned getConstantLine(bool value);
        void     getConstantLines(unsigned bitwidth, unsigned value, std::vector<unsigned>& lines);

        static bool synthesize(SyrecSynthesis* synthesizer, Circuit& circ, const Program& program, const Properties::ptr& settings, const Properties::ptr& statistics);

        std::stack<Statement::ptr>    stmts;
        Circuit&                      circ; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
        Number::loop_variable_mapping loopMap;
        std::stack<Module::ptr>       modules;

    private:
        var_lines_map                         varLines;
        std::map<bool, std::vector<unsigned>> freeConstLinesMap;
    };

} // namespace syrec
