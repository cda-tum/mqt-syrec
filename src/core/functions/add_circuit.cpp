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

#include "core/functions/add_circuit.hpp"

//#include <boost/bind.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/insert.hpp>
#include <functional>

namespace revkit {

    void append_circuit(circuit& circ, const circuit& src, const gate::line_container& controls) {
        insert_circuit(circ, circ.num_gates(), src, controls);
    }

    void prepend_circuit(circuit& circ, const circuit& src, const gate::line_container& controls) {
        insert_circuit(circ, 0, src, controls);
    }

    void insert_circuit(circuit& circ, unsigned pos, const circuit& src, const gate::line_container& controls) {
        typedef std::pair<std::string, std::string> pair_t;
        if (controls.empty()) {
            for (const gate& g: src) {
                gate& new_gate                                                         = circ.insert_gate(pos++);
                new_gate                                                               = g;
                boost::optional<const std::map<std::string, std::string>&> annotations = src.annotations(g);
                if (annotations) {
                    for (const pair_t p: *annotations) {
                        circ.annotate(new_gate, p.first, p.second);
                    }
                }
            }
        } else {
            for (const gate& g: src) {
                gate& new_gate = circ.insert_gate(pos++);
                boost::for_each(controls, std::bind(&gate::add_control, &new_gate, std::placeholders::_1));
                std::for_each(g.begin_controls(), g.end_controls(), std::bind(&gate::add_control, &new_gate, std::placeholders::_1));
                std::for_each(g.begin_targets(), g.end_targets(), std::bind(&gate::add_target, &new_gate, std::placeholders::_1));
                new_gate.set_type(g.type());
                boost::optional<const std::map<std::string, std::string>&> annotations = src.annotations(g);
                if (annotations) {
                    for (const pair_t p: *annotations) {
                        circ.annotate(new_gate, p.first, p.second);
                    }
                }
            }
        }
    }

} // namespace revkit
