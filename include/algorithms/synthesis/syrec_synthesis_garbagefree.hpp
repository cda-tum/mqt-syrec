/**
 * @file syrec_synthesis_garbagefree.hpp
 *
 * @brief garbage-free SyReC Synthesis
 */

#ifndef SYREC_SYNTHESIS_GARBAGEFREE_HPP
#define SYREC_SYNTHESIS_GARBAGEFREE_HPP

#include "syrec_synthesis.hpp"
/* #include <core/circuit.hpp>
#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <boost/assign/std/set.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/irange.hpp>
#include <boost/tuple/tuple.hpp>
#include <core/properties.hpp>
#include <core/syrec/program.hpp>*/
#include <functional>
#include <numeric>

namespace revkit {
    class garbagefree_syrec_synthesizer: public standard_syrec_synthesizer {
    public:
        garbagefree_syrec_synthesizer(circuit& circ, const syrec::program& prog);

        void initialize_changing_variables(const syrec::program& program) override;

        bool on_module(syrec::module::ptr) override;
        bool on_statement(syrec::statement::ptr statement) override;
        bool on_expression(const syrec::expression::ptr& expression, std::vector<unsigned>& lines);

        void set_settings(properties::ptr settings) override;
        void set_main_module(syrec::module::ptr main_module) override;

    protected:
        // statements
        bool on_statement(const syrec::swap_statement& statement) override;
        bool on_statement(const syrec::unary_statement& statement) override;
        bool on_statement(const syrec::assign_statement& statement) override;
        bool on_statement(const syrec::if_statement& statement) override;
        bool on_statement(const syrec::for_statement& statement) override;
        bool on_statement(const syrec::call_statement& statement) override;
        bool on_statement(const syrec::uncall_statement& statement) override;
        bool on_statement(const syrec::skip_statement& statement) override;

        // expressions
        bool on_expression(const syrec::numeric_expression& expression, std::vector<unsigned>& lines);
        bool on_expression(const syrec::variable_expression& expression, std::vector<unsigned>& lines);
        bool on_expression(const syrec::shift_expression& expression, std::vector<unsigned>& lines);
        bool on_expression(const syrec::binary_expression& expression, std::vector<unsigned>& lines);
        bool on_expression(const syrec::unary_expression& expression, std::vector<unsigned>& lines);

        bool on_condition(const syrec::expression::ptr& condition, unsigned result_line);
        bool on_condition(const syrec::numeric_expression& condition, unsigned result_line);
        bool on_condition(const syrec::variable_expression& condition, unsigned result_line);
        bool on_condition(const syrec::shift_expression& condition, unsigned result_line);
        bool on_condition(const syrec::binary_expression& condition, unsigned result_line);
        bool on_condition(const syrec::unary_expression& condition, unsigned result_line);

        // unary operations
        bool bitwise_negation(const std::vector<unsigned>& dest); // ~
        bool decrement(const std::vector<unsigned>& dest);        // --
        bool increment(const std::vector<unsigned>& dest);        // ++

        // binary operations
        bool bitwise_and(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2); // &
        bool bitwise_cnot(const std::vector<unsigned>& dest, const std::vector<unsigned>& src);                                    // ^=
        bool bitwise_or(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);  // |
        bool conjunction(unsigned dest, unsigned src1, unsigned src2);                                                             // &&
        bool decrease(const std::vector<unsigned>& dest, const std::vector<unsigned>& src);                                        // -=
        bool decrease_with_carry(const std::vector<unsigned>& dest, const std::vector<unsigned>& src, unsigned carry);
        bool disjunction(unsigned dest, unsigned src1, unsigned src2);                                                          // ||
        bool division(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2); // /
        bool equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                       // =
        bool greater_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);               // >=
        bool greater_than(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                 // >
        bool increase(const std::vector<unsigned>& dest, const std::vector<unsigned>& src);                                     // +=
        bool increase_with_carry(const std::vector<unsigned>& dest, const std::vector<unsigned>& src, unsigned carry);
        bool less_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                             // <=
        bool less_than(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                               // <
        bool modulo(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);              // %
        bool multiplication(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);      // *
        bool multiplication_full(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2); // *%*
        bool not_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                              // !=
        bool swap(const std::vector<unsigned>& dest1, const std::vector<unsigned>& dest2);                                                 // <=>

        // shift operations
        bool left_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2);  // <<
        bool right_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2); // >>

