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

#include "core/functions/add_gates.hpp"

#include "core/target_tags.hpp"

#include <boost/assign/std/vector.hpp>
//#include <boost/bind.hpp>
//#include <boost/range/algorithm.hpp>
#include <functional>
#include <algorithm>

using namespace boost::assign;

namespace revkit {

    ////////////////////////////// class target_line_adder
    target_line_adder::target_line_adder(gate* gate):
        g(gate) {
    }

    gate& target_line_adder::operator()(const gate::line& l1) {
        g->add_target(l1);
        return *g;
    }

    gate& target_line_adder::operator()(const gate::line& l1, const gate::line& l2) {
        g->add_target(l1);
        g->add_target(l2);
        return *g;
    }

    ////////////////////////////// class control_line_adder
    control_line_adder::control_line_adder(gate& gate):
        g(&gate) {
    }

    target_line_adder control_line_adder::operator()() {
        return target_line_adder(g);
    }

    target_line_adder control_line_adder::operator()(const gate::line& l1) {
        g->add_control(l1);
        return target_line_adder(g);
    }

    target_line_adder control_line_adder::operator()(const gate::line& l1, const gate::line& l2) {
        g->add_control(l1);
        g->add_control(l2);
        return target_line_adder(g);
    }

    target_line_adder control_line_adder::operator()(const gate::line& l1, const gate::line& l2, const gate::line& l3) {
        g->add_control(l1);
        g->add_control(l2);
        g->add_control(l3);
        return target_line_adder(g);
    }

    target_line_adder control_line_adder::operator()(const gate::line& l1, const gate::line& l2, const gate::line& l3, const gate::line& l4) {
        g->add_control(l1);
        g->add_control(l2);
        g->add_control(l3);
        g->add_control(l4);
        return target_line_adder(g);
    }

    target_line_adder control_line_adder::operator()(const gate::line& l1, const gate::line& l2, const gate::line& l3, const gate::line& l4, const gate::line& l5) {
        g->add_control(l1);
        g->add_control(l2);
        g->add_control(l3);
        g->add_control(l4);
        g->add_control(l5);
        return target_line_adder(g);
    }

    target_line_adder control_line_adder::operator()(const gate::line& l1, const gate::line& l2, const gate::line& l3, const gate::line& l4, const gate::line& l5, const gate::line& l6) {
        g->add_control(l1);
        g->add_control(l2);
        g->add_control(l3);
        g->add_control(l4);
        g->add_control(l5);
        g->add_control(l6);
        return target_line_adder(g);
    }

    target_line_adder control_line_adder::operator()(const gate::line& l1, const gate::line& l2, const gate::line& l3, const gate::line& l4, const gate::line& l5, const gate::line& l6, const gate::line& l7) {
        g->add_control(l1);
        g->add_control(l2);
        g->add_control(l3);
        g->add_control(l4);
        g->add_control(l5);
        g->add_control(l6);
        g->add_control(l7);
        return target_line_adder(g);
    }

    target_line_adder control_line_adder::operator()(const gate::line& l1, const gate::line& l2, const gate::line& l3, const gate::line& l4, const gate::line& l5, const gate::line& l6, const gate::line& l7, const gate::line& l8) {
        g->add_control(l1);
        g->add_control(l2);
        g->add_control(l3);
        g->add_control(l4);
        g->add_control(l5);
        g->add_control(l6);
        g->add_control(l7);
        g->add_control(l8);
        return target_line_adder(g);
    }

    target_line_adder control_line_adder::operator()(const gate::line& l1, const gate::line& l2, const gate::line& l3, const gate::line& l4, const gate::line& l5, const gate::line& l6, const gate::line& l7, const gate::line& l8, const gate::line& l9) {
        g->add_control(l1);
        g->add_control(l2);
        g->add_control(l3);
        g->add_control(l4);
        g->add_control(l5);
        g->add_control(l6);
        g->add_control(l7);
        g->add_control(l8);
        g->add_control(l9);
        return target_line_adder(g);
    }

    ////////////////////////////// create_ functions

    gate& create_toffoli(gate& g, const gate::line_container& controls, const gate::line& target) {
        //std::for_each(controls.begin(), controls.end(), std::bind(&gate::add_control, &g, std::placeholders::_1));
        for (const auto& control: controls) {
            g.add_control(control);
        }
        g.add_target(target);
        g.set_type(toffoli_tag());

        return g;
    }

