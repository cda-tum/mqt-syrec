/**
* @file syrec_synthesis.hpp
*
* @brief SyReC Synthesis
*/

#ifndef SYREC_SYNTHESIS_HPP
#define SYREC_SYNTHESIS_HPP

#include "core/circuit.hpp"
#include "core/functions/add_circuit.hpp"
#include "core/gate.hpp"
#include "core/properties.hpp"
#include "core/syrec/expression.hpp"
#include "core/syrec/program.hpp"

#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <cmath>
#include <memory>
#include <stack>

namespace syrec::internal {
   struct node_properties {
       node_properties() = default;

       unsigned                 control{};
       gate::line_container     controls;
       std::shared_ptr<circuit> circ;
   };

   typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
                                 boost::property<boost::vertex_name_t, node_properties>>
           cct;

   typedef boost::graph_traits<cct>::vertex_descriptor cct_node;

   struct cct_manager {
       cct      tree;
       cct_node current;
       cct_node root;
   };
} // namespace syrec::internal

namespace syrec {
   using namespace internal;

   class standard_syrec_synthesizer {
   public:
       std::stack<unsigned>               exp_opp;
       std::stack<std::vector<unsigned>>  exp_lhss, exp_rhss;
       bool                               sub_flag = false;
       std::vector<unsigned>              op_vec, assign_op_vector, exp_op_vector;
       std::vector<std::vector<unsigned>> exp_lhs_vector, exp_rhs_vector;

       typedef std::map<applications::variable::ptr, unsigned> var_lines_map;

       standard_syrec_synthesizer(circuit& circ, const applications::program& prog);

       virtual ~standard_syrec_synthesizer() = default;

       virtual void add_variables(circuit& circ, const applications::variable::vec& variables);
       virtual bool on_module(const applications::module::ptr&);
       virtual bool on_statement(const applications::statement::ptr& statement);
       virtual bool on_expression(const applications::expression::ptr& expression, std::vector<unsigned>& lines, std::vector<unsigned>& lhs_stat, unsigned op);
       virtual bool op_rhs_lhs_expression(const applications::expression::ptr& expression, std::vector<unsigned>& v);
       virtual bool full_statement(const applications::statement::ptr& statement);
       virtual bool flow(const applications::expression::ptr& expression, std::vector<unsigned>& v);
       virtual void set_main_module(const applications::module::ptr& main_module);
       virtual bool solver(const std::vector<unsigned>& stat_lhs, unsigned stat_op, const std::vector<unsigned>& exp_lhs, unsigned exp_op, const std::vector<unsigned>& exp_rhs);

   protected:
       virtual bool on_statement(const applications::swap_statement& statement);
       virtual bool on_statement(const applications::unary_statement& statement);
       virtual bool on_statement(const applications::assign_statement& statement);
       virtual bool on_statement(const applications::if_statement& statement);
       virtual bool on_statement(const applications::for_statement& statement);
       virtual bool on_statement(const applications::call_statement& statement);
       virtual bool on_statement(const applications::uncall_statement& statement);
       virtual bool on_statement(const applications::skip_statement& statement);
       virtual bool op_rhs_lhs_expression(const applications::variable_expression& expression, std::vector<unsigned>& v);
       virtual bool op_rhs_lhs_expression(const applications::binary_expression& expression, std::vector<unsigned>& v);
       virtual bool full_statement(const applications::assign_statement& statement);

       // expressions
       virtual bool on_expression(const applications::numeric_expression& expression, std::vector<unsigned>& lines);
       virtual bool on_expression(const applications::variable_expression& expression, std::vector<unsigned>& lines);
       virtual bool on_expression(const applications::binary_expression& expression, std::vector<unsigned>& lines, std::vector<unsigned>& lhs_stat, unsigned op);
       virtual bool on_expression(const applications::shift_expression& expression, std::vector<unsigned>& lines, std::vector<unsigned>& lhs_stat, unsigned op);

       virtual bool flow(const applications::variable_expression& expression, std::vector<unsigned>& v);
       virtual bool flow(const applications::binary_expression& expression, std::vector<unsigned>& v);

       // unary operations
       virtual bool bitwise_negation(const std::vector<unsigned>& dest); // ~
       virtual bool decrement(const std::vector<unsigned>& dest);        // --
       virtual bool increment(const std::vector<unsigned>& dest);        // ++

       // binary operations
       virtual bool bitwise_and(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2); // &
       virtual bool bitwise_cnot(const std::vector<unsigned>& dest, const std::vector<unsigned>& src);                                    // ^=
       virtual bool bitwise_or(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);  // &
       virtual bool conjunction(unsigned dest, unsigned src1, unsigned src2);                                                             // &&// -=
       virtual bool decrease_with_carry(const std::vector<unsigned>& dest, const std::vector<unsigned>& src, unsigned carry);
       virtual bool disjunction(unsigned dest, unsigned src1, unsigned src2);                                                          // ||
       virtual bool division(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2); // /
       virtual bool equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                       // =
       virtual bool greater_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);               // >
       virtual bool greater_than(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                 // >// +=
       virtual bool increase_with_carry(const std::vector<unsigned>& dest, const std::vector<unsigned>& src, unsigned carry);
       virtual bool less_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                        // <=
       virtual bool less_than(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                          // <
       virtual bool modulo(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);         // %
       virtual bool multiplication(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2); // *
       virtual bool not_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                         // !=
       virtual void swap(const std::vector<unsigned>& dest1, const std::vector<unsigned>& dest2);                                            // <=>
       virtual bool check_repeats();

       virtual bool decrease_new(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs);
       virtual bool decrease_new_assign(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs);
       virtual bool increase_new(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs);
       virtual bool expression_op_inverse(unsigned op, const std::vector<unsigned>& exp_lhs, const std::vector<unsigned>& exp_rhs);
       virtual bool expression_single_op(unsigned op, const std::vector<unsigned>& exp_lhs, const std::vector<unsigned>& exp_rhs);
       virtual bool exp_evaluate(std::vector<unsigned>& lines, unsigned op, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs);
       // shift operations
       virtual void left_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2);  // <<
       virtual void right_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2); // >>

       // efficient controls
       virtual void add_active_control(unsigned);
       virtual void remove_active_control(unsigned);

       virtual bool assemble_circuit(const cct_node&);

       cct_manager cct_man;

       virtual void add_variable(circuit& circ, const std::vector<unsigned>& dimensions, const applications::variable::ptr& var, constant _constant, bool _garbage, const std::string& arraystr);
       virtual void get_variables(const applications::variable_access::ptr& var, std::vector<unsigned>& lines);

       virtual unsigned get_constant_line(bool value);
       virtual void     get_constant_lines(unsigned bitwidth, unsigned value, std::vector<unsigned>& lines);

   private:
       circuit&                                    _circ;
       std::stack<applications::statement::ptr>    _stmts;
       var_lines_map                               _var_lines;
       std::map<bool, std::vector<unsigned>>       free_const_lines_map;
       applications::number::loop_variable_mapping loop_map;

       std::stack<applications::module::ptr> modules;
   };

   /**
  * @brief SyReC Synthesis
  *
  *
  *
  */
   bool syrec_synthesis(circuit& circ, const applications::program& program, const properties::ptr& settings = std::make_shared<properties>(), const properties::ptr& statistics = std::make_shared<properties>());
} // namespace syrec

#endif /* SYREC_SYNTHESIS_HPP */