        bool get_variables(syrec::variable_access::ptr var, std::vector<unsigned>& lines);
        bool get_expr_variables(syrec::variable_access::ptr var, std::vector<unsigned>& lines);
        //bool unget_variables( syrec::variable_access::ptr var, std::vector<unsigned>& lines );
        bool array_swapping(unsigned offset, std::vector<unsigned> dimensions, std::vector<std::shared_ptr<syrec::expression>> indexes, unsigned bitwidth, std::vector<unsigned>& lines);
        bool array_copying(unsigned offset, std::vector<unsigned> dimensions, std::vector<std::shared_ptr<syrec::expression>> indexes, unsigned bitwidth, std::vector<unsigned>& lines);

        bool     get_reusable_constant_lines(unsigned bitwidth, unsigned value, std::vector<unsigned>& lines);
        unsigned get_reusable_constant_line(bool value);
        //void release_constant_lines( std::map< bool, std::vector<unsigned> > const_lines );
        //void release_inv_constant_lines( std::map< bool, std::vector<unsigned> > const_lines );
        void release_constant_lines(const std::list<boost::tuple<bool, bool, unsigned>>& const_lines);
        void release_constant_line(boost::tuple<bool, bool, unsigned> const_line);
        void release_constant_line(unsigned index, bool value);

        // efficient controls
        bool assemble_circuit(const cct_node&);
        bool assemble_circuit(circuit& circ, const cct_node& current, gate::line_container controls);

        bool optimization_decision(const cct_node&);

        // stuff for reverse synthesis
        bool unget_variables(syrec::variable_access::ptr var, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl);
        bool unget_expr_variables(syrec::variable_access::ptr var, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl);
        void get_expr_lines(syrec::expression::ptr expression, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl);
        void get_expr_lines(syrec::variable_access::ptr var, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl);
        bool reverse_array_swapping(unsigned offset, std::vector<unsigned> dimensions, std::vector<std::shared_ptr<syrec::expression>> indexes, unsigned bitwidth, std::vector<unsigned>& lines);
        bool reverse_array_copying(unsigned offset, std::vector<unsigned> dimensions, std::vector<std::shared_ptr<syrec::expression>> indexes, unsigned bitwidth, std::vector<unsigned>& lines);

        // expressions
        bool off_expression(const syrec::numeric_expression& expression, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl);
        bool off_expression(const syrec::variable_expression& expression, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl);
        bool off_expression(const syrec::shift_expression& expression, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl);
        bool off_expression(const syrec::binary_expression& expression, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl);
        bool off_expression(const syrec::unary_expression& expression, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl);
        bool off_expression(const syrec::expression::ptr& expression, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl);

        bool get_subexpr_lines(const syrec::binary_expression& expression, std::stack<boost::tuple<bool, bool, unsigned>>& se_cl);
        bool get_subexpr_lines(const syrec::unary_expression& expression, std::stack<boost::tuple<bool, bool, unsigned>>& se_cl);
        bool get_subexpr_lines(const syrec::numeric_expression& expression, std::stack<boost::tuple<bool, bool, unsigned>>& se_cl);
        bool get_subexpr_lines(const syrec::variable_expression& expression, std::stack<boost::tuple<bool, bool, unsigned>>& se_cl);
        bool get_subexpr_lines(const syrec::shift_expression& expression, std::stack<boost::tuple<bool, bool, unsigned>>& se_cl);
        bool get_subexpr_lines(const syrec::expression::ptr& expression, std::stack<boost::tuple<bool, bool, unsigned>>& se_cl);

        // unary operations
        bool reverse_bitwise_negation(const std::vector<unsigned>& dest);

