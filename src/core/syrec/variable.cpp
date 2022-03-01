/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2010  The RevKit Developers <revkit@informatik.uni-bremen.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "core/syrec/variable.hpp"
#include "core/syrec/expression.hpp"

#include <optional>
#include <tuple>
#include <utility>
#include <cassert>

namespace syrec::applications {

    class variable::priv {
    public:
        priv() = default;

        unsigned              type{};
        std::string           name{};
        unsigned              bitwidth{};
        variable::ptr         reference = nullptr;
        std::vector<unsigned> dimensions{};
    };

    variable::variable():
        d(new priv()) {
    }

    variable::variable(unsigned type, const std::string& name, unsigned bitwidth):
        d(new priv()) {
        d->type     = type;
        d->name     = name;
        d->bitwidth = bitwidth;
    }

    variable::variable(unsigned type, const std::string& name, const std::vector<unsigned>& dimensions, unsigned bitwidth):
        d(new priv()) {
        d->type       = type;
        d->name       = name;
        d->dimensions = dimensions;
        d->bitwidth   = bitwidth;
    }

    variable::~variable() {
        delete d;
    }

    void variable::set_type(unsigned type) {
        d->type = type;
    }

    unsigned variable::type() const {
        return d->type;
    }

    void variable::set_name(const std::string& name) {
        d->name = name;
    }

    const std::string& variable::name() const {
        return d->name;
    }

    [[maybe_unused]] void variable::set_bitwidth(unsigned bitwidth) {
        d->bitwidth = bitwidth;
    }

    unsigned variable::bitwidth() const {
        return d->bitwidth;
    }

    void variable::set_reference(variable::ptr reference) {
        d->reference = std::move(reference);
    }

    variable::ptr variable::reference() const {
        return d->reference;
    }

    [[maybe_unused]] void variable::set_dimensions(const std::vector<unsigned>& dimensions) {
        d->dimensions = dimensions;
    }

    const std::vector<unsigned>& variable::dimensions() const {
        return d->dimensions;
    }

    class variable_access::priv {
    public:
        priv() = default;

        variable::ptr                                      var;
        std::optional<std::pair<number::ptr, number::ptr>> range;
        expression::vec                                    indexes;
    };

    variable_access::variable_access():
        d(new priv()) {
    }

    variable_access::variable_access(variable::ptr var):
        d(new priv()) {
        d->var = std::move(var);
    }

    variable_access::~variable_access() {
        delete d;
    }

    void variable_access::set_var(variable::ptr var) {
        d->var = std::move(var);
    }

    variable::ptr variable_access::var() const {
        if (d->var->reference()) {
            return d->var->reference();
        } else {
            return d->var;
        }
    }

    void variable_access::set_range(const std::optional<std::pair<number::ptr, number::ptr>>& range) {
        d->range = range;
    }

    const std::optional<std::pair<number::ptr, number::ptr>>& variable_access::range() const {
        return d->range;
    }

    unsigned variable_access::bitwidth() const {
        if (d->range) {
            number::ptr first, second;
            std::tie(first, second) = *d->range;

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
            return d->var->bitwidth();
        }
    }

    void variable_access::set_indexes(const expression::vec& indexes) {
        d->indexes = indexes;
    }

    const expression::vec& variable_access::indexes() const {
        return d->indexes;
    }

    std::ostream& operator<<(std::ostream& os, const variable& v) {

        std::vector<std::string> types{"in", "out", "inout", "state", "wire"};

        os << std::string(os.precision(), ' ')
           << types.at(v.type()) << " "
           << v.name();
        for (const auto& dim: v.dimensions()) {
            os << "[" << dim << "] ";
        }
        os << "(" << v.bitwidth() << ")";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const variable_access& v) {
        os << v.var()->name();

        for (const auto& expr: v.indexes()) {
            os << "[" << *expr << "]";
        }

        if (v.range()) {
            number::ptr first, second;
            std::tie(first, second) = *v.range();

            os << "." << *first;

            if (second.get() != first.get()) {
                os << ":" << *second;
            }
        }

        return os;
    }

} // namespace syrec::applications
