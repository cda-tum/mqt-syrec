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

#include "core/syrec/number.hpp"

#include <cassert>
#include <utility>
#include <variant>

namespace syrec::applications {

    struct evaluate_visitor {
        explicit evaluate_visitor(const number::loop_variable_mapping& map):
            map(map) {}

        unsigned operator()(unsigned value) const {
            return value;
        }

        unsigned operator()(const std::string& value) const {
            auto it = map.find(value);
            assert(it != map.end());
            return it->second;
        }

    private:
        const number::loop_variable_mapping& map;
    };

    class number::priv {
    public:
        explicit priv(std::variant<unsigned, std::string> number):
            number(std::move(number)) {}

        std::variant<unsigned, std::string> number;
    };

    number::number(unsigned value):
        d(new priv(value)) {
    }

    number::number(const std::string& value):
        d(new priv(value)) {
    }

    number::~number() {
        delete d;
    }

    bool number::is_loop_variable() const {
        return std::holds_alternative<std::string>(d->number);
    }

    bool number::is_constant() const {
        return std::holds_alternative<unsigned>(d->number);
    }

    const std::string& number::variable_name() const {
        return std::get<std::string>(d->number);
    }

    unsigned number::evaluate(const loop_variable_mapping& map) const {
        return std::visit(evaluate_visitor(map), d->number);
    }

} // namespace syrec::applications
