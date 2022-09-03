#pragma once

#include "core/circuit.hpp"
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

    using cct      = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, boost::property<boost::vertex_name_t, node_properties>>;
    using cct_node = boost::graph_traits<cct>::vertex_descriptor;

    struct cct_manager {
        cct      tree;
        cct_node current;
        cct_node root;
    };
} // namespace syrec::internal

namespace syrec {
    using namespace internal;

    class SyrecSynthesis {
    public:
        std::stack<unsigned>               exp_opp;
        std::stack<std::vector<unsigned>>  exp_lhss;
        std::stack<std::vector<unsigned>>  exp_rhss;
        bool                               sub_flag = false;
        std::vector<unsigned>              op_vec;
        std::vector<unsigned>              assign_op_vector;
        std::vector<unsigned>              exp_op_vector;
        std::vector<std::vector<unsigned>> exp_lhs_vector;
        std::vector<std::vector<unsigned>> exp_rhs_vector;

        using var_lines_map = std::map<variable::ptr, unsigned int>;

        explicit SyrecSynthesis(circuit& circ);
        virtual ~SyrecSynthesis() = default;

        void add_variables(circuit& circ, const variable::vec& variables);
        void set_main_module(const module::ptr& main_module);

    protected:
        bool               on_statement(const swap_statement& statement);
        bool               on_statement(const unary_statement& statement);
        [[nodiscard]] bool on_statement(const skip_statement& statement) const;

        bool on_expression(const numeric_expression& expression, std::vector<unsigned>& lines);
        bool on_expression(const variable_expression& expression, std::vector<unsigned>& lines);

        // unary operations
        bool bitwise_negation(const std::vector<unsigned>& dest); // ~
        bool decrement(const std::vector<unsigned>& dest);        // --
        bool increment(const std::vector<unsigned>& dest);        // ++

        // binary operations
        bool bitwise_and(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2); // &
        bool bitwise_cnot(const std::vector<unsigned>& dest, const std::vector<unsigned>& src);                                    // ^=
        bool bitwise_or(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);  // &
        bool conjunction(unsigned dest, unsigned src1, unsigned src2);                                                             // &&// -=
        bool decrease_with_carry(const std::vector<unsigned>& dest, const std::vector<unsigned>& src, unsigned carry);
        bool disjunction(unsigned dest, unsigned src1, unsigned src2);                                                          // ||
        bool division(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2); // /
        bool equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                       // =
        bool greater_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);               // >
        bool greater_than(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                 // >// +=
        bool increase_with_carry(const std::vector<unsigned>& dest, const std::vector<unsigned>& src, unsigned carry);
        bool less_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                        // <=
        bool less_than(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                          // <
        bool modulo(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);         // %
        bool multiplication(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2); // *
        bool not_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                         // !=
        void swap(const std::vector<unsigned>& dest1, const std::vector<unsigned>& dest2);                                            // <=>
        bool decrease(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs);
        bool increase(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs);
        bool check_repeats();

        // shift operations
        void left_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2);  // <<
        void right_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2); // >>

        void add_active_control(unsigned);
        void remove_active_control(unsigned);

        bool assemble_circuit(const cct_node&);

        cct_manager cct_man;

        void add_variable(circuit& circ, const std::vector<unsigned>& dimensions, const variable::ptr& var, constant _constant, bool _garbage, const std::string& arraystr);
        void get_variables(const variable_access::ptr& var, std::vector<unsigned>& lines);

        unsigned get_constant_line(bool value);
        void     get_constant_lines(unsigned bitwidth, unsigned value, std::vector<unsigned>& lines);

        std::stack<statement::ptr>    _stmts;
        circuit&                      _circ;
        number::loop_variable_mapping loop_map;
        std::stack<module::ptr>       modules;

    private:
        var_lines_map                         _var_lines;
        std::map<bool, std::vector<unsigned>> free_const_lines_map;
    };

} // namespace syrec
