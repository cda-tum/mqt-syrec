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

#include "core/syrec/expression.hpp"

namespace revkit
{
  namespace syrec
  {

    class expression::priv
    {
    public:
      priv() {}


    };

    expression::expression()
      : d( new priv() )
    {
    }

    expression::~expression()
    {
      delete d;
    }



    class numeric_expression::priv
    {
    public:
      priv() {}

      number::ptr value;
      unsigned bitwidth;
    };

    numeric_expression::numeric_expression()
      : d( new priv() )
    {
    }

    numeric_expression::numeric_expression( const number::ptr& value, unsigned bitwidth )
      : d( new priv() )
    {
      d->value = value;
      d->bitwidth = bitwidth;
    }

    numeric_expression::~numeric_expression()
    {
      delete d;
    }

    void numeric_expression::set_value( const number::ptr& value )
    {
      d->value = value;
    }

    const number::ptr& numeric_expression::value() const
    {
      return d->value;
    }

    unsigned numeric_expression::bitwidth() const
    {
      return d->bitwidth;
    }

    std::ostream& numeric_expression::print( std::ostream& os ) const
    {
      return os << *d->value;
    }



    class variable_expression::priv
    {
    public:
      priv() {}

      variable_access::ptr var;
    };

    variable_expression::variable_expression()
      : d( new priv() )
    {
    }

    variable_expression::variable_expression( variable_access::ptr var )
      : d( new priv() )
    {
      d->var = var;
    }

    variable_expression::~variable_expression()
    {
      delete d;
    }

    void variable_expression::set_var( variable_access::ptr var )
    {
      d->var = var;
    }

    variable_access::ptr variable_expression::var() const
    {
      return d->var;
    }

    unsigned variable_expression::bitwidth() const
    {
      return d->var->bitwidth();
    }

    std::ostream& variable_expression::print( std::ostream& os ) const
    {
      return os << *d->var;
    }



    class binary_expression::priv
    {
    public:
      priv() {}

      expression::ptr lhs;
      expression::ptr rhs;
      unsigned op;
    };

    binary_expression::binary_expression()
      : d( new priv() )
    {
    }

    binary_expression::binary_expression( expression::ptr lhs,
                                          unsigned op,
                                          expression::ptr rhs )
      : d( new priv() )
    {
      d->lhs = lhs;
      d->op = op;
      d->rhs = rhs;
    }

    binary_expression::~binary_expression()
    {
      delete d;
    }

    void binary_expression::set_lhs( expression::ptr lhs )
    {
      d->lhs = lhs;
    }

    expression::ptr binary_expression::lhs() const
    {
      return d->lhs;
    }

    void binary_expression::set_rhs( expression::ptr rhs )
    {
      d->rhs = rhs;
    }

    expression::ptr binary_expression::rhs() const
    {
      return d->rhs;
    }

    void binary_expression::set_op( unsigned op )
    {
      d->op = op;
    }

    unsigned binary_expression::op() const
    {
      return d->op;
    }

    unsigned binary_expression::bitwidth() const
    {
      switch ( d->op )
      {
      case logical_and:
        return 1;

      case logical_or:
        return 1;

      case less_than:
        return 1;

      case greater_than:
        return 1;

      case equals:
        return 1;

      case not_equals:
        return 1;

      case less_equals:
        return 1;

      case greater_equals:
        return 1;
	
      default:
	// lhs and rhs are assumed to have the same bit-width
        return d->lhs->bitwidth();
      }; 
    }

    std::ostream& binary_expression::print( std::ostream& os ) const
    {
      os << "(" << *d->lhs << " ";

      switch ( d->op )
      {
      case add:
        os << "+"; break;

      case subtract:
        os << "-"; break;

      case exor:
        os << "^"; break;

      case multiply:
        os << "*"; break;

      case divide:
        os << "/"; break;

      case modulo:
        os << "%"; break;

      case frac_divide:
        os << "*>"; break;

      case logical_and:
        os << "&&"; break;

      case logical_or:
        os << "||"; break;

      case bitwise_and:
        os << "&"; break;

      case bitwise_or:
        os << "|"; break;

      case less_than:
        os << "<"; break;

      case greater_than:
        os << ">"; break;

      case equals:
        os << "="; break;

      case not_equals:
        os << "!="; break;

      case less_equals:
        os << "<="; break;

      case greater_equals:
        os << ">="; break;
      };

      return os << " " << *d->rhs << ")";
    }

    
    class unary_expression::priv
    {
    public:
      priv() {}

      unsigned op;
      expression::ptr expr;
    };
    
    unary_expression::unary_expression()
      : d( new priv() )
    {
    }
    
    unary_expression::unary_expression( unsigned op, expression::ptr expr )
      : d( new priv() )
    {
      d->op = op;
      d->expr = expr;
    }
    
    unary_expression::~unary_expression()
    {
      delete d;
    }
    
    void unary_expression::set_expr( expression::ptr expr )
    {
      d->expr = expr;
    }
    
    expression::ptr unary_expression::expr() const
    {
      return d->expr;
    }
    
    void unary_expression::set_op( unsigned op )
    {
      d->op = op;
    }

    unsigned unary_expression::op() const
    {
      return d->op;
    }

    unsigned unary_expression::bitwidth() const
    {
      return ( d->op == logical_not ? 1 : d->expr->bitwidth() );
    }

    std::ostream& unary_expression::print( std::ostream& os ) const
    {
      return os << ( d->op == logical_not ? "!" : "~" ) << *d->expr;
    }    

    
    class shift_expression::priv
    {
    public:
      priv() {}

      expression::ptr lhs;
      number::ptr rhs;
      unsigned op;
    };

    shift_expression::shift_expression()
      : d( new priv() )
    {
    }

    shift_expression::shift_expression( expression::ptr lhs,
                                        unsigned op,
                                        const number::ptr& rhs )
      : d( new priv() )
    {
      d->lhs = lhs;
      d->op = op;
      d->rhs = rhs;
    }

    shift_expression::~shift_expression()
    {
      delete d;
    }

    void shift_expression::set_lhs( expression::ptr lhs )
    {
      d->lhs = lhs;
    }

    expression::ptr shift_expression::lhs() const
    {
      return d->lhs;
    }

    void shift_expression::set_rhs( const number::ptr& rhs )
    {
      d->rhs = rhs;
    }

    const number::ptr& shift_expression::rhs() const
    {
      return d->rhs;
    }

    void shift_expression::set_op( unsigned op )
    {
      d->op = op;
    }

    unsigned shift_expression::op() const
    {
      return d->op;
    }

    unsigned shift_expression::bitwidth() const
    {
      return d->lhs->bitwidth();
    }

    std::ostream& shift_expression::print( std::ostream& os ) const
    {
      return os << "(" << *d->lhs << " " << ( d->op == left ? "<<" : ">>" ) << " " << *d->rhs << ")";
    }



    std::ostream& operator<<( std::ostream& os, const expression& e )
    {
      return e.print( os );
    }
  }
}

