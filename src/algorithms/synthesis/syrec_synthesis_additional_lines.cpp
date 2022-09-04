#include "algorithms/synthesis/syrec_synthesis_additional_lines.hpp"

namespace syrec {
    bool SyrecSynthesisAdditionalLines::on_statement(const assign_statement& statement) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;
        get_variables(statement.lhs, lhs);
        SyrecSynthesis::on_expression(statement.rhs, rhs, lhs, statement.op);

        assert(lhs.size() == rhs.size());

        bool status = false;

        switch (statement.op) {
            case assign_statement::add: {
                status = increase(lhs, rhs);
            } break;

            case assign_statement::subtract: {
                status = decrease(lhs, rhs);
            } break;

            case assign_statement::exor: {
                status = bitwise_cnot(lhs, rhs);
            } break;

            default:
                return false;
        }
        return status;
    }

    bool SyrecSynthesisAdditionalLines::on_expression(const binary_expression& expression, std::vector<unsigned>& lines, std::vector<unsigned> const& lhs_stat, unsigned op) {
        std::vector<unsigned> lhs;
        std::vector<unsigned> rhs;

        if (!SyrecSynthesis::on_expression(expression.lhs, lhs, lhs_stat, op) || !SyrecSynthesis::on_expression(expression.rhs, rhs, lhs_stat, op)) {
            return false;
        }

        switch (expression.op) {
            case binary_expression::add: // +
                get_constant_lines(expression.bitwidth(), 0u, lines);
                bitwise_cnot(lines, lhs); // duplicate lhs
                increase(lines, rhs);
                break;
            case binary_expression::subtract: // -
                get_constant_lines(expression.bitwidth(), 0u, lines);
                bitwise_cnot(lines, lhs); // duplicate lhs
                decrease(lines, rhs);
                break;
            case binary_expression::exor: // ^
                get_constant_lines(expression.bitwidth(), 0u, lines);
                bitwise_cnot(lines, lhs); // duplicate lhs
                bitwise_cnot(lines, rhs);
                break;
            case binary_expression::multiply: // *
                get_constant_lines(expression.bitwidth(), 0u, lines);
                multiplication(lines, lhs, rhs);
                break;
            case binary_expression::divide: // /
                get_constant_lines(expression.bitwidth(), 0u, lines);
                division(lines, lhs, rhs);
                break;
            case binary_expression::modulo: { // %
                get_constant_lines(expression.bitwidth(), 0u, lines);
                std::vector<unsigned> quot;
                get_constant_lines(expression.bitwidth(), 0u, quot);

                bitwise_cnot(lines, lhs); // duplicate lhs
                modulo(quot, lines, rhs);
            } break;
            case binary_expression::logical_and: // &&
                lines.emplace_back(get_constant_line(false));
                conjunction(lines.at(0), lhs.at(0), rhs.at(0));
                break;
            case binary_expression::logical_or: // ||
                lines.emplace_back(get_constant_line(false));
                disjunction(lines.at(0), lhs.at(0), rhs.at(0));
                break;
            case binary_expression::bitwise_and: // &
                get_constant_lines(expression.bitwidth(), 0u, lines);
                bitwise_and(lines, lhs, rhs);
                break;
            case binary_expression::bitwise_or: // |
                get_constant_lines(expression.bitwidth(), 0u, lines);
                bitwise_or(lines, lhs, rhs);
                break;
            case binary_expression::less_than: // <
                lines.emplace_back(get_constant_line(false));
                less_than(lines.at(0), lhs, rhs);
                break;
            case binary_expression::greater_than: // >
                lines.emplace_back(get_constant_line(false));
                greater_than(lines.at(0), lhs, rhs);
                break;
            case binary_expression::equals: // =
                lines.emplace_back(get_constant_line(false));
                equals(lines.at(0), lhs, rhs);
                break;
            case binary_expression::not_equals: // !=
                lines.emplace_back(get_constant_line(false));
                not_equals(lines.at(0), lhs, rhs);
                break;
            case binary_expression::less_equals: // <=
                lines.emplace_back(get_constant_line(false));
                less_equals(lines.at(0), lhs, rhs);
                break;
            case binary_expression::greater_equals: // >=
                lines.emplace_back(get_constant_line(false));
                greater_equals(lines.at(0), lhs, rhs);
                break;
            default:
                return false;
        }

        return true;
    }

    bool SyrecSynthesisAdditionalLines::synthesize(circuit& circ, const program& program, const properties::ptr& settings, const properties::ptr& statistics) {
        SyrecSynthesisAdditionalLines synthesizer(circ);
        return SyrecSynthesis::synthesize(&synthesizer, circ, program, settings, statistics);
    }
} // namespace syrec
