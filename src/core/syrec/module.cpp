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

#include <boost/algorithm/string/join.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <iterator>
#include <sstream>

#define foreach_ BOOST_FOREACH

using namespace boost::assign;

namespace revkit::syrec {

    class module::priv {
    public:
        priv() = default;

        std::string    name;
        variable::vec  parameters;
        variable::vec  variables;
        statement::vec statements;
    };

    module::module():
            d(new priv()) {}

    module::module(const std::string& name):
        d(new priv()) {
        d->name = name;
    }

    module::~module() {
        delete d;
    }

    void module::set_name(const std::string& name) {
        d->name = name;
    }

    const std::string& module::name() const {
        return d->name;
    }

    void module::add_parameter(variable::ptr parameter) {
        d->parameters += parameter;
    }

    const variable::vec& module::parameters() const {
        return d->parameters;
    }

    void module::add_variable(variable::ptr variable) {
        d->variables += variable;
    }

    const variable::vec& module::variables() const {
        return d->variables;
    }

    variable::ptr module::find_parameter_or_variable(const std::string& name) const {
        foreach_(variable::ptr var, d->parameters) {
            if (var->name() == name) {
                return var;
            }
        }

        foreach_(variable::ptr var, d->variables) {
            if (var->name() == name) {
                return var;
            }
        }

        return {};
    }

    void module::add_statement(statement::ptr statement) {
        d->statements += statement;
    }

    const statement::vec& module::statements() const {
        return d->statements;
    }

    /* helper function which creates a string from a output stream */
    struct to_string {
        typedef std::string result_type;

        template<typename T>
        std::string operator()(const T& t) const {
            std::stringstream oss;
            oss.precision(0u);
            oss << t;
            return oss.str();
        }
    };

    std::ostream& operator<<(std::ostream& os, const module& m) {
        using boost::adaptors::indirected;
        using boost::adaptors::transformed;

        os << "module " << m.name() << "(" << boost::algorithm::join(m.parameters() | indirected | transformed(to_string()), ", ") << ")" << std::endl;
        boost::copy(m.variables() | indirected, std::ostream_iterator<const variable>(os, "\n"));
        boost::copy(m.statements() | indirected, std::ostream_iterator<const statement>(os));
        return os;
    }

} // namespace revkit::syrec
