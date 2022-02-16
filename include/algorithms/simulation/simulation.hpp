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

/**
 * @file simulation.hpp
 *
 * @brief General Simulation type definitions
 */

#include <boost/dynamic_bitset.hpp>
#include <boost/function.hpp>

#include <core/circuit.hpp>
#include <core/functor.hpp>

namespace revkit
{

  /**
   * @brief Simulation functor
   *
   * @author RevKit
   * @since  1.0
   */
  typedef functor<bool(boost::dynamic_bitset<>&, const circuit&, const boost::dynamic_bitset<>&)> simulation_func;

  /**
   * @brief Multi step simulation functor
   *
   * @author RevKit
   * @since  1.2
   */
  typedef functor<bool(std::vector<boost::dynamic_bitset<> >&, const circuit&, const std::vector<boost::dynamic_bitset<> >& )> multi_step_simulation_func;

}
