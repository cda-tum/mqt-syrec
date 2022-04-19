#include "core/syrec/variable.hpp"

#include "core/syrec/expression.hpp"

#include <cassert>
#include <optional>
#include <tuple>
#include <utility>

namespace syrec {

    variable::variable(unsigned type, std::string name, std::vector<unsigned> dimensions, unsigned bitwidth):
        type(type),
        name(std::move(name)),
        dimensions(std::move(dimensions)),
        bitwidth(bitwidth) {}

    void variable::set_reference(variable::ptr ref) {
        reference = std::move(ref);
    }

    void variable_access::set_var(variable::ptr v) {
        var = std::move(v);
    }

    variable::ptr variable_access::get_var() const {
        if (var->reference) {
            return var->reference;
        } else {
            return var;
        }
    }

    unsigned variable_access::bitwidth() const {
        if (range) {
            number::ptr first, second;
            std::tie(first, second) = *range;

            /* if both variables are loop variables but have the same name,
           then the bit-width is 1, otherwise we cannot determine it now. */
            if (first->is_loop_variable() && second->is_loop_variable()) {
                if (first->variable_name() == second->variable_name()) {
                    return 1u;
                } else {
                    assert(false);
                }
            }

            number::loop_variable_mapping map; // empty map
            return abs((int)(first->evaluate(map) - second->evaluate(map))) + 1;
        } else {
            return var->bitwidth;
        }
    }

} // namespace syrec::applications
