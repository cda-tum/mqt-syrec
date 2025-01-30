#ifndef CORE_SYREC_PARSER_UTILS_SYREC_OPERATION_UTILS_HPP
#define CORE_SYREC_PARSER_UTILS_SYREC_OPERATION_UTILS_HPP

#include <core/syrec/expression.hpp>

namespace utils {
    [[nodiscard]] inline unsigned int truncateConstantValueToExpectedBitwidth(unsigned int valueToTruncate, unsigned int expectedResultBitwidth) {
        /*if (!expectedResultBitwidth)
            return 0;

        return expectedResultBitwidth < 32 && valueToTruncate > (1 << expectedResultBitwidth)
            ? valueToTruncate % (1 << expectedResultBitwidth)
            : valueToTruncate;*/
        return valueToTruncate;
    }

    [[nodiscard]] inline bool isOperandIdentityElementOfOperation(const unsigned int operandValue, syrec::BinaryExpression::BinaryOperation binaryOperation) {
        switch (binaryOperation) {
            case syrec::BinaryExpression::BinaryOperation::Add:
            case syrec::BinaryExpression::BinaryOperation::Exor:
            case syrec::BinaryExpression::BinaryOperation::BitwiseOr:
            case syrec::BinaryExpression::BinaryOperation::LogicalOr:
            case syrec::BinaryExpression::BinaryOperation::Subtract:
                return !operandValue;
            case syrec::BinaryExpression::BinaryOperation::Divide:
            case syrec::BinaryExpression::BinaryOperation::Multiply:
            case syrec::BinaryExpression::BinaryOperation::Modulo:
            case syrec::BinaryExpression::BinaryOperation::LogicalAnd:
                return operandValue == 1;
            default:
                return false;
        }    
    }

    [[nodiscard]] inline std::optional<unsigned int> tryEvaluate(const std::optional<unsigned int> lOperand, syrec::BinaryExpression::BinaryOperation binaryOperation, const std::optional<unsigned int> rOperand, unsigned int bitwidthOfResult) {
        if (!bitwidthOfResult)
            return std::nullopt;

        std::optional<unsigned int> evaluationResult;
        if (lOperand.has_value() && rOperand.has_value()) {
            const unsigned int constantValueOfLOperand = *lOperand;
            const unsigned int constantValueOfROperand = *rOperand;
            switch (binaryOperation) {
                case syrec::BinaryExpression::BinaryOperation::Add:
                    evaluationResult = constantValueOfLOperand + constantValueOfROperand;
                    break;
                case syrec::BinaryExpression::BinaryOperation::Subtract:
                    evaluationResult = constantValueOfLOperand - constantValueOfROperand;
                    break;
                case syrec::BinaryExpression::BinaryOperation::Multiply:
                    evaluationResult = constantValueOfLOperand * constantValueOfROperand;
                    break;
                case syrec::BinaryExpression::BinaryOperation::Divide:
                    if (!constantValueOfROperand)
                        return std::nullopt;

                    evaluationResult = constantValueOfLOperand / constantValueOfROperand;
                    break;
                case syrec::BinaryExpression::BinaryOperation::FracDivide:
                    if (!constantValueOfROperand)
                        return std::nullopt;

                    // The sizeof the unsigned int type does not necessarily need to be equal to 32 bits (https://en.cppreference.com/w/cpp/language/types), thus a case distinction is needed to calculate the correct value
                    // based on the actual size of the data type.
                    if constexpr (sizeof(unsigned int) == 2)
                        evaluationResult = (static_cast<unsigned long>(constantValueOfLOperand) * static_cast<unsigned long>(constantValueOfROperand)) >> 16;
                    else
                        evaluationResult = (static_cast<unsigned long long>(constantValueOfLOperand) * static_cast<unsigned long long>(constantValueOfROperand)) >> 32;
                    break;
                case syrec::BinaryExpression::BinaryOperation::Modulo:
                    if (!constantValueOfROperand)
                        return std::nullopt;

                    evaluationResult = constantValueOfLOperand % constantValueOfROperand;
                    break;
                case syrec::BinaryExpression::BinaryOperation::Equals:
                    evaluationResult = constantValueOfLOperand == constantValueOfROperand;
                    break;
                case syrec::BinaryExpression::BinaryOperation::NotEquals:
                    evaluationResult = constantValueOfLOperand != constantValueOfROperand;
                    break;
                case syrec::BinaryExpression::BinaryOperation::GreaterEquals:
                    evaluationResult = constantValueOfLOperand >= constantValueOfROperand;
                    break;
                case syrec::BinaryExpression::BinaryOperation::GreaterThan:
                    evaluationResult = constantValueOfLOperand > constantValueOfROperand;
                    break;
                case syrec::BinaryExpression::BinaryOperation::LessEquals:
                    evaluationResult = constantValueOfLOperand <= constantValueOfROperand;
                    break;
                case syrec::BinaryExpression::BinaryOperation::LessThan:
                    evaluationResult = constantValueOfLOperand < constantValueOfROperand;
                    break;
                case syrec::BinaryExpression::BinaryOperation::Exor:
                    evaluationResult = constantValueOfLOperand ^ constantValueOfROperand;
                    break;
                case syrec::BinaryExpression::BinaryOperation::BitwiseAnd:
                    evaluationResult = constantValueOfLOperand & constantValueOfROperand;
                    break;
                case syrec::BinaryExpression::BinaryOperation::BitwiseOr:
                    evaluationResult = constantValueOfLOperand | constantValueOfROperand;
                    break;
                case syrec::BinaryExpression::BinaryOperation::LogicalAnd:
                    evaluationResult = constantValueOfLOperand > 0 && constantValueOfROperand > 0;
                    break;
                case syrec::BinaryExpression::BinaryOperation::LogicalOr:
                    evaluationResult = constantValueOfLOperand > 0 || constantValueOfROperand > 0;
                    break;
                default:
                    break;
            }
        }
        else if (lOperand.has_value() && isOperandIdentityElementOfOperation(*lOperand, binaryOperation))
            evaluationResult = rOperand;
        else if (rOperand.has_value() && isOperandIdentityElementOfOperation(*rOperand, binaryOperation))
            evaluationResult = lOperand;
        return evaluationResult.has_value() ? std::make_optional(truncateConstantValueToExpectedBitwidth(*evaluationResult, bitwidthOfResult)) : std::nullopt;
    }

    [[nodiscard]] inline std::optional<unsigned int> tryEvaluate(const std::optional<unsigned int> toBeShiftedValue, syrec::ShiftExpression::ShiftOperation shiftOperation, const std::optional<unsigned int> shiftAmount, unsigned int bitwidthOfResult) {
        if (!bitwidthOfResult)
            return std::nullopt;

        if (shiftAmount.has_value() && !*shiftAmount)
            return toBeShiftedValue.has_value() ? std::make_optional(truncateConstantValueToExpectedBitwidth(*toBeShiftedValue, bitwidthOfResult)) : std::nullopt;
        if (toBeShiftedValue.has_value() && shiftAmount.has_value()) {
            if (!*toBeShiftedValue)
                return 0;

            return shiftOperation == syrec::ShiftExpression::ShiftOperation::Left
                ? truncateConstantValueToExpectedBitwidth(*toBeShiftedValue << *toBeShiftedValue, bitwidthOfResult)
                : truncateConstantValueToExpectedBitwidth(*toBeShiftedValue >> *toBeShiftedValue, bitwidthOfResult);   
        }
        return std::nullopt;
    }
} // namespace utils
#endif