#ifndef CORE_SYREC_PARSER_UTILS_IF_STATEMENT_EXPRESSION_COMPONENTS_RECORDER_HPP
#define CORE_SYREC_PARSER_UTILS_IF_STATEMENT_EXPRESSION_COMPONENTS_RECORDER_HPP

#include "core/syrec/expression.hpp"
#include "core/syrec/variable.hpp"

#include <variant>

namespace utils {
    class IfStatementExpressionComponentsRecorder {
    public:
        using ptr = std::shared_ptr<IfStatementExpressionComponentsRecorder>;
        enum class ExpressionBracketKind {
            Opening,
            Closing
        };

        enum class OperationMode {
            Recording,
            Comparing,
            Ignoring
        };
        using ExpressionComponent = std::variant<std::string, syrec::VariableAccess::ptr, unsigned int, syrec::BinaryExpression::BinaryOperation, syrec::ShiftExpression::ShiftOperation, ExpressionBracketKind>;

        IfStatementExpressionComponentsRecorder():
            operationMode(OperationMode::Recording), indexForComparisonOfExpressionComponents(0) {}

        void                              recordExpressionComponent(const ExpressionComponent& expressionComponent);
        void                              switchMode(OperationMode newOperationMode);
        [[nodiscard]] std::optional<bool> recordedMatchingExpressionComponents() const;
        [[nodiscard]] OperationMode       getCurrentOperationMode() const;

    protected:
        OperationMode                    operationMode;
        std::vector<ExpressionComponent> expressionComponents;
        std::size_t                      indexForComparisonOfExpressionComponents;
        std::optional<bool>              aggregateOfComparisonResultsOfExpressionComponents;
    };
} // namespace utils
#endif