        // binary operations
        bool reverse_bitwise_and(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);         // &
        bool reverse_bitwise_cnot(const std::vector<unsigned>& dest, const std::vector<unsigned>& src);                                            // ^=
        bool reverse_bitwise_or(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);          // |
        bool reverse_disjunction(unsigned dest, unsigned src1, unsigned src2);                                                                     // ||
        bool reverse_division(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);            // /
        bool reverse_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                                  // =
        bool reverse_greater_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                          // >=
        bool reverse_greater_than(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                            // >
        bool reverse_less_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                             // <=
        bool reverse_less_than(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                               // <
        bool reverse_modulo(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);              // %
        bool reverse_multiplication(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);      // *
        bool reverse_multiplication_full(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2); // *%*
        bool reverse_not_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                              // !=
        bool reverse_swap(const std::vector<unsigned>& dest1, const std::vector<unsigned>& dest2);                                                 // <=>

        // shift operations
        bool reverse_left_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2);  // <<
        bool reverse_right_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2); // >>

    private:
        std::stack<syrec::module::ptr> modules;

        properties::ptr _settings;
        std::string     variable_name_format;
        unsigned        if_realization;
        bool            efficient_controls;
        bool            garbagefree;

        std::stack<boost::tuple<bool, bool, unsigned>> used_const_lines;

        unsigned                              depth;
        unsigned                              dupl_count;
        std::map<bool, std::vector<unsigned>> free_const_lines; // auch als stack statt vector?
        syrec::number::loop_variable_mapping  intern_variable_mapping;

        syrec::expression::ptr      evaluate_to_numeric_expression(syrec::expression::ptr expression);
        syrec::variable_access::ptr evaluate_to_numeric_expression(const syrec::variable_access::ptr& var_access);

        template<typename T>
        struct is_type {
            template<typename Ptr>
            bool operator()(const Ptr& p) const {
                return dynamic_cast<T*>(p.get());
            }
        };
        /*
    static bool less( const std::vector<std::shared_ptr<syrec::expression> >& idx1, const std::vector<std::shared_ptr<syrec::expression> >& idx2 ) 
    {
      if ( idx1.size() < idx2.size() ) return true;
      if ( idx1.size() > idx2.size() ) return false;
      for ( unsigned i = 0; i < idx1.size(); ++i )
      {
	if ( boost::lexical_cast<std::string>( *idx1.at( i ) ) < boost::lexical_cast<std::string>( *idx2.at( i ) ) ) return true;
	if ( boost::lexical_cast<std::string>( *idx1.at( i ) ) > boost::lexical_cast<std::string>( *idx2.at( i ) ) ) return false;
      }
      return false;
    }
    
    static bool equal( const std::vector<std::shared_ptr<syrec::expression> >& idx1, const std::vector<std::shared_ptr<syrec::expression> >& idx2 ) 
    {
      return ( !( less( idx1, idx2 ) ) && !( less( idx2, idx1 ) ) );
    }
    
    static bool less_relaxed( const std::vector<std::shared_ptr<syrec::expression> >& idx1, const std::vector<std::shared_ptr<syrec::expression> >& idx2 ) 
    {
      if ( idx1.size() < idx2.size() ) return true;
      if ( idx1.size() > idx2.size() ) return false;
      bool all_numbers1 = true;
      bool all_numbers2 = true;
      if ( (unsigned)boost::count_if( idx1, is_type<syrec::numeric_expression>() ) == idx1.size() )
      {
	for ( unsigned i = 0u; i < idx1.size(); ++i )
	{
	  all_numbers1 &= !dynamic_cast<syrec::numeric_expression*>( idx1.at( i ).get() )->value()->is_loop_variable();
	}
      }
      else
      {
	all_numbers1 = false;
      }
      if ( (unsigned)boost::count_if( idx2, is_type<syrec::numeric_expression>() ) == idx2.size() )
      {
	for ( unsigned i = 0u; i < idx2.size(); ++i )
	{
	  all_numbers2 &= !dynamic_cast<syrec::numeric_expression*>( idx2.at( i ).get() )->value()->is_loop_variable();
	}
      }
      else
      {
	all_numbers2 = false;
      }
      if ( !all_numbers1 && !all_numbers2 ) return false;
      if ( all_numbers1 && !all_numbers2 ) return true;
      if ( !all_numbers1 && all_numbers2 ) return false;
      for ( unsigned i = 0u; i < idx1.size(); ++i )
      {
	unsigned n1 = boost::lexical_cast<unsigned>( *( dynamic_cast<syrec::numeric_expression*>( idx1.at( i ).get() )->value() ) );
	unsigned n2 = boost::lexical_cast<unsigned>( *( dynamic_cast<syrec::numeric_expression*>( idx2.at( i ).get() )->value() ) );
	if ( n1 < n2 ) return true;
	if ( n1 > n2 ) return false;
      }
      return false;
    }
    
    static bool less( const boost::optional<std::pair<syrec::number::ptr, syrec::number::ptr> >& range1, const boost::optional<std::pair<syrec::number::ptr, syrec::number::ptr> >& range2 )
    {
      if ( !range1 ) return range2;
      if ( !range2 ) return false;

      std::string tf = boost::lexical_cast<std::string>( *range1->first  );
      std::string ts = boost::lexical_cast<std::string>( *range1->second );
      std::string rf = boost::lexical_cast<std::string>( *range2->first );
      std::string rs = boost::lexical_cast<std::string>( *range2->second );
      return ( ( tf < rf ) || ( ( tf == rf ) && ( ts < rs ) ) );
    }
    
    static bool equal( const boost::optional<std::pair<syrec::number::ptr, syrec::number::ptr> >& range1, const boost::optional<std::pair<syrec::number::ptr, syrec::number::ptr> >& range2 ) 
    {
      return ( !( less( range1, range2 ) ) && !( less( range2, range1 ) ) );
    }
    
    struct cmp_vptr
    {
      bool operator()(const std::pair< syrec::variable_access::ptr, unsigned > a, const std::pair< syrec::variable_access::ptr, unsigned > b) const
       {
         return ( ( a.first->var()->name() < b.first->var()->name() ) || ( ( a.first->var()->name() == b.first->var()->name() ) && ( a.second < b.second ) ) 
	   || ( ( a.first->var()->name() == b.first->var()->name() ) && ( a.second == b.second ) && less( a.first->indexes(), b.first->indexes() ) )
	   || ( ( a.first->var()->name() == b.first->var()->name() ) && ( a.second == b.second ) && equal( a.first->indexes(), b.first->indexes() ) && less( a.first->range(), b.first->range() ) )
	);
       }
    };*/

