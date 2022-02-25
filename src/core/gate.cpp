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

namespace revkit {

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

    [[maybe_unused]] gate::iterator gate::begin_controls() {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(d->controls.begin(), d->controls.end()), transform_line());
    }

    [[maybe_unused]] gate::iterator gate::end_controls() {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(d->controls.end(), d->controls.end()), transform_line());
    }

    gate::const_iterator gate::begin_targets() const {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(d->targets.begin(), d->targets.end()), transform_line());
    }

    gate::const_iterator gate::end_targets() const {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(d->targets.end(), d->targets.end()), transform_line());
    }

    [[maybe_unused]] gate::iterator gate::begin_targets() {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(d->targets.begin(), d->targets.end()), transform_line());
    }

    [[maybe_unused]] gate::iterator gate::end_targets() {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(d->targets.end(), d->targets.end()), transform_line());
    }

    unsigned gate::size() const {
        return d->controls.size() + d->targets.size();
    }

    void gate::add_control(line c) {
        d->controls.insert(c);
    }

    void gate::remove_control(line c) {
        d->controls.erase(c);
    }

    void gate::add_target(line l) {
        d->targets.insert(l);
    }

    void gate::remove_target(line l) {
        d->targets.erase(l);
    }

    void gate::set_type(const std::any& t) {
        d->target_type = t;
    }

    const std::any& gate::type() const {
        return d->target_type;
    }

    // filtered_gate //////////

    struct filtered_gate::priv {
        priv(gate& base, std::vector<unsigned>& filter):
            base(base),
            filter(filter) {}

        gate&                  base;
        std::vector<unsigned>& filter;
    };

    filtered_gate::filtered_gate(gate& base, std::vector<unsigned>& filter):
        gate(), d(new priv(base, filter)) {
    }

    filtered_gate::filtered_gate(const filtered_gate& other):
        gate(other), d(new priv(*other.d)) {
    }

    filtered_gate::~filtered_gate() {
        delete d;
    }

    filtered_gate& filtered_gate::operator=(const filtered_gate& other) {
        if (this != &other) {
            d->base   = other.d->base;
            d->filter = other.d->filter;
        }
        return *this;
    }

    filtered_gate::const_iterator filtered_gate::begin_controls() const {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(filter_line(d->filter), d->base.d->controls.begin(), d->base.d->controls.end()), transform_line(d->filter));
    }

    filtered_gate::const_iterator filtered_gate::end_controls() const {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(filter_line(d->filter), d->base.d->controls.end(), d->base.d->controls.end()), transform_line(d->filter));
    }

    filtered_gate::iterator filtered_gate::begin_controls() {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(filter_line(d->filter), d->base.d->controls.begin(), d->base.d->controls.end()), transform_line(d->filter));
    }

    filtered_gate::iterator filtered_gate::end_controls() {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(filter_line(d->filter), d->base.d->controls.end(), d->base.d->controls.end()), transform_line(d->filter));
    }

    filtered_gate::const_iterator filtered_gate::begin_targets() const {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(filter_line(d->filter), d->base.d->targets.begin(), d->base.d->targets.end()), transform_line(d->filter));
    }

    filtered_gate::const_iterator filtered_gate::end_targets() const {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(filter_line(d->filter), d->base.d->targets.end(), d->base.d->targets.end()), transform_line(d->filter));
    }

    filtered_gate::iterator filtered_gate::begin_targets() {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(filter_line(d->filter), d->base.d->targets.begin(), d->base.d->targets.end()), transform_line(d->filter));
    }

    filtered_gate::iterator filtered_gate::end_targets() {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(filter_line(d->filter), d->base.d->targets.end(), d->base.d->targets.end()), transform_line(d->filter));
    }

    unsigned filtered_gate::size() const {
        // The size of the filtered gate with filter F and base gate with controls C and T
        // is the size of the intersection of F with C and T.
        gate::line_container size_set;
        std::set_intersection(d->base.d->controls.begin(), d->base.d->controls.end(), d->filter.begin(), d->filter.end(), std::insert_iterator<gate::line_container>(size_set, size_set.begin()));
        std::set_intersection(d->base.d->targets.begin(), d->base.d->targets.end(), d->filter.begin(), d->filter.end(), std::insert_iterator<gate::line_container>(size_set, size_set.begin()));

        return size_set.size();
    }

    void filtered_gate::add_control(line c) {
        // Can only add the control if the line is filtered and the control is not set in the base
        if (c < d->filter.size() &&
            std::find(d->base.d->controls.begin(), d->base.d->controls.end(), c) == d->base.d->controls.end()) {
            d->base.add_control(d->filter.at(c));
        }
    }

    void filtered_gate::remove_control(line c) {
        // Can only remove the control if the line is filtered
        if (c < d->filter.size()) {
            d->base.remove_control(d->filter.at(c));
        }
    }

    void filtered_gate::add_target(line l) {
        // Can only add the target if the line is filtered and the target is not set in the base
        if (l < d->filter.size() &&
            std::find(d->base.d->targets.begin(), d->base.d->targets.end(), l) == d->base.d->targets.end()) {
            d->base.add_target(d->filter.at(l));
        }
    }

    void filtered_gate::remove_target(line l) {
        // Can only remove the target if the line is filtered
        if (l < d->filter.size()) {
            d->base.remove_target(d->filter.at(l));
        }
    }

    void filtered_gate::set_type(const std::any& t) {
        d->base.set_type(t);
    }

    const std::any& filtered_gate::type() const {
        return d->base.type();
    }

} // namespace revkit
