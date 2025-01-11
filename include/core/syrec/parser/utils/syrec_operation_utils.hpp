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

    [[nodiscard]] bool isOperandIdentityElementOfOperation(const unsigned int operandValue, syrec::BinaryExpression::BinaryOperation binaryOperation) {
        return false;
    }

    [[nodiscard]] std::optional<unsigned int> tryEvaluate(const std::optional<unsigned int> lOperand, syrec::BinaryExpression::BinaryOperation binaryOperation, const std::optional<unsigned int> rOperand, unsigned int bitwidthOfResult) {
        return std::nullopt;
    }

    [[nodiscard]] std::optional<unsigned int> tryEvaluate(const std::optional<unsigned int> toBeShiftedValue, syrec::ShiftExpression::ShiftOperation shiftOperation, const std::optional<unsigned int> shiftAmount, unsigned int bitwidthOfResult) {
        return std::nullopt;
    }
} // namespace utils
#endif