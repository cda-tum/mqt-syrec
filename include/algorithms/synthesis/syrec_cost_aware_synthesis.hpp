/*
 * Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
 * Copyright (c) 2025 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#pragma once

#include "algorithms/synthesis/syrec_synthesis.hpp"
#include "core/circuit.hpp"
#include "core/properties.hpp"
#include "core/syrec/program.hpp"
#include "core/syrec/statement.hpp"

#include <memory>
#include <vector>

namespace syrec {
    class CostAwareSynthesis: public SyrecSynthesis {
    public:
        using SyrecSynthesis::SyrecSynthesis;

        static bool synthesize(Circuit& circ, const Program& program, const Properties::ptr& settings = std::make_shared<Properties>(), const Properties::ptr& statistics = std::make_shared<Properties>());

    protected:
        bool processStatement(const Statement::ptr& statement) override {
            return !SyrecSynthesis::onStatement(statement);
        }

        void assignAdd(bool& status, std::vector<unsigned>& rhs, std::vector<unsigned>& lhs, [[maybe_unused]] const AssignStatement::AssignOperation& op) override {
            status = SyrecSynthesis::increase(rhs, lhs);
        }

        void assignSubtract(bool& status, std::vector<unsigned>& rhs, std::vector<unsigned>& lhs, [[maybe_unused]] const AssignStatement::AssignOperation& op) override {
            status = SyrecSynthesis::decrease(rhs, lhs);
        }

        void assignExor(bool& status, std::vector<unsigned>& lhs, std::vector<unsigned>& rhs, [[maybe_unused]] const AssignStatement::AssignOperation& op) override {
            status = SyrecSynthesis::bitwiseCnot(lhs, rhs);
        }

        void expAdd(const unsigned& bitwidth, std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs, const std::vector<unsigned>& lines) override;
        void expSubtract(const unsigned& bitwidth, std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs, const std::vector<unsigned>& lines) override;

        void expExor(const unsigned& bitwidth, std::vector<unsigned>& lines, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) override;
    };
} // namespace syrec
