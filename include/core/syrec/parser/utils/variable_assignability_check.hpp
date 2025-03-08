#ifndef CORE_SYREC_PARSER_UTILS_VARIABLE_ASSIGNABILITY_CHECK_HPP
#define CORE_SYREC_PARSER_UTILS_VARIABLE_ASSIGNABILITY_CHECK_HPP
#pragma once

#include "core/syrec/variable.hpp"

namespace variable_assignability_check {
    [[nodiscard]] constexpr static bool doesModuleParameterTypeAllowAssignmentFromVariableType(syrec::Variable::Type assignedToVariableType, syrec::Variable::Type assignedFromVariableType) {
        switch (assignedToVariableType) {
            case syrec::Variable::Type::In:
                return true;
            case syrec::Variable::Type::Out:
            case syrec::Variable::Type::Inout:
                return assignedFromVariableType == syrec::Variable::Type::Out || assignedFromVariableType == syrec::Variable::Type::Inout || assignedFromVariableType == syrec::Variable::Type::Wire;
            default:
                return false;
        }
    }
} // namespace variable_assignability_check
#endif