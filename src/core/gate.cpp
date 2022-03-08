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

#include "core/gate.hpp"

namespace syrec {

    struct gate::priv {
        priv() = default;

        line_container controls;
        line_container targets;
        std::any       target_type;
    };

    gate::gate():
        d(new priv()) {
    }

    gate::gate(const gate& other):
        d(new priv()) {
        operator=(other);
    }

    gate::~gate() {
        delete d;
    }

    gate& gate::operator=(const gate& other) {
        if (this != &other) {
            d->controls.clear();
            std::copy(other.begin_controls(), other.end_controls(), std::insert_iterator<gate::line_container>(d->controls, d->controls.begin()));
            d->targets.clear();
            std::copy(other.begin_targets(), other.end_targets(), std::insert_iterator<gate::line_container>(d->targets, d->targets.begin()));
            d->target_type = other.type();
        }
        return *this;
    }

    gate::const_iterator gate::begin_controls() const {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(d->controls.begin(), d->controls.end()), transform_line());
    }

    gate::const_iterator gate::end_controls() const {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(d->controls.end(), d->controls.end()), transform_line());
    }

    gate::const_iterator gate::begin_targets() const {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(d->targets.begin(), d->targets.end()), transform_line());
    }

    gate::const_iterator gate::end_targets() const {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(d->targets.end(), d->targets.end()), transform_line());
    }

    void gate::add_control(line c) {
        d->controls.insert(c);
    }

    void gate::add_target(line l) {
        d->targets.insert(l);
    }

    void gate::set_type(const std::any& t) {
        d->target_type = t;
    }

    const std::any& gate::type() const {
        return d->target_type;
    }

} // namespace syrec