        struct cmp_vptr {
            bool operator()(const std::pair<syrec::variable_access::ptr, unsigned> a, const std::pair<syrec::variable_access::ptr, unsigned> b) {
                return ((a.first->var() < b.first->var()) || ((a.first->var() == b.first->var()) && (a.second < b.second)));
            }
        };

        std::map<std::pair<syrec::variable_access::ptr, unsigned>, syrec::variable_access::ptr, cmp_vptr> dupl_if_var_mapping; // duplication if: mapping of variables to there duplicates

        bool get_dupl_variables(syrec::variable_access::ptr var, std::vector<unsigned>& lines, std::map<std::pair<syrec::variable_access::ptr, unsigned>, syrec::variable_access::ptr, cmp_vptr> dupl_mapping);

        typedef std::set<syrec::variable_access::ptr, syrec::set_comperator> var_set;

        std::map<const syrec::statement*, var_set> _changing_variables; // for if_realization_duplication
        //std::stack< syrec::module::ptr > modules;

        syrec::variable_access::ptr get_dupl(syrec::variable_access::ptr var);

        void compute_changing_variables(const syrec::program& program, std::map<const syrec::statement*, var_set>& changing_variables);
        void compute_changing_variables(const syrec::module::ptr module, std::map<const syrec::statement*, var_set>& changing_variables);
        void compute_changing_variables(const syrec::statement::ptr statement, std::map<const syrec::statement*, var_set>& changing_variables);
    };

    /**
   * @brief garbage-free SyReC Synthesis
   *
   * TODO
   *
   * @author RevKit
   * @since  1.1
   */
    bool syrec_synthesis_garbagefree(circuit& circ, const syrec::program& program, properties::ptr settings = properties::ptr(), properties::ptr statistics = properties::ptr());

} // namespace revkit

#endif /* SYREC_SYNTHESIS_GARBAGEFREE_HPP */
