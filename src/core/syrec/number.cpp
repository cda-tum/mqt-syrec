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

#include "core/syrec/number.hpp"
#include "core/syrec/expression.hpp"

#include <boost/variant.hpp>

namespace revkit
{
  namespace syrec
  {
    
    class binary_numeric_expr 
    {
    public:
      
      explicit binary_numeric_expr( const number::ptr lhs, const unsigned op, const number::ptr rhs )
        : op ( op ), lhs( lhs ), rhs( rhs )
      {
      }
      
      unsigned get_op() const
      {
	return op;
      }
      
      number::ptr get_lhs() const
      {
	return lhs;
      }
      
      number::ptr get_rhs() const
      {
	return rhs;
      }
      
    private:
      unsigned op;
      number::ptr lhs;
      number::ptr rhs;
    };
    
    struct evaluate_visitor : public boost::static_visitor<unsigned>
    {
      explicit evaluate_visitor( const number::loop_variable_mapping& map )
        : map( map ) {}

      unsigned operator()( unsigned value ) const
      {
        return value;
      }

      unsigned operator()( const std::string& value ) const
      {
        number::loop_variable_mapping::const_iterator it = map.find( value );
        assert( it != map.end() );
        return it->second;
      }
      
      unsigned operator()( const binary_numeric_expr& value ) const
      {
	unsigned lhs_value = value.get_lhs()->evaluate( map );
	unsigned rhs_value = value.get_rhs()->evaluate( map );
	
	switch ( value.get_op() )
	{
	  case numeric_expression::add: // +
          {
            return lhs_value + rhs_value;
          } 
    
	  case numeric_expression::subtract: // -
          {
            return lhs_value - rhs_value;
          }
    
	  case numeric_expression::multiply: // *
          {
            return lhs_value * rhs_value;
          }
    
	  case numeric_expression::divide: // /
          {
            return lhs_value / rhs_value;
          }
    
	  default:
	    return 0;
	}
      }

    private:
      const number::loop_variable_mapping& map;
    };
    
    

    class number::priv
    {
    public:
      explicit priv( const boost::variant<unsigned, std::string, boost::recursive_wrapper<binary_numeric_expr> >& number )
        : number( number ) {}

      boost::variant<unsigned, std::string, boost::recursive_wrapper<binary_numeric_expr> > number;
    };

    number::number( unsigned value )
      : d( new priv( value ) )
    {
    }

    number::number( const std::string& value )
      : d( new priv( value ) )
    {
    }
    
    number::number( const number::ptr lhs, const unsigned op, const number::ptr rhs )
      : d ( new priv( binary_numeric_expr( lhs, op, rhs ) ) )
    {
    }

    number::~number()
    {
      delete d;
    }

    
    bool number::is_loop_variable() const
    {
      return boost::get<std::string>( &d->number );
    }
    
    bool number::is_conjunction() const
    {
      return boost::get<binary_numeric_expr>( &d->number );
    }
    
    bool number::is_constant() const
    {
      return boost::get<unsigned>( &d->number );
    }

    const std::string& number::variable_name() const
    {
      return *boost::get<std::string>( &d->number );
    }
    
    binary_numeric_expr* number::conjunction_expr() const
    {
      return boost::get<binary_numeric_expr>( &d->number );
    }

    unsigned number::evaluate( const loop_variable_mapping& map ) const
    {
      return boost::apply_visitor( evaluate_visitor( map ), d->number );
    }

    struct output_visitor : public boost::static_visitor<std::ostream&>
    {
      explicit output_visitor( std::ostream& os ) : os( os ) {}

      std::ostream& operator()( unsigned value ) const
      {
        return os << value;
      }

      std::ostream& operator()( const std::string& value ) const
      {
        return os << '$' << value;
      }
      
      std::ostream& operator()( const binary_numeric_expr& value ) const
      {
	os << "( " << *value.get_lhs();
	
	switch ( value.get_op() )
	{
	  case numeric_expression::add: // +
          {
            os << " + ";
          } break;
    
	  case numeric_expression::subtract: // -
          {
            os << " - ";
          } break;
    
	  case numeric_expression::multiply: // *
          {
            os << " * ";
          } break;
    
	  case numeric_expression::divide: // /
          {
            os << " / ";
          } break;
    
	  default:
	    os << "Invalid Op";
	}	
        
        os << *value.get_rhs() << " )";
	
	return os; 
      }

    private:
      std::ostream& os;
    };

    std::ostream& operator<<( std::ostream& os, const number& n )
    {
      return boost::apply_visitor( output_visitor( os ), n.d->number );
    }

  }
}