    gate& create_fredkin(gate& g, const gate::line_container& controls, const gate::line& target1, const gate::line& target2) {
        //std::for_each(controls.begin(), controls.end(), std::bind(&gate::add_control, &g, std::placeholders::_1));
        for (const auto& control: controls) {
            g.add_control(control);
        }
        g.add_target(target1);
        g.add_target(target2);
        g.set_type(fredkin_tag());

        return g;
    }

    gate& create_peres(gate& g, const gate::line& control, const gate::line& target1, const gate::line& target2) {
        g.add_control(control);
        g.add_target(target1);
        g.add_target(target2);

        peres_tag tag;
        tag.swap_targets = target1 > target2;
        g.set_type(tag);

        return g;
    }

    gate& create_cnot(gate& g, const gate::line& control, const gate::line& target) {
        g.add_control(control);
        g.add_target(target);
        g.set_type(toffoli_tag());

        return g;
    }

    gate& create_v(gate& g, const gate::line& control, const gate::line& target) {
        g.add_control(control);
        g.add_target(target);
        g.set_type(v_tag());

        return g;
    }

    gate& create_vplus(gate& g, const gate::line& control, const gate::line& target) {
        g.add_control(control);
        g.add_target(target);
        g.set_type(vplus_tag());

        return g;
    }

    gate& create_not(gate& g, const gate::line& target) {
        g.add_target(target);
        g.set_type(toffoli_tag());
        return g;
    }

    gate& create_module(gate& g, const circuit& circ, const std::string& name, const gate::line_container& controls, const std::vector<unsigned>& targets) {
        typedef std::map<std::string, std::shared_ptr<circuit>> map_t;
        const map_t&                                            modules = circ.modules();
        map_t::const_iterator                                   it      = modules.find(name);
        assert(it != modules.end());

        //boost::for_each(controls, std::bind(&gate::add_control, &g, std::placeholders::_1));
        for (const auto& control: controls) {
            g.add_control(control);
        }
        //boost::for_each(targets, std::bind(&gate::add_target, &g, std::placeholders::_1));
        for (const auto& target: targets) {
            g.add_target(target);
        }
        module_tag module;
        module.reference = it->second;
        module.name      = name;

        // sort order
        std::vector<unsigned> targets_sorted(targets.begin(), targets.end());
        std::sort(targets_sorted.begin(), targets_sorted.end());
        for (unsigned index: targets) {
            module.target_sort_order += std::distance(targets_sorted.begin(), std::find(targets_sorted.begin(), targets_sorted.end(),  index));
        }

        g.set_type(module);
        return g;
    }

    ////////////////////////////// append_ functions

    gate& append_toffoli(circuit& circ, const gate::line_container& controls, const gate::line& target) {
        return create_toffoli(circ.append_gate(), controls, target);
    }

    gate& append_fredkin(circuit& circ, const gate::line_container& controls, const gate::line& target1, const gate::line& target2) {
        return create_fredkin(circ.append_gate(), controls, target1, target2);
    }

    gate& append_peres(circuit& circ, const gate::line& control, const gate::line& target1, const gate::line& target2) {
        return create_peres(circ.append_gate(), control, target1, target2);
    }

    gate& append_cnot(circuit& circ, const gate::line& control, const gate::line& target) {
        return create_cnot(circ.append_gate(), control, target);
    }

    gate& append_v(circuit& circ, const gate::line& control, const gate::line& target) {
        return create_v(circ.append_gate(), control, target);
    }

    gate& append_vplus(circuit& circ, const gate::line& control, const gate::line& target) {
        return create_vplus(circ.append_gate(), control, target);
    }

    gate& append_not(circuit& circ, const gate::line& target) {
        return create_not(circ.append_gate(), target);
    }

    gate& append_module(circuit& circ, const std::string& module_name, const gate::line_container& controls, const std::vector<unsigned>& targets) {
        return create_module(circ.append_gate(), circ, module_name, controls, targets);
    }

    control_line_adder append_gate(circuit& circ, const std::any& tag) {
        gate& g = circ.append_gate();
        g.set_type(tag);
        return control_line_adder(g);
    }

    control_line_adder append_toffoli(circuit& circ) {
        return append_gate(circ, toffoli_tag());
    }

    control_line_adder append_fredkin(circuit& circ) {
        return append_gate(circ, fredkin_tag());
    }

    control_line_adder append_v(circuit& circ) {
        return append_gate(circ, v_tag());
    }

    control_line_adder append_vnot(circuit& circ) {
        return append_gate(circ, vplus_tag());
    }

