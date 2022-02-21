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

#include "core/syrec/program.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <iterator>

#define foreach_ BOOST_FOREACH

using namespace boost::assign;

namespace revkit {
    namespace syrec {

        class program::priv {
        public:
            priv() {}

            module::vec modules;
        };

        program::program():
            d(new priv()) {
        }

        program::~program() {
            delete d;
        }

        void program::add_module(module::ptr module) {
            d->modules += module;
        }

        const module::vec& program::modules() const {
            return d->modules;
        }

        module::ptr program::find_module(const std::string& name) const {
            for (module::ptr& p: d->modules) {
                if (p->name() == name) {
                    return p;
                }
            }

            return module::ptr();
        }

        std::ostream& operator<<(std::ostream& os, const program& p) {
            using boost::adaptors::indirected;

            unsigned old_precision = os.precision(2u);
            boost::copy(p.modules() | indirected, std::ostream_iterator<const module>(os));
            os.precision(old_precision);

            return os;
        }

    } // namespace syrec
} // namespace revkit
