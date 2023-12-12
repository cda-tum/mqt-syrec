#pragma once

#include "algorithms/synthesis/syrec_synthesis.hpp"

namespace syrec {
    class CostAwareSynthesis: public SyrecSynthesis {
    public:
        using SyrecSynthesis::SyrecSynthesis;

        static bool synthesize(Circuit& circ, const Program& program, const Properties::ptr& settings = std::make_shared<Properties>(), const Properties::ptr& statistics = std::make_shared<Properties>());

    protected:
        bool processStatement(const Statement::ptr& statement) override {
            return !SyrecSynthesis::onStatement(statement);
        }

        void assignAdd(bool& status, std::vector<unsigned>& rhs, std::vector<unsigned>& lhs, [[maybe_unused]] const unsigned& op) override {
            status = SyrecSynthesis::increase(rhs, lhs);
        }

        void assignSubtract(bool& status, std::vector<unsigned>& rhs, std::vector<unsigned>& lhs, [[maybe_unused]] const unsigned& op) override {
            status = SyrecSynthesis::decrease(rhs, lhs);
        }

        void assignExor(bool& status, std::vector<unsigned>& lhs, std::vector<unsigned>& rhs, [[maybe_unused]] const unsigned& op) override {
            status = SyrecSynthesis::bitwiseCnot(lhs, rhs);
        }

        void expAdd(const unsigned& bitwidth, std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs, const std::vector<unsigned>& lines) override;
        void expSubtract(const unsigned& bitwidth, std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs, const std::vector<unsigned>& lines) override;

        void expExor(const unsigned& bitwidth, std::vector<unsigned>& lines, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) override;
    };
} // namespace syrec
