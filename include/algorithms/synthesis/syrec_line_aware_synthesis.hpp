#pragma once

#include "algorithms/synthesis/syrec_synthesis.hpp"

namespace syrec {
    class LineAwareSynthesis: public SyrecSynthesis {
    public:
        using SyrecSynthesis::SyrecSynthesis;

        static bool synthesize(Circuit& circ, const Program& program, const Properties::ptr& settings = std::make_shared<Properties>(), const Properties::ptr& statistics = std::make_shared<Properties>());

    protected:
        bool processStatement(const Statement::ptr& statement) override {
            return !fullStatement(statement) && !SyrecSynthesis::onStatement(statement);
        }

        bool fullStatement(const Statement::ptr& statement);
        bool fullStatement(const AssignStatement& statement);

        bool opRhsLhsExpression(const Expression::ptr& expression, std::vector<unsigned>& v) override;

        bool opRhsLhsExpression(const VariableExpression& expression, std::vector<unsigned>& v) override;

        bool opRhsLhsExpression(const BinaryExpression& expression, std::vector<unsigned>& v) override;

        void popExp();

        void inverse();

        void assignAdd(bool& status, std::vector<unsigned>& rhs, std::vector<unsigned>& lhs, const AssignStatement::AssignOperation& op) override;

        void assignSubtract(bool& status, std::vector<unsigned>& rhs, std::vector<unsigned>& lhs, const AssignStatement::AssignOperation& op) override;

        void assignExor(bool& status, std::vector<unsigned>& lhs, std::vector<unsigned>& rhs, const AssignStatement::AssignOperation& op) override;

        bool solver(const std::vector<unsigned>& expRhs, AssignStatement::AssignOperation statOp, const std::vector<unsigned>& expLhs, BinaryExpression::BinaryOperation expOp, const std::vector<unsigned>& statLhs);

        bool flow(const Expression::ptr& expression, std::vector<unsigned>& v);
        bool flow(const VariableExpression& expression, std::vector<unsigned>& v);
        bool flow(const BinaryExpression& expression, const std::vector<unsigned>& v);

        void expAdd([[maybe_unused]] const unsigned& bitwidth, std::vector<unsigned>& lines, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) override {
            SyrecSynthesis::increase(rhs, lhs);
            lines = rhs;
        }

        void expSubtract([[maybe_unused]] const unsigned& bitwidth, std::vector<unsigned>& lines, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) override {
            decreaseNewAssign(rhs, lhs);
            lines = rhs;
        }

        void expExor([[maybe_unused]] const unsigned& bitwidth, std::vector<unsigned>& lines, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) override {
            bitwiseCnot(rhs, lhs); // duplicate lhs
            lines = rhs;
        }

        bool expEvaluate(std::vector<unsigned>& lines, BinaryExpression::BinaryOperation op, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs);

        bool expressionSingleOp(BinaryExpression::BinaryOperation op, const std::vector<unsigned>& expLhs, const std::vector<unsigned>& expRhs);

        bool decreaseNewAssign(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs);

        bool expressionOpInverse(BinaryExpression::BinaryOperation op, const std::vector<unsigned>& expLhs, const std::vector<unsigned>& expRhs) override;

        [[nodiscard]] static std::optional<AssignStatement::AssignOperation> tryMapBinaryToAssignmentOperation(BinaryExpression::BinaryOperation binaryOperation) noexcept;
        [[nodiscard]] static std::optional<BinaryExpression::BinaryOperation> tryMapAssignmentToBinaryOperation(AssignStatement::AssignOperation assignOperation) noexcept;

    };
} // namespace syrec
