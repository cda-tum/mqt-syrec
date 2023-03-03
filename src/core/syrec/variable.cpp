#include "core/syrec/variable.hpp"

#include "core/syrec/expression.hpp"

#include <cassert>
#include <optional>
#include <tuple>
#include <utility>

namespace syrec {

    Variable::Variable(unsigned type, std::string name, std::vector<unsigned> dimensions, unsigned bitwidth):
        type(type),
        name(std::move(name)),
        dimensions(std::move(dimensions)),
        bitwidth(bitwidth) {}

    void Variable::setReference(Variable::ptr ref) {
        reference = std::move(ref);
    }

    void VariableAccess::setVar(Variable::ptr v) {
        var = std::move(v);
    }

    Variable::ptr VariableAccess::getVar() const {
        if (var->reference) {
            return var->reference;
        }
        return var;
    }

    unsigned VariableAccess::bitwidth() const {
        if (range) {
            Number::ptr first;
            Number::ptr second;
            std::tie(first, second) = *range;

            /* if both variables are loop variables but have the same name,
           then the bit-width is 1, otherwise we cannot determine it now. */
            if (first->isLoopVariable() && second->isLoopVariable()) {
                if (first->variableName() == second->variableName()) {
                    return 1U;
                }
                assert(false);
            }

            Number::loop_variable_mapping map; // empty map
            return static_cast<unsigned>(abs(static_cast<int>(first->evaluate(map) - second->evaluate(map)))) + 1U;
        }
        return var->bitwidth;
    }

} // namespace syrec
