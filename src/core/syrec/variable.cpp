#include "core/syrec/expression.hpp"
#include "core/syrec/variable.hpp"

#include <cassert>
#include <optional>
#include <utility>

namespace syrec {

    Variable::Variable(const Type type, std::string name, std::vector<unsigned> dimensions, const unsigned bitwidth):
        type(type),
        name(std::move(name)),
        dimensions(std::move(dimensions)),
        bitwidth(bitwidth) {}

    void Variable::setReference(Variable::ptr updatedReference) {
        reference = std::move(updatedReference);
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
            auto [first, second] = *range;
            /* if both variables are loop variables but have the same name,
           then the bit-width is 1, otherwise we cannot determine it now. */
            if (first->isLoopVariable() && second->isLoopVariable()) {
                if (first->variableName() == second->variableName()) {
                    return 1U;
                }
                assert(false);
            }   
        }
        return var->bitwidth;
    }

} // namespace syrec
