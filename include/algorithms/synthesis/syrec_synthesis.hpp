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

    typedef boost::graph_traits<cct>::vertex_descriptor                cct_node;
    [[maybe_unused]] typedef boost::graph_traits<cct>::edge_descriptor cct_edge;

    struct cct_manager {
        cct      tree;
        cct_node current;
        cct_node root;
    };
} // namespace syrec::internal

namespace syrec {

    namespace applications {
        struct set_comperator {
            bool operator()(const applications::variable_access::ptr& var1, const applications::variable_access::ptr& var2) const {
                return var1->var().get() < var2->var().get();
            }
        };

        class program;
    } // namespace applications
    using namespace internal;


    class standard_syrec_synthesizer {
    public:
        typedef std::map<applications::variable::ptr, unsigned> var_lines_map;

        standard_syrec_synthesizer(circuit& circ, const applications::program& prog);

        virtual ~standard_syrec_synthesizer() = default;

        virtual bool on_module(const applications::module::ptr&);
        virtual bool on_statement(const applications::statement::ptr& statement);
        bool         on_expression(const applications::expression::ptr& expression, std::vector<unsigned>& lines, std::vector<unsigned>& lhs_stat, unsigned op);
        [[maybe_unused]] bool var_expression(const applications::expression::ptr& expression, std::vector<unsigned>& v); //new//new
        bool         op_rhs_lhs_expression(const applications::expression::ptr& expression, std::vector<unsigned>& v); //new
        bool         full_statement(const applications::statement::ptr& statement);                                    //new
        bool         flow(const applications::expression::ptr& expression, std::vector<unsigned>& v);                  // new
        virtual void set_settings(const properties::ptr& settings);
        virtual void set_main_module(const applications::module::ptr& main_module);
        bool         solver(const std::vector<unsigned>& stat_lhs, unsigned stat_op, const std::vector<unsigned>& exp_lhs, unsigned exp_op, const std::vector<unsigned>& exp_rhs);
        // Virtual Methods to override for custom synthesizers
    protected:
        // statements
        virtual bool on_statement(const applications::swap_statement& statement);
        virtual bool on_statement(const applications::unary_statement& statement);
        virtual bool on_statement(const applications::assign_statement& statement);
        virtual bool on_statement(const applications::if_statement& statement);
        virtual bool on_statement(const applications::for_statement& statement);
        virtual bool on_statement(const applications::call_statement& statement);
        virtual bool on_statement(const applications::uncall_statement& statement);
        virtual bool on_statement(const applications::skip_statement& statement);
        //virtual bool on_full_statement(const applications::assign_statement& statement);                                   //new
        virtual bool var_expression(const applications::variable_expression& expression, std::vector<unsigned>& v);        //new
        virtual bool op_rhs_lhs_expression(const applications::variable_expression& expression, std::vector<unsigned>& v); //new
        virtual bool op_rhs_lhs_expression(const applications::binary_expression& expression, std::vector<unsigned>& v);   //new
        virtual bool full_statement(const applications::assign_statement& statement);                                      //new

        // expressions
        virtual bool on_expression(const applications::numeric_expression& expression, std::vector<unsigned>& lines, std::vector<unsigned>& lhs_stat, unsigned op);
        virtual bool on_expression(const applications::variable_expression& expression, std::vector<unsigned>& lines, std::vector<unsigned>& lhs_stat, unsigned op);
        virtual bool on_expression(const applications::binary_expression& expression, std::vector<unsigned>& lines, std::vector<unsigned>& lhs_stat, unsigned op);
        virtual bool on_expression(const applications::shift_expression& expression, std::vector<unsigned>& lines, std::vector<unsigned>& lhs_stat, unsigned op);

        virtual bool flow(const applications::variable_expression& expression, std::vector<unsigned>& v); //new
        virtual bool flow(const applications::binary_expression& expression, std::vector<unsigned>& v);   //new
                                                                                                          // Helper methods (can also be used by custom synthesizers)
    protected:
        virtual // unary operations
                bool
                     bitwise_negation(const std::vector<unsigned>& dest); // ~
        virtual bool decrement(const std::vector<unsigned>& dest);        // --
        virtual bool increment(const std::vector<unsigned>& dest); // ++

        virtual // binary operations
                bool
                     bitwise_and(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2); // &
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
        virtual bool not_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2); // !=
        virtual bool swap(const std::vector<unsigned>& dest1, const std::vector<unsigned>& dest2);                    // <=>
        static bool check_repeats();


        bool decrease_new(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs);
        bool decrease_new_assign(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs);
        bool increase_new(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs);
        bool expression_op_inverse(unsigned op, const std::vector<unsigned>& exp_lhs, const std::vector<unsigned>& exp_rhs);
        bool expression_single_op(unsigned op, const std::vector<unsigned>& exp_lhs, const std::vector<unsigned>& exp_rhs);
        bool exp_evaluate(std::vector<unsigned>& lines, unsigned op, const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs);
                // shift operations
                bool
                     left_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2);  // <<
        virtual bool right_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2); // >>

        // efficient controls
        bool add_active_control(unsigned);
        bool remove_active_control(unsigned);

        virtual bool assemble_circuit(const cct_node&);


        cct_manager cct_man;

    public:
        var_lines_map& var_lines();


    protected:


        virtual bool get_variables(applications::variable_access::ptr var, std::vector<unsigned>& lines);
        bool         unget_variables(const applications::variable_access::ptr& var, std::vector<unsigned>& lines);

        unsigned get_constant_line(bool value);
        bool     get_constant_lines(unsigned bitwidth, unsigned value, std::vector<unsigned>& lines);


    private:
        circuit&                                    _circ;
        properties::ptr                             _settings; // Settings to use them recursively in module call
        std::stack<applications::statement::ptr>    _stmts;
        var_lines_map                               _var_lines;
        std::map<bool, std::vector<unsigned>>       free_const_lines_map; // TODO: set statt vector?
        applications::number::loop_variable_mapping loop_map;             // TODO: umbenennen: intern_variable_mapping oder aehnlich


        typedef std::set<applications::variable_access::ptr, applications::set_comperator> var_set;

        std::stack<applications::module::ptr> modules;

        std::string variable_name_format;

        /**
     * @brief number of merged control lines for in-/decrement
     *
     * in the realization of the in-/decrement this number of control lines will be merged to save quantum and transistor costs; but one additional constant line is needed for that, but is released after the process.
     * if set to less than 2 the merging is deactivated
     * default value is 8.
     *

     */
        unsigned crement_merge_line_count{};

        unsigned if_realization{};

        bool efficient_controls{};
    };

    /**
   * @brief IF statement realization
   *

   */
    enum {
        /**
     * @brief Realization by adding if condition as controlled line
     */
        syrec_synthesis_if_realization_controlled,

        /**
     * @brief Realization by using duplicated variables in else block
     */
        syrec_synthesis_if_realization_duplication
    };

    /**
   * @brief SyReC Synthesis
   *
   * TODO
   *

   */
    bool syrec_synthesis(circuit& circ, const applications::program& program, const properties::ptr& settings = std::make_shared<properties>(), const properties::ptr& statistics = std::make_shared<properties>());

    /**
   * @brief Functor for the syrec_synthesis algorithm
   *
   * @param settings Settings (see syrec_synthesis)
   * @param statistics Statistics (see syrec_synthesis)
   *
   * @return A functor which complies with the hdl_synthesis_func interface
   *

   */

} // namespace syrec

#endif /* SYREC_SYNTHESIS_HPP */
