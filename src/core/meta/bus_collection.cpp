/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2011  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include "core/meta/bus_collection.hpp"

#include <algorithm>
#include <cassert>

namespace syrec {

    class bus_collection::priv {
    public:
        priv() = default;

        bus_collection::map             buses;
        std::map<std::string, unsigned> initial_values;
    };

    bus_collection::bus_collection():
        d(new priv()) {
    }

    bus_collection::~bus_collection() {
        // NOTE uncommenting this line leads to an segmentation fault
        //    delete d;
    }

    void bus_collection::add(const map::key_type& name, const map::mapped_type& line_indices, const std::optional<unsigned>& initial_value) {
        d->buses.insert(std::make_pair(name, line_indices));

        if (initial_value) {
            d->initial_values[name] = *initial_value;
        }
    }

    bus_collection::map::mapped_type bus_collection::get(const map::key_type& name) const {
        auto it = d->buses.find(name);

        if (it != d->buses.end()) {
            return it->second;
        } else {
            assert(false);
        }

        std::vector<unsigned> v;
        return v;
    }

    const bus_collection::map& bus_collection::buses() const {
        return d->buses;
    }

    [[maybe_unused]] bus_collection::map::key_type bus_collection::find_bus(map::mapped_type::value_type line_index) const {
        for (const map::value_type& p: d->buses) {
            if (std::find(p.second.begin(), p.second.end(), line_index) != p.second.end()) {
                return p.first;
            }
        }

        return {};
    }

    [[maybe_unused]] bool bus_collection::has_bus(map::mapped_type::value_type line_index) const {
        return std::any_of(d->buses.cbegin(), d->buses.cend(), [&](const auto& p) { return std::find(p.second.begin(), p.second.end(), line_index) != p.second.end(); });
    }

    [[maybe_unused]] unsigned bus_collection::signal_index(unsigned line_index) const {
        for (const map::value_type& p: d->buses) {
            auto it = std::find(p.second.begin(), p.second.end(), line_index);
            if (it != p.second.end()) {
                return std::distance(p.second.begin(), it);
            }
        }

        assert(false);
        return 0u;
    }

    [[maybe_unused]] void bus_collection::set_initial_value(const std::string& name, unsigned initial_value) {
        auto it = d->buses.find(name);

        if (it != d->buses.end()) {
            d->initial_values[name] = initial_value;
        }
    }

    [[maybe_unused]] std::optional<unsigned> bus_collection::initial_value(const std::string& name) const {
        auto it = d->buses.find(name);

        if (it != d->buses.end()) {
            auto it2 = d->initial_values.find(name);
            if (it2 != d->initial_values.end()) {
                return it2->second;
            } else {
                return {};
            }
        } else {
            return {};
        }
    }

} // namespace syrec
