#ifndef CORE_SYREC_PARSER_UTILS_SYREC_OPERATION_UTILS_HPP
#define CORE_SYREC_PARSER_UTILS_SYREC_OPERATION_UTILS_HPP

#include <core/syrec/expression.hpp>

namespace utils {
    [[nodiscard]] inline unsigned int truncateConstantValueToExpectedBitwidth(unsigned int valueToTruncate, unsigned int expectedResultBitwidth) {
        if (!expectedResultBitwidth)
            return 0;

        return expectedResultBitwidth < 32 && valueToTruncate > (1 << expectedResultBitwidth)
            ? valueToTruncate % (1 << expectedResultBitwidth)
            : valueToTruncate;
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
        if (lOperand.has_value() && isOperandIdentityElementOfOperation(*lOperand, binaryOperation))
            evaluationResult = rOperand;
        else if (rOperand.has_value() && isOperandIdentityElementOfOperation(*rOperand, binaryOperation))
            evaluationResult = lOperand;
        else if (lOperand.has_value() && rOperand.has_value()) {
            const unsigned int truncatedLOperandValue = truncateConstantValueToExpectedBitwidth(*lOperand, bitwidthOfResult);
            const unsigned int truncatedROperandValue = truncateConstantValueToExpectedBitwidth(*rOperand, bitwidthOfResult);
            switch (binaryOperation) {
                case syrec::BinaryExpression::BinaryOperation::Add:
                    evaluationResult = truncatedLOperandValue + truncatedROperandValue;
                    break;
                case syrec::BinaryExpression::BinaryOperation::Subtract:
                    evaluationResult = truncatedLOperandValue - truncatedROperandValue;
                    break;
                case syrec::BinaryExpression::BinaryOperation::Multiply:
                    evaluationResult = truncatedLOperandValue * truncatedROperandValue;
                    break;
                case syrec::BinaryExpression::BinaryOperation::Divide:
                    if (!truncatedROperandValue)
                        return std::nullopt;

                    evaluationResult = truncatedLOperandValue / truncatedROperandValue;
                    break;
                case syrec::BinaryExpression::BinaryOperation::FracDivide:
                    if (!truncatedROperandValue)
                        return std::nullopt;

                    // The sizeof the unsigned int type does not necessarily need to be equal to 32 bits (https://en.cppreference.com/w/cpp/language/types), thus a case distinction is needed to calculate the correct value
                    // based on the actual size of the data type.
                    if constexpr (sizeof(unsigned int) == 2)
                        evaluationResult = (static_cast<unsigned long>(truncatedLOperandValue) / static_cast<unsigned long>(truncatedROperandValue) >> 16);
                    else
                        evaluationResult = (static_cast<unsigned long long>(truncatedLOperandValue) / static_cast<unsigned long long>(truncatedROperandValue) >> 32);
                    break;
                case syrec::BinaryExpression::BinaryOperation::Modulo:
                    if (!truncatedROperandValue)
                        return std::nullopt;

                    evaluationResult = truncatedLOperandValue % truncatedROperandValue;
                    break;
                case syrec::BinaryExpression::BinaryOperation::Equals:
                    evaluationResult = truncatedLOperandValue == truncatedROperandValue;
                    break;
                case syrec::BinaryExpression::BinaryOperation::NotEquals:
                    evaluationResult = truncatedLOperandValue != truncatedROperandValue;
                    break;
                case syrec::BinaryExpression::BinaryOperation::GreaterEquals:
                    evaluationResult = truncatedLOperandValue >= truncatedROperandValue;
                    break;
                case syrec::BinaryExpression::BinaryOperation::GreaterThan:
                    evaluationResult = truncatedLOperandValue > truncatedROperandValue;
                    break;
                case syrec::BinaryExpression::BinaryOperation::LessEquals:
                    evaluationResult = truncatedLOperandValue <= truncatedROperandValue;
                    break;
                case syrec::BinaryExpression::BinaryOperation::LessThan:
                    evaluationResult = truncatedLOperandValue < truncatedROperandValue;
                    break;
                case syrec::BinaryExpression::BinaryOperation::Exor:
                    evaluationResult = truncatedLOperandValue ^ truncatedROperandValue;
                    break;
                case syrec::BinaryExpression::BinaryOperation::BitwiseAnd:
                    evaluationResult = truncatedLOperandValue & truncatedROperandValue;
                    break;
                case syrec::BinaryExpression::BinaryOperation::BitwiseOr:
                    evaluationResult = truncatedLOperandValue | truncatedROperandValue;
                    break;
                case syrec::BinaryExpression::BinaryOperation::LogicalAnd:
                    evaluationResult = truncatedLOperandValue && truncatedROperandValue;
                    break;
                case syrec::BinaryExpression::BinaryOperation::LogicalOr:
                    evaluationResult = truncatedLOperandValue || truncatedROperandValue;
                    break;
                default:
                    break;
            }
        }
        return evaluationResult.has_value() ? std::make_optional(truncateConstantValueToExpectedBitwidth(*evaluationResult, bitwidthOfResult)) : std::nullopt;
    }

    [[nodiscard]] inline std::optional<unsigned int> tryEvaluate(const std::optional<unsigned int> toBeShiftedValue, syrec::ShiftExpression::ShiftOperation shiftOperation, const std::optional<unsigned int> shiftAmount, unsigned int bitwidthOfResult) {
        if (!bitwidthOfResult)
            return std::nullopt;

        if (shiftAmount.has_value() && !*shiftAmount)
            return toBeShiftedValue.has_value() ? std::make_optional(truncateConstantValueToExpectedBitwidth(*toBeShiftedValue, bitwidthOfResult)) : std::nullopt;
        if (toBeShiftedValue.has_value() && shiftAmount.has_value()) {
            const unsigned int truncatedToBeShiftedValue = truncateConstantValueToExpectedBitwidth(*toBeShiftedValue, bitwidthOfResult);
            const unsigned int truncatedShiftAmount      = truncatedToBeShiftedValue ? truncateConstantValueToExpectedBitwidth(*shiftAmount, bitwidthOfResult) : 0;

            return shiftOperation == syrec::ShiftExpression::ShiftOperation::Left
                ? truncateConstantValueToExpectedBitwidth(truncatedToBeShiftedValue << truncatedShiftAmount, bitwidthOfResult)
                : truncateConstantValueToExpectedBitwidth(truncatedToBeShiftedValue >> truncatedShiftAmount, bitwidthOfResult);   
        }
        return std::nullopt;
    }
} // namespace utils
#endif