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

namespace syrec {

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

    target_line_adder control_line_adder::operator()(const gate::line& l1, const gate::line& l2) {
        g->add_control(l1);
        g->add_control(l2);
        return target_line_adder(g);
    }

    ////////////////////////////// create_ functions

    gate& create_toffoli(gate& g, const gate::line_container& controls, const gate::line& target) {
        //std::for_each(controls.begin(), controls.end(), std::bind(&gate::add_control, &g, std::placeholders::_1));
        for (const auto& control: controls) {
            g.add_control(control);
        }
        g.add_target(target);
        g.set_type(gateType::Toffoli);

        return g;
    }

    gate& create_cnot(gate& g, const gate::line& control, const gate::line& target) {
        g.add_control(control);
        g.add_target(target);
        g.set_type(gateType::Toffoli);

        return g;
    }

    gate& create_not(gate& g, const gate::line& target) {
        g.add_target(target);
        g.set_type(gateType::Toffoli);
        return g;
    }

    ////////////////////////////// append_ functions

    gate& append_toffoli(circuit& circ, const gate::line_container& controls, const gate::line& target) {
        return create_toffoli(circ.append_gate(), controls, target);
    }

    gate& append_cnot(circuit& circ, const gate::line& control, const gate::line& target) {
        return create_cnot(circ.append_gate(), control, target);
    }

    gate& append_not(circuit& circ, const gate::line& target) {
        return create_not(circ.append_gate(), target);
    }

    control_line_adder append_gate(circuit& circ, const gateType tag) {
        gate& g = circ.append_gate();
        g.set_type(tag);
        return control_line_adder(g);
    }

    control_line_adder append_toffoli(circuit& circ) {
        return append_gate(circ, gateType::Toffoli);
    }

    control_line_adder append_fredkin(circuit& circ) {
        return append_gate(circ, gateType::Fredkin);
    }

} // namespace syrec
