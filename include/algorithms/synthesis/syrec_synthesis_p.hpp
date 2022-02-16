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

/** @cond */
#ifndef SYREC_SYNTHESIS_P_HPP
#define SYREC_SYNTHESIS_P_HPP

#include <boost/graph/adjacency_list.hpp>

#include <core/circuit.hpp>
#include <core/gate.hpp>

namespace revkit
{
 
namespace internal
{
  struct node_properties
  {
    node_properties() {}

    unsigned control;
    gate::line_container controls;
    std::shared_ptr<circuit> circ;
  };
  
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
                                boost::property<boost::vertex_name_t, node_properties> > cct;

  typedef boost::graph_traits<cct>::vertex_descriptor cct_node;
  typedef boost::graph_traits<cct>::edge_descriptor   cct_edge;
  
  struct cct_manager
  {
    cct tree;
    cct_node current;
    cct_node root;
  };
}

}

#endif /* SYREC_SYNTHESIS_P_HPP */
/** @endcond */

