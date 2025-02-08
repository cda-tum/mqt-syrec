#include <core/syrec/parser/utils/if_statement_expression_components_recorder.hpp>

using namespace utils;

// We currently have no other choice than to split the function header and its implementation into two separate but same anonymous namespace
// if we are planning to use said function before its implementation was provided in an anonymous namespace.
namespace {
    std::optional<bool> doExpressionsMatch(const syrec::Expression::ptr& lExpr, const syrec::Expression::ptr& rExpr);
}

namespace {
    std::optional<bool> doVariableInstancesMatch(const syrec::Variable::ptr& lVar, const syrec::Variable::ptr& rVar) {
        if (lVar == nullptr ^ rVar == nullptr)
            return false;
        if (!lVar)
            return std::nullopt;

        return lVar->name == rVar->name && lVar->type == rVar->type && lVar->bitwidth == rVar->bitwidth && lVar->dimensions.size() == rVar->dimensions.size() && std::equal(lVar->dimensions.cbegin(), lVar->dimensions.cend(), rVar->dimensions.cbegin(), rVar->dimensions.cend());
    }

    std::optional<bool> doExpressionComponentsMatch(const syrec::Number::ptr& lNumber, const syrec::Number::ptr& rNumber) {
        if (lNumber == nullptr ^ rNumber == nullptr)
            return false;
        if (!lNumber)
            return std::nullopt;

        if (lNumber->isConstant())
            return rNumber->isConstant() && lNumber->evaluate({}) == rNumber->evaluate({});
        if (lNumber->isLoopVariable())
            return rNumber->isLoopVariable() && lNumber->variableName() == rNumber->variableName();
        if (lNumber->isConstantExpression()) {
            const std::optional<syrec::Number::ConstantExpression>& lConstantExpression = lNumber->constantExpression();
            const std::optional<syrec::Number::ConstantExpression>& rConstantExpression = rNumber->constantExpression();
            if (lConstantExpression.has_value() != rConstantExpression.has_value())
                return false;
            if (!lConstantExpression.has_value())
                return std::nullopt;

            const std::optional<bool> doesLhsOperandMatch = doExpressionComponentsMatch(lConstantExpression->lhsOperand, rConstantExpression->lhsOperand);
            if (doesLhsOperandMatch.value_or(false)) {
                if (lConstantExpression->operation != rConstantExpression->operation)
                    return false;
                return doExpressionComponentsMatch(lConstantExpression->rhsOperand, rConstantExpression->rhsOperand);
            }
            return doesLhsOperandMatch;
        }
        return std::nullopt;
    }

    std::optional<bool> doExpressionComponentsMatch(const syrec::VariableAccess::ptr& lVarAccess, const syrec::VariableAccess::ptr& rVarAccess) {
        if (lVarAccess == nullptr ^ rVarAccess == nullptr)
            return false;
        if (!lVarAccess)
            return std::nullopt;

        std::optional<bool> containerForMatchResult = doVariableInstancesMatch(lVarAccess->var, rVarAccess->var);
        if (containerForMatchResult.value_or(false)) {
            if (lVarAccess->range.has_value() == rVarAccess->range.has_value()) {
                if (lVarAccess->range.has_value()) {
                    containerForMatchResult = doExpressionComponentsMatch(lVarAccess->range->first, rVarAccess->range->first);
                    if (containerForMatchResult.value_or(false)) {
                        containerForMatchResult = doExpressionComponentsMatch(lVarAccess->range->second, rVarAccess->range->second);
                    }
                }
            } else {
                containerForMatchResult = !lVarAccess->range.has_value() && !rVarAccess->range.has_value();
            }

            if (containerForMatchResult.value_or(false)) {
                if (lVarAccess->indexes.size() != rVarAccess->indexes.size())
                    return false;

                for (std::size_t i = 0; i < lVarAccess->indexes.size() && containerForMatchResult.value_or(false); ++i) {
                    containerForMatchResult = doExpressionsMatch(lVarAccess->indexes.at(i), rVarAccess->indexes.at(i));
                }
            }
        }
        return containerForMatchResult;
    }

    std::optional<bool> doExpressionsMatch(const syrec::BinaryExpression& lExpr, const syrec::BinaryExpression& rExpr) {
        const std::optional<bool> doLhsOperandsMatch = doExpressionsMatch(lExpr.lhs, rExpr.lhs);
        if (doLhsOperandsMatch.value_or(false)) {
            if (lExpr.binaryOperation != rExpr.binaryOperation)
                return false;
            return doExpressionsMatch(lExpr.rhs, rExpr.rhs);
        }
        return doLhsOperandsMatch;
    }

    std::optional<bool> doExpressionsMatch(const syrec::ShiftExpression& lExpr, const syrec::ShiftExpression& rExpr) {
        const std::optional<bool> doShiftedExpressionsMatch = doExpressionsMatch(lExpr.lhs, rExpr.lhs);
        if (doShiftedExpressionsMatch.value_or(false)) {
            if (lExpr.shiftOperation != rExpr.shiftOperation)
                return false;
            return doExpressionComponentsMatch(lExpr.rhs, rExpr.rhs);
        }
        return doShiftedExpressionsMatch;
    }

    std::optional<bool> doExpressionsMatch(const syrec::NumericExpression& lExpr, const syrec::NumericExpression& rExpr) {
        return doExpressionComponentsMatch(lExpr.value, rExpr.value);
    }

