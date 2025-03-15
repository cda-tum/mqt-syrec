#pragma once

#include "core/syrec/expression.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace utils {
    class IfStatementExpressionComponentsRecorder {
    public:
        using ptr = std::shared_ptr<IfStatementExpressionComponentsRecorder>;
        enum class ExpressionBracketKind {
            Opening,
            Closing
        };

        enum class VariableAccessComponent {
            BitrangeStart,
            BitrangeEnd,
            DimensionAccessExpressionStart,
            DimensionAccessExpressionEnd
        };

        enum class OperationMode {
            Recording,
            Comparing
        };
        using ExpressionComponent = std::variant<std::string, unsigned int, syrec::BinaryExpression::BinaryOperation, syrec::ShiftExpression::ShiftOperation, ExpressionBracketKind, VariableAccessComponent>;

        void                              recordExpressionComponent(const ExpressionComponent& expressionComponent);
        void                              switchMode(OperationMode newOperationMode);
        [[nodiscard]] std::optional<bool> recordedMatchingExpressionComponents() const;
        [[nodiscard]] OperationMode       getCurrentOperationMode() const;

    protected:
        OperationMode                    operationMode = OperationMode::Recording;
        std::vector<ExpressionComponent> expressionComponents;
        std::size_t                      indexForComparisonOfExpressionComponents = 0;
        std::optional<bool>              aggregateOfComparisonResultsOfExpressionComponents;
    };
} // namespace utils
