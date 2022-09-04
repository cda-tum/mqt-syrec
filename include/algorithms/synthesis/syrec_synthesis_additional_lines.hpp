#pragma once

#include "algorithms/synthesis/syrec_synthesis.hpp"

namespace syrec {
    class SyrecSynthesisAdditionalLines: public SyrecSynthesis {
    public:
        using SyrecSynthesis::SyrecSynthesis;

        static bool synthesize(circuit& circ, const program& program, const properties::ptr& settings = std::make_shared<properties>(), const properties::ptr& statistics = std::make_shared<properties>());

    protected:
        bool process_statement(const statement::ptr& statement) override {
            return !SyrecSynthesis::on_statement(statement);
        }

        bool on_statement(const assign_statement& statement) override;

        bool on_expression(const binary_expression& expression, std::vector<unsigned>& lines, std::vector<unsigned> const& lhs_stat, unsigned op);
    };
} // namespace syrec