    std::optional<bool> doExpressionsMatch(const syrec::VariableExpression& lExpr, const syrec::VariableExpression& rExpr) {
        return doExpressionComponentsMatch(lExpr.var, rExpr.var);
    }

    std::optional<bool> doExpressionsMatch(const syrec::Expression::ptr& lExpr, const syrec::Expression::ptr& rExpr) {
        if (lExpr == nullptr ^ rExpr == nullptr)
            return false;
        if (!lExpr)
            return std::nullopt;

        bool matcherForExprTypeDefined = true;
        if (const auto& lExprAsBinaryOne = dynamic_cast<const syrec::BinaryExpression*>(&*lExpr)) {
            if (const auto& rExprAsBinaryOne = dynamic_cast<const syrec::BinaryExpression*>(&*rExpr); lExprAsBinaryOne && rExprAsBinaryOne) {
                return doExpressionsMatch(*lExprAsBinaryOne, *rExprAsBinaryOne);
            }
        } else if (const auto& lExprAsShiftOne = dynamic_cast<const syrec::ShiftExpression*>(&*lExpr)) {
            if (const auto& rExprAsShiftOne = dynamic_cast<const syrec::ShiftExpression*>(&*rExpr); lExprAsShiftOne && rExprAsShiftOne) {
                return doExpressionsMatch(*lExprAsShiftOne, *rExprAsShiftOne);
            }
        } else if (const auto& lExprAsVarExpr = dynamic_cast<const syrec::VariableExpression*>(&*lExpr)) {
            if (const auto& rExprAsVarExpr = dynamic_cast<const syrec::VariableExpression*>(&*rExpr); lExprAsVarExpr && rExprAsVarExpr) {
                return doExpressionsMatch(*lExprAsVarExpr, *rExprAsVarExpr);
            }
        } else if (const auto& lExprAsNumericOne = dynamic_cast<const syrec::NumericExpression*>(&*lExpr)) {
            if (const auto& rExprAsNumericOne = dynamic_cast<const syrec::NumericExpression*>(&*rExpr); lExprAsNumericOne && rExprAsNumericOne) {
                return doExpressionsMatch(*lExprAsNumericOne, *rExprAsNumericOne);
            }
        } else {
            matcherForExprTypeDefined = false;
        }
        return !matcherForExprTypeDefined ? std::nullopt : std::make_optional(false);
    }

    std::optional<bool> doExpressionComponentsMatch(const IfStatementExpressionComponentsRecorder::ExpressionComponent& lExprComponent, const IfStatementExpressionComponentsRecorder::ExpressionComponent& rExprComponent) {
        if (lExprComponent.index() != rExprComponent.index())
            return false;

        if (std::holds_alternative<IfStatementExpressionComponentsRecorder::ExpressionBracketKind>(lExprComponent))
            return std::get<IfStatementExpressionComponentsRecorder::ExpressionBracketKind>(lExprComponent) == std::get<IfStatementExpressionComponentsRecorder::ExpressionBracketKind>(rExprComponent);
        if (std::holds_alternative<unsigned int>(lExprComponent))
            return std::get<unsigned int>(lExprComponent) == std::get<unsigned int>(rExprComponent);
        if (std::holds_alternative<std::string>(lExprComponent))
            return std::get<std::string>(lExprComponent) == std::get<std::string>(rExprComponent);
        if (std::holds_alternative<syrec::VariableAccess::ptr>(lExprComponent))
            return doExpressionComponentsMatch(std::get<syrec::VariableAccess::ptr>(lExprComponent), std::get<syrec::VariableAccess::ptr>(rExprComponent));
        if (std::holds_alternative<syrec::BinaryExpression::BinaryOperation>(lExprComponent))
            return std::get<syrec::BinaryExpression::BinaryOperation>(lExprComponent) == std::get<syrec::BinaryExpression::BinaryOperation>(rExprComponent);
        if (std::holds_alternative<syrec::ShiftExpression::ShiftOperation>(lExprComponent))
            return std::get<syrec::ShiftExpression::ShiftOperation>(lExprComponent) == std::get<syrec::ShiftExpression::ShiftOperation>(rExprComponent);

        return std::nullopt;
    }
} // namespace

void IfStatementExpressionComponentsRecorder::recordExpressionComponent(const ExpressionComponent& expressionComponent) {
    switch (operationMode) {
        case OperationMode::Ignoring:
            return;
        case OperationMode::Recording:
            expressionComponents.emplace_back(expressionComponent);
            return;
        case OperationMode::Comparing: {
            if (aggregateOfComparisonResultsOfExpressionComponents.has_value() && !aggregateOfComparisonResultsOfExpressionComponents.value())
                return;

            if (indexForComparisonOfExpressionComponents < expressionComponents.size()) {
                const ExpressionComponent& expectedExpressionComponentAtIndex = expressionComponents.at(indexForComparisonOfExpressionComponents++);
                aggregateOfComparisonResultsOfExpressionComponents            = doExpressionComponentsMatch(expectedExpressionComponentAtIndex, expressionComponent);
            } else {
                aggregateOfComparisonResultsOfExpressionComponents = false;
            }
        }
    }
}

void IfStatementExpressionComponentsRecorder::switchMode(OperationMode newOperationMode) {
    operationMode = newOperationMode;
}

std::optional<bool> IfStatementExpressionComponentsRecorder::recordedMatchingExpressionComponents() const {
    return aggregateOfComparisonResultsOfExpressionComponents;
}

IfStatementExpressionComponentsRecorder::OperationMode IfStatementExpressionComponentsRecorder::getCurrentOperationMode() const {
    return operationMode;
}