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

#include "algorithms/simulation/simple_simulation.hpp"

#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>

#include <core/gate.hpp>
#include <core/target_tags.hpp>

#include <core/utils/timer.hpp>

namespace revkit
{

  boost::dynamic_bitset<>& core_gate_simulation::operator()( const gate& g, boost::dynamic_bitset<>& input ) const
  {
    if ( is_toffoli( g ) )
    {
      boost::dynamic_bitset<> c_mask( input.size() );
      for ( gate::const_iterator itControl = g.begin_controls(); itControl != g.end_controls(); ++itControl )
      {
        c_mask.set( *itControl );
      }

      if ( c_mask.none() || ( ( input & c_mask ) == c_mask ) )
      {
        input.flip( *g.begin_targets() );
      }

      return input;
    }
    else if ( is_fredkin( g ) )
    {
      boost::dynamic_bitset<> c_mask( input.size() );
      for ( gate::const_iterator itControl = g.begin_controls(); itControl != g.end_controls(); ++itControl )
      {
        c_mask.set( *itControl );
      }

      if ( c_mask.none() || ( ( input & c_mask ) == c_mask ) )
      {
        // get both positions and values
        gate::const_iterator it = g.begin_targets();
        unsigned t1 = *it++;
        unsigned t2 = *it;

        bool t1v = input.test( t1 );
        bool t2v = input.test( t2 );

        // only swap when different
        if ( t1v != t2v )
        {
          input.set( t1, t2v );
          input.set( t2, t1v );
        }
      }

      return input;
    }
    else if ( is_peres( g ) )
    {
      if ( input.test( *g.begin_controls() ) ) // is single control set
      {
        // get both positions and value of t1
        gate::const_iterator it = g.begin_targets();
        unsigned t1 = *it++;
        unsigned t2 = *it;

        if ( boost::any_cast<peres_tag>( &g.type() )->swap_targets )
        {
          unsigned tmp = t1;
          t1 = t2;
          t2 = tmp;
        }

        bool t1v = input.test( t1 );

        /* flip t1 */
        input.flip( t1 );

        /* flip t2 if t1 was true */
        if ( t1v )
        {
          input.flip( t2 );
        }
      }

      return input;
    }
    else if ( is_module( g ) )
    {
      boost::dynamic_bitset<> c_mask( input.size() );
      for ( gate::const_iterator itControl = g.begin_controls(); itControl != g.end_controls(); ++itControl )
      {
        c_mask.set( *itControl );
      }

      // cancel if controls are not hit
      if ( !c_mask.is_subset_of( input ) )
      {
        return input;
      }

      const module_tag* tag = boost::any_cast<module_tag>( &g.type() );

      // get the new input sub pattern
      std::vector<unsigned> targets( g.begin_targets(), g.end_targets() );
      boost::dynamic_bitset<> tpattern( targets.size() );
      for ( unsigned i = 0u; i < targets.size(); ++i )
      {
        tpattern.set( tag->target_sort_order.at( i ), input.test( targets.at( i ) ) );
      }
      boost::dynamic_bitset<> toutput;
      assert( simple_simulation( toutput, *tag->reference, tpattern ) );

      for ( unsigned i = 0u; i < targets.size(); ++i )
      {
        input.set( targets.at( i ), toutput.test( tag->target_sort_order.at( i ) ) );
      }

      return input;
    }
    else
    {
      assert( false );
    }
  }

  bool simple_simulation( boost::dynamic_bitset<>& output, const gate& g, const boost::dynamic_bitset<>& input,
                          properties::ptr settings,
                          properties::ptr statistics )
  {
    gate_simulation_func gate_simulation = get<gate_simulation_func>( settings, "gate_simulation", core_gate_simulation() );
    step_result_func     step_result     = get<step_result_func>( settings, "step_result", step_result_func() );

    timer<properties_timer> t;

    if ( statistics )
    {
      properties_timer rt( statistics );
      t.start( rt );
    }

    output = input;
    output = gate_simulation( g, output );
    if ( step_result )
    {
      step_result( g, output );
    }
    return true;
  }

  bool simple_simulation( boost::dynamic_bitset<>& output, circuit::const_iterator first, circuit::const_iterator last, const boost::dynamic_bitset<>& input,
                          properties::ptr settings,
                          properties::ptr statistics )
  {
    gate_simulation_func gate_simulation = get<gate_simulation_func>( settings, "gate_simulation", core_gate_simulation() );
    step_result_func     step_result     = get<step_result_func>( settings, "step_result", step_result_func() );

    timer<properties_timer> t;

    if ( statistics )
    {
      properties_timer rt( statistics );
      t.start( rt );
    }

    output = input;
    while ( first != last )
    {
      output = gate_simulation( *first, output );
      if ( step_result )
      {
        step_result( *first, output );
      }
      ++first;
    }
    return true;
  }

  bool simple_simulation( boost::dynamic_bitset<>& output, const circuit& circ, const boost::dynamic_bitset<>& input,
                          properties::ptr settings,
                          properties::ptr statistics )
  {
    return simple_simulation( output, circ.begin(), circ.end(), input, settings, statistics );
  }

  simulation_func simple_simulation_func( properties::ptr settings, properties::ptr statistics )
  {
    simulation_func f = boost::bind( static_cast<bool (*)(boost::dynamic_bitset<>&, const circuit&, const boost::dynamic_bitset<>&, properties::ptr, properties::ptr)>( simple_simulation ), _1, _2, _3, settings, statistics );
    f.init( settings, statistics );
    return f;
  }

}