    ////////////////////////////// prepend_ functions

    gate& prepend_toffoli(circuit& circ, const gate::line_container& controls, const gate::line& target) {
        return create_toffoli(circ.prepend_gate(), controls, target);
    }

    gate& prepend_fredkin(circuit& circ, const gate::line_container& controls, const gate::line& target1, const gate::line& target2) {
        return create_fredkin(circ.prepend_gate(), controls, target1, target2);
    }

    gate& prepend_peres(circuit& circ, const gate::line& control, const gate::line& target1, const gate::line& target2) {
        return create_peres(circ.prepend_gate(), control, target1, target2);
    }

    gate& prepend_cnot(circuit& circ, const gate::line& control, const gate::line& target) {
        return create_cnot(circ.prepend_gate(), control, target);
    }

    gate& prepend_v(circuit& circ, const gate::line& control, const gate::line& target) {
        return create_v(circ.prepend_gate(), control, target);
    }

    gate& prepend_vplus(circuit& circ, const gate::line& control, const gate::line& target) {
        return create_vplus(circ.prepend_gate(), control, target);
    }

    gate& prepend_not(circuit& circ, const gate::line& target) {
        return create_not(circ.prepend_gate(), target);
    }

    gate& prepend_module(circuit& circ, const std::string& module_name, const gate::line_container& controls, const std::vector<unsigned>& targets) {
        return create_module(circ.prepend_gate(), circ, module_name, controls, targets);
    }

    control_line_adder prepend_gate(circuit& circ, const std::any& tag) {
        gate& g = circ.prepend_gate();
        g.set_type(tag);
        return control_line_adder(g);
    }

    control_line_adder prepend_toffoli(circuit& circ) {
        return prepend_gate(circ, toffoli_tag());
    }

    control_line_adder prepend_fredkin(circuit& circ) {
        return prepend_gate(circ, fredkin_tag());
    }

    control_line_adder prepend_v(circuit& circ) {
        return prepend_gate(circ, v_tag());
    }

    control_line_adder prepend_vnot(circuit& circ) {
        return prepend_gate(circ, vplus_tag());
    }

    ////////////////////////////// insert_ functions

    gate& insert_toffoli(circuit& circ, unsigned n, const gate::line_container& controls, const gate::line& target) {
        return create_toffoli(circ.insert_gate(n), controls, target);
    }

    gate& insert_fredkin(circuit& circ, unsigned n, const gate::line_container& controls, const gate::line& target1, const gate::line& target2) {
        return create_fredkin(circ.insert_gate(n), controls, target1, target2);
    }

    gate& insert_peres(circuit& circ, unsigned n, const gate::line& control, const gate::line& target1, const gate::line& target2) {
        return create_peres(circ.insert_gate(n), control, target1, target2);
    }

    gate& insert_cnot(circuit& circ, unsigned n, const gate::line& control, const gate::line& target) {
        return create_cnot(circ.insert_gate(n), control, target);
    }

    gate& insert_v(circuit& circ, unsigned n, const gate::line& control, const gate::line& target) {
        return create_v(circ.insert_gate(n), control, target);
    }

    gate& insert_vplus(circuit& circ, unsigned n, const gate::line& control, const gate::line& target) {
        return create_vplus(circ.insert_gate(n), control, target);
    }

    gate& insert_not(circuit& circ, unsigned n, const gate::line& target) {
        return create_not(circ.insert_gate(n), target);
    }

    gate& insert_module(circuit& circ, unsigned n, const std::string& module_name, const gate::line_container& controls, const std::vector<unsigned>& targets) {
        return create_module(circ.insert_gate(n), circ, module_name, controls, targets);
    }

    control_line_adder insert_gate(circuit& circ, unsigned n, const std::any& tag) {
        gate& g = circ.insert_gate(n);
        g.set_type(tag);
        return control_line_adder(g);
    }

    control_line_adder insert_toffoli(circuit& circ, unsigned n) {
        return insert_gate(circ, n, toffoli_tag());
    }

    control_line_adder insert_fredkin(circuit& circ, unsigned n) {
        return insert_gate(circ, n, fredkin_tag());
    }

    control_line_adder insert_v(circuit& circ, unsigned n) {
        return insert_gate(circ, n, v_tag());
    }

    control_line_adder insert_vnot(circuit& circ, unsigned n) {
        return insert_gate(circ, n, vplus_tag());
    }

} // namespace revkit
