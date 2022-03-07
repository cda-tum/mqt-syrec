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

#include "core/syrec/module.hpp"

#include <iterator>

namespace syrec::applications {

    class module::priv {
    public:
        priv() = default;

        std::string    name;
        variable::vec  parameters;
        variable::vec  variables;
        statement::vec statements;
    };

    /* module::module():
        d(new priv()) {}*/

    module::module(const std::string& name):
        d(new priv()) {
        d->name = name;
    }

    module::~module() {
        delete d;
    }

    /*void module::set_name(const std::string& name) {
        d->name = name;
    }*/

    const std::string& module::name() const {
        return d->name;
    }

    void module::add_parameter(const variable::ptr& parameter) {
        d->parameters.emplace_back(parameter);
    }

    const variable::vec& module::parameters() const {
        return d->parameters;
    }

    /*void module::add_variable(const variable::ptr& variable) {
        d->variables.emplace_back(variable);
    }*/

    const variable::vec& module::variables() const {
        return d->variables;
    }

    variable::ptr module::find_parameter_or_variable(const std::string& name) const {
        for (variable::ptr var: d->parameters) {
            if (var->name() == name) {
                return var;
            }
        }

        /*    for (variable::ptr var: d->variables) {
            if (var->name() == name) {
                return var;
            }
        }*/

        return {};
    }

    void module::add_statement(const statement::ptr& statement) {
        d->statements.emplace_back(statement);
    }

    const statement::vec& module::statements() const {
        return d->statements;
    }

    /* helper function which creates a string from a output stream */
    /* struct to_string {
        typedef std::string result_type;

        template<typename T>
        std::string operator()(const T& t) const {
            std::stringstream oss;
            oss.precision(0u);
            oss << t;
            return oss.str();
        }
    };*/

    /*std::ostream& operator<<(std::ostream& os, const module& m) {
        os << "module " << m.name() << "(";
        for (const auto& parameter: m.parameters()) {
            if (parameter != m.parameters().front()) {
                os << ", ";
            }
            os << to_string()(*parameter);
        }
        os << ")" << std::endl;
        for (const auto& variable: m.variables()) {
            os << *variable << "\n";
        }
        for (const auto& statement: m.statements()) {
            os << *statement;
        }
        return os;
    }*/

} // namespace syrec::applications
