#ifndef ORIGINAL_SYREC_HPP
#define ORIGINAL_SYREC_HPP

#include "core/circuit.hpp"
#include "core/properties.hpp"
#include "core/syrec/program.hpp"

#include <boost/dynamic_bitset.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <cmath>
#include <stack>

namespace syrec::internal {
    struct nodePropAdditionalLines {
        nodePropAdditionalLines() = default;

        unsigned                 control{};
        gate::line_container     controls;
        std::shared_ptr<circuit> circ;
    };

    using cctAdditionalLines = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, boost::property<boost::vertex_name_t, nodePropAdditionalLines>>;

    using cct_node = boost::graph_traits<cctAdditionalLines>::vertex_descriptor;

    struct cctManagerAdditionallines {
        cctAdditionalLines tree;
        cct_node           current;
        cct_node           root;
    };
} // namespace syrec::internal

namespace syrec {
    using namespace internal;

    class standardSyrecSynthesizerAdditionalLines {
    public:
        using var_lines_map = std::map<variable::ptr, unsigned int>;

        standardSyrecSynthesizerAdditionalLines(circuit& circ, const syrec::program& prog); //constructor

        virtual ~standardSyrecSynthesizerAdditionalLines() = default;

        virtual bool on_module(const module::ptr& main);
        virtual bool on_statement(const statement::ptr& statement);
        virtual bool on_expression(const expression::ptr& expression, std::vector<unsigned>& lines);

        virtual void set_main_module(const module::ptr& main_module);

        virtual void add_variables(circuit& circ, const variable::vec& variables);

    protected:
        // statements
        virtual bool on_statement(const swap_statement& statement);
        virtual bool on_statement(const unary_statement& statement);
        virtual bool on_statement(const assign_statement& statement);
        virtual bool on_statement(const if_statement& statement);
        virtual bool on_statement(const for_statement& statement);
        virtual bool on_statement(const call_statement& statement);
        virtual bool on_statement(const uncall_statement& statement);
        virtual bool on_statement(const skip_statement& statement);

        // expressions
        virtual bool on_expression(const numeric_expression& expression, std::vector<unsigned>& lines);
        virtual bool on_expression(const variable_expression& expression, std::vector<unsigned>& lines);
        virtual bool on_expression(const binary_expression& expression, std::vector<unsigned>& lines);
        virtual bool on_expression(const shift_expression& expression, std::vector<unsigned>& lines);

        // unary operations
        virtual bool bitwise_negation(const std::vector<unsigned>& dest); // ~ TODO: test
        virtual bool decrement(const std::vector<unsigned>& dest);        // -- TODO: test
        virtual bool increment(const std::vector<unsigned>& dest);        // ++ TODO: test

        // binary operations
        virtual bool bitwise_and(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2); // & TODO: test
        virtual bool bitwise_cnot(const std::vector<unsigned>& dest, const std::vector<unsigned>& src);                                    // ^=
        virtual bool bitwise_or(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);  // & TODO: test
        virtual bool conjunction(unsigned dest, unsigned src1, unsigned src2);                                                             // && TODO: test
        virtual bool decrease(const std::vector<unsigned>& dest, const std::vector<unsigned>& src);                                        // -=
        virtual bool decrease_with_carry(const std::vector<unsigned>& dest, const std::vector<unsigned>& src, unsigned carry);
        virtual bool disjunction(unsigned dest, unsigned src1, unsigned src2);                                                          // || TODO: test
        virtual bool division(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2); // /
        virtual bool equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                       // =
        virtual bool greater_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);               // > TODO: test
        virtual bool greater_than(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                 // > TODO: test
        virtual bool increase(const std::vector<unsigned>& rhs, const std::vector<unsigned>& lhs);                                      // +=
        virtual bool increase_with_carry(const std::vector<unsigned>& dest, const std::vector<unsigned>& src, unsigned carry);
        virtual bool less_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                        // <= TODO: test
        virtual bool less_than(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                          // < TODO: test
        virtual bool modulo(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);         // % TODO: testen
        virtual bool multiplication(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2); // *
        virtual bool not_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2);                         // !=
        virtual bool swap(const std::vector<unsigned>& dest1, const std::vector<unsigned>& dest2);                                            // <=>

        // shift operations
        virtual bool left_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2);  // << TODO: testen
        virtual bool right_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2); // >> TODO: testen

        // efficient controls
        virtual bool add_active_control(unsigned);
        virtual bool remove_active_control(unsigned);

        virtual bool assemble_circuit(const cct_node&);

        cctManagerAdditionallines cct_man;

        virtual void     get_variables(const variable_access::ptr& var, std::vector<unsigned>& lines);
        virtual unsigned get_constant_line(bool value);
        virtual void     get_constant_lines(unsigned bitwidth, unsigned value, std::vector<unsigned>& lines);
        virtual void     add_variable(circuit& circ, const std::vector<unsigned>& dimensions, const variable::ptr& var, constant _constant, bool _garbage, const std::string& arraystr);

    private:
        circuit&                              _circ;
        std::stack<statement::ptr>            _stmts;
        var_lines_map                         _var_lines;
        std::map<bool, std::vector<unsigned>> free_const_lines_map;
        number::loop_variable_mapping         loop_map;

        std::stack<module::ptr> modules;
    };

    bool synthesisAdditionalLines(circuit& circ, const program& program, const properties::ptr& settings = std::make_shared<properties>(), const properties::ptr& statistics = std::make_shared<properties>());

} // namespace syrec

#endif /* ORIGINAL_SYREC_HPP */
