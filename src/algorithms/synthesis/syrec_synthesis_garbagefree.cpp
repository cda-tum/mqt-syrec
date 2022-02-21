#include "algorithms/synthesis/syrec_synthesis_garbagefree.hpp"

#include <boost/assign.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <core/circuit.hpp>
#include <core/functions/add_gates.hpp>
#include <core/functions/add_line_to_circuit.hpp>
#include <core/functor.hpp>
#include <core/gate.hpp>
#include <core/syrec/program.hpp>
#include <core/syrec/reverse_statements.hpp>
#include <core/syrec/variable.hpp>
#include <core/utils/costs.hpp>
#include <core/utils/timer.hpp>
#include <memory>

//#define foreach_ BOOST_FOREACH
#define reverse_foreach_ BOOST_REVERSE_FOREACH
#define UNUSED(x) (void)(x)

using namespace boost::assign;

namespace revkit {

    garbagefree_syrec_synthesizer::garbagefree_syrec_synthesizer(circuit& circ, const syrec::program& prog):
        standard_syrec_synthesizer(circ, prog) {
        depth      = 0;
        dupl_count = 0;
        //used_const_lines.push_back( std::map< bool, std::vector<unsigned> >() );
        //inv_used_const_lines.push_back( std::map< bool, std::vector<unsigned> >() );
        //var_used_const_lines.push_back( std::map< bool, std::vector<unsigned> >() );
        //var_inv_used_const_lines.push_back( std::map< bool, std::vector<unsigned> >() );
    }

    void garbagefree_syrec_synthesizer::set_settings(properties::ptr settings) {
        _settings = settings;

        variable_name_format = get<std::string>(settings, "variable_name_format", "%1$s%3$s.%2$d");
        if_realization       = get<unsigned>(settings, "if_realization", syrec_synthesis_if_realization_controlled);
        efficient_controls   = get<bool>(settings, "efficient_controls", true);
        garbagefree          = get<bool>(settings, "garbage-free", true);
        if (garbagefree && (if_realization == syrec_synthesis_if_realization_duplication)) {
            std::cerr << "Warning: The selected if-realization will produce garbage outputs. " << std::endl;
        }
    }

    void garbagefree_syrec_synthesizer::set_main_module(syrec::module::ptr main_module) {
        assert(modules.empty());
        modules.push(main_module);
    }

    bool garbagefree_syrec_synthesizer::on_module(syrec::module::ptr main) {
        //std::cout << "on_module " << main->name() << std::endl;
        for (syrec::statement::ptr stat: main->statements()) {
            //std::cout << "on_statement " << *stat << std::endl;
            if (!on_statement(stat)) return false;
        }
        free_const_lines.clear();
        return assemble_circuit(cct_man.root);
    }

    // TODO sind lines und const_lines beide notwendig?
    //TODO muss bei den generellen Sachen (vor on_statement) noch was geändert werden?)

    bool garbagefree_syrec_synthesizer::on_statement(syrec::statement::ptr statement) {
        stmts().push(statement);
        //depth++;
        /*if( used_const_lines.size() <= depth){
      used_const_lines.push_back( std::map< bool, std::vector<unsigned> >() );
    }
    if( inv_used_const_lines.size() <= depth){
      inv_used_const_lines.push_back( std::map< bool, std::vector<unsigned> >() );
    }
    if( var_used_const_lines.size() <= depth){
      var_used_const_lines.push_back( std::map< bool, std::vector<unsigned> >() );
    }
    if( var_inv_used_const_lines.size() <= depth){
      var_inv_used_const_lines.push_back( std::map< bool, std::vector<unsigned> >() );
    }*/
        bool okay = false;
        if (auto* stat = dynamic_cast<syrec::swap_statement*>(statement.get())) {
            okay = on_statement(*stat);
        } else if (auto* stat = dynamic_cast<syrec::unary_statement*>(statement.get())) {
            okay = on_statement(*stat);
        } else if (auto* stat = dynamic_cast<syrec::assign_statement*>(statement.get())) {
            okay = on_statement(*stat);
        } else if (auto* stat = dynamic_cast<syrec::if_statement*>(statement.get())) {
            okay = on_statement(*stat);
        } else if (auto* stat = dynamic_cast<syrec::for_statement*>(statement.get())) {
            okay = on_statement(*stat);
        } else if (auto* stat = dynamic_cast<syrec::call_statement*>(statement.get())) {
            okay = on_statement(*stat);
        } else if (auto* stat = dynamic_cast<syrec::uncall_statement*>(statement.get())) {
            okay = on_statement(*stat);
        } else if (auto* stat = dynamic_cast<syrec::skip_statement*>(statement.get())) {
            okay = on_statement(*stat);
        } else {
            std::cout << "Synthesize" << std::endl;
            return false;
        }

        stmts().pop();
        //depth--;
        return okay;
    }

    bool garbagefree_syrec_synthesizer::on_statement(const syrec::swap_statement& statement) {
        // altesTODO: wenn keine controlling line aktiv, einfach die Lines tauschen statt dem Fredkin-Gate
        std::vector<unsigned>       lhs, rhs, lhs2, rhs2;
        syrec::variable_access::ptr stmt_lhs = evaluate_to_numeric_expression(statement.lhs());
        syrec::variable_access::ptr stmt_rhs = evaluate_to_numeric_expression(statement.rhs());
        get_variables(stmt_lhs, lhs);
        get_variables(stmt_rhs, rhs);

        assert(lhs.size() == rhs.size());

        swap(lhs, rhs);

        std::list<boost::tuple<bool, bool, unsigned>> rhs_cl, lhs_cl;
        get_expr_lines(stmt_rhs, rhs2, rhs_cl);
        if (!unget_variables(stmt_rhs, rhs2, rhs_cl)) return false;
        get_expr_lines(stmt_lhs, lhs2, lhs_cl);
        return unget_variables(stmt_lhs, lhs2, lhs_cl);
    }

    bool garbagefree_syrec_synthesizer::on_statement(const syrec::unary_statement& statement) {
        std::vector<unsigned>       var, var2;
        syrec::variable_access::ptr stmt_var = evaluate_to_numeric_expression(statement.var());
        get_variables(stmt_var, var);

        switch (statement.op()) {
            case syrec::unary_statement::invert: {
                bitwise_negation(var);
            } break;

            case syrec::unary_statement::increment: {
                increment(var);
            } break;

            case syrec::unary_statement::decrement: {
                decrement(var);
            } break;

            default: {
                std::cout << "Synthesize" << std::endl;
                return false;
            }
        }

        std::list<boost::tuple<bool, bool, unsigned>> var_cl;
        get_expr_lines(stmt_var, var2, var_cl);
        return unget_variables(stmt_var, var2, var_cl);
    }

    bool garbagefree_syrec_synthesizer::on_statement(const syrec::assign_statement& statement) {
        std::vector<unsigned>       lhs, rhs;
        syrec::variable_access::ptr stmt_lhs = evaluate_to_numeric_expression(statement.lhs());
        syrec::expression::ptr      stmt_rhs = evaluate_to_numeric_expression(statement.rhs());
        //std::cout << "getting variables" << std::endl;
        get_variables(stmt_lhs, lhs);

        //std::cout << "on expression" << std::endl;
        on_expression(stmt_rhs, rhs);

        assert(lhs.size() == rhs.size());

        bool status = false;
        //std::cout << "assign gates" << std::endl;
        switch (statement.op()) {
            case syrec::assign_statement::add: {
                status = increase(lhs, rhs);
            } break;

            case syrec::assign_statement::subtract: {
                status = decrease(lhs, rhs);
            } break;

            case syrec::assign_statement::exor: {
                status = bitwise_cnot(lhs, rhs);
            } break;

            default:
                std::cout << "Assign Statement" << std::endl;
                return false;
        }

        std::vector<unsigned>                         rhs2, lhs2;
        std::list<boost::tuple<bool, bool, unsigned>> rhs_cl, lhs_cl;
        get_expr_lines(stmt_rhs, rhs2, rhs_cl);
        // rhs2 ist an sich überflüssig, aber wegen Rekursion, und hier zum debuggen:
        // -> bei modulo ist es nicht überflüssig
        /*std::cout << "rhs : ";
    for (std::vector<unsigned>::iterator it = rhs.begin(); it != rhs.end(); ++it ) 
    {
      std::cout << *it << " ";
    }
    std::cout << std::endl << "rhs2: ";
    for (std::vector<unsigned>::iterator it = rhs2.begin(); it != rhs2.end(); ++it ) 
    {
      std::cout << *it << " ";
    }
    std::cout << std::endl;*/
        status &= off_expression(stmt_rhs, rhs2, rhs_cl);
        get_expr_lines(stmt_lhs, lhs2, lhs_cl);
        status &= unget_variables(stmt_lhs, lhs2, lhs_cl);

        return status;
    }

    // helper function (copied from syrec_synthesis)
    void _add_variable2(circuit& circ, const std::vector<unsigned>& dimensions, const syrec::variable::ptr& var, const std::string& variable_name_format,
                        constant _constant, bool _garbage, const std::string& arraystr) {
        if (dimensions.empty()) {
            for (unsigned i = 0u; i < var->bitwidth(); ++i) {
                std::string name = boost::str(boost::format(variable_name_format) % var->name() % i % arraystr);
                //std::string input_name = ( _constant ? ( std::string( "const_" ) + boost::lexical_cast<std::string>( *_constant ) ) : name );
                //std::string output_name = ( _garbage ? "garbage" : name );
                add_line_to_circuit(circ, name, name, _constant, _garbage);
                //std::cout << "added line to circuit" << std::endl;
            }

            // busses
            std::vector<unsigned> affected_lines;
            boost::push_back(affected_lines, boost::irange(circ.lines() - var->bitwidth(), circ.lines()));
            std::string name = boost::str(boost::format("%s%s") % var->name() % arraystr);

            if (var->type() == syrec::variable::in || var->type() == syrec::variable::inout) {
                circ.inputbuses().add(name, affected_lines);
            }
            if (var->type() == syrec::variable::out || var->type() == syrec::variable::inout) {
                circ.outputbuses().add(name, affected_lines);
            }
            if (var->type() == syrec::variable::state) {
                circ.statesignals().add(name, affected_lines);
            }
        } else {
            unsigned              len = dimensions.front();
            std::vector<unsigned> new_dimensions(dimensions.begin() + 1u, dimensions.end());

            for (unsigned i = 0u; i < len; ++i) {
                _add_variable2(circ, new_dimensions, var, variable_name_format,
                               _constant, _garbage, boost::str(boost::format("%s[%d]") % arraystr % i));
            }
        }
    }

    void add_variables2(circuit& circ, garbagefree_syrec_synthesizer& synthesizer, const std::string& variable_name_format, const syrec::variable::vec& variables) {
        for (syrec::variable::ptr var: variables) {
            // entry in var lines map
            synthesizer.var_lines().insert(std::make_pair(var, circ.lines()));

            // types of constant and garbage
            constant _constant = (var->type() == syrec::variable::out || var->type() == syrec::variable::wire) ? constant(false) : constant();
            bool     _garbage  = (var->type() == syrec::variable::in || var->type() == syrec::variable::wire);

            _add_variable2(circ, var->dimensions(), var, variable_name_format,
                           _constant, _garbage, std::string());
        }
    }

    bool garbagefree_syrec_synthesizer::on_statement(const syrec::if_statement& statement) {
        ++depth;
        // evaluate expression
        boost::tuple<bool, bool, unsigned> helper_line;

        syrec::expression::ptr condition    = evaluate_to_numeric_expression(statement.condition());
        syrec::expression::ptr fi_condition = evaluate_to_numeric_expression(statement.fi_condition());

        if (auto* cond = dynamic_cast<syrec::numeric_expression*>(condition.get())) {
            if (cond->value()->evaluate(intern_variable_mapping)) {
                for (syrec::statement::ptr stat: statement.then_statements()) {
                    if (!on_statement(stat)) return false;
                }
            } else {
                for (syrec::statement::ptr stat: statement.else_statements()) {
                    if (!on_statement(stat)) return false;
                }
            }
            return true;
        }

        get_reusable_constant_line(false);
        helper_line = used_const_lines.top();
        used_const_lines.pop();

        on_condition(condition, helper_line.get<2>());

        switch (if_realization) {
            case syrec_synthesis_if_realization_controlled: {
                // activate this line
                add_active_control(helper_line.get<2>());

                for (syrec::statement::ptr stat: statement.then_statements()) {
                    if (!on_statement(stat)) return false;
                }

                // toggle helper line
                remove_active_control(helper_line.get<2>());
                append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), helper_line.get<2>());
                add_active_control(helper_line.get<2>());

                for (syrec::statement::ptr stat: statement.else_statements()) {
                    if (!on_statement(stat)) return false;
                }

                // de-activate helper line
                remove_active_control(helper_line.get<2>());
                if (garbagefree) {
                    append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), helper_line.get<2>());
                    on_condition(fi_condition, helper_line.get<2>());
                    release_constant_line(helper_line);
                }

            } break;
            case syrec_synthesis_if_realization_duplication: {
                // std::cout << "if statement" << std::endl;
                std::map<std::pair<syrec::variable_access::ptr, unsigned>, syrec::variable_access::ptr, cmp_vptr> then_var_mapping = dupl_if_var_mapping;
                for (syrec::variable_access::ptr var: _changing_variables.find(&statement)->second) {
                    syrec::variable_access::ptr dupl_var = std::make_shared<syrec::variable_access>(std::make_shared<syrec::variable>(syrec::variable::wire, ("dupl_" + var->var()->name() + "_" + boost::lexical_cast<std::string>(dupl_count)), var->var()->dimensions(), var->var()->bitwidth()));
                    // NEU NEU NEU
                    dupl_var->set_range(var->range());
                    dupl_var->set_indexes(var->indexes());
                    // +++++++++++
                    syrec::variable::vec var_vec;
                    var_vec.push_back(dupl_var->var());

                    add_variables2(circ(), *this, variable_name_format, var_vec);

                    std::vector<unsigned> lhs, rhs, rhs2;
                    //std::cout << "GET VARIABLES" << std::endl;
                    //get_variables( dupl_var, lhs );
                    get_dupl_variables(dupl_var, lhs, dupl_if_var_mapping);
                    //std::cout << "got lhs" << std::endl;
                    //get_variables( var, rhs );
                    get_dupl_variables(var, rhs, dupl_if_var_mapping);
                    //std::cout << "got rhs" << std::endl;
                    bitwise_cnot(lhs, rhs);

                    then_var_mapping.insert(std::make_pair(std::make_pair(var, depth), dupl_var));
                }
                ++dupl_count;
                dupl_if_var_mapping.swap(then_var_mapping);
                //std::cout << "dupl_if_var_mapping:" << std::endl;
                //for ( std::map< std::pair< syrec::variable_access::ptr, unsigned >, syrec::variable_access::ptr, cmp_vptr >::iterator it = dupl_if_var_mapping.begin(); it != dupl_if_var_mapping.end(); ++it )
                //{
                //std::cout << it->first.first->var()->name() << ", " << it->first.second << ": " << it->second->var()->name() << std::endl;
                // }

                // then branch
                // std::cout << "then depth: " << depth << std::endl;
                for (syrec::statement::ptr stat: statement.then_statements()) {
                    if (!on_statement(stat)) return false;
                }

                dupl_if_var_mapping.swap(then_var_mapping);

                // else branch
                --depth;
                //  std::cout << "else depth: " << depth << std::endl;
                for (syrec::statement::ptr stat: statement.else_statements()) {
                    if (!on_statement(stat)) return false;
                }
                //  std::cout << "synthesized then stmts " << std::endl;
                add_active_control(helper_line.get<2>());
                // std::cout << "then_var_mapping:" << std::endl;
                // for ( std::map< std::pair< syrec::variable_access::ptr, unsigned >, syrec::variable_access::ptr, cmp_vptr >::iterator it = then_var_mapping.begin(); it != then_var_mapping.end(); ++it )
                // {
                //std::cout << it->first.first->var()->name() << ", " << it->first.second << ": " << it->second->var()->name() << std::endl;
                //  }
                for (syrec::variable_access::ptr var: _changing_variables.find(&statement)->second) {
                    std::vector<unsigned> lhs, rhs;
                    // std::cout << "search for " << var->var()->name() << ", " << (depth+1) << std::endl;
                    ++depth;
                    // syrec::variable_access::ptr var_tmp = then_var_mapping.find( std::make_pair( var, ( depth + 1 ) ) )->second;
                    //  std::cout << "getting variable lines" << std::endl;
                    get_dupl_variables(var, lhs, then_var_mapping);
                    --depth;
                    //std::cout << "lhs: ";
                    //	for ( unsigned i=0; i < lhs.size(); ++i)
                    //	{
                    //	  std::cout << lhs.at(i) << " ";
                    //	}
                    //	std::cout << std::endl;
                    get_dupl_variables(var, rhs, then_var_mapping);
                    //	std::cout << "rhs: ";
                    //	for ( unsigned i=0; i < rhs.size(); ++i)
                    //	{
                    //	  std::cout << rhs.at(i) << " ";
                    //	}
                    //	std::cout << std::endl;

                    swap(lhs, rhs);
                }
                //     std::cout << "swapped variables" << std::endl;
                remove_active_control(helper_line.get<2>());
                if (garbagefree) {
                    on_condition(fi_condition, helper_line.get<2>());
                    release_constant_line(helper_line);
                }
            } break;
            default:
                std::cout << "if Statement" << std::endl;
                return false;
        }

        return true;
    }

    bool garbagefree_syrec_synthesizer::on_statement(const syrec::for_statement& statement) {
        syrec::number::ptr nfrom, nto;
        boost::tie(nfrom, nto) = statement.range();

        unsigned from = nfrom ? nfrom->evaluate(intern_variable_mapping) : 1u; // default value is 1u
        unsigned to   = nto->evaluate(intern_variable_mapping);

        //assert( to >= from );
        unsigned step = 1u; // default step is +1
        bool     negative_step;
        //unsigned step = statement.step() ? statement.step()->evaluate( intern_variable_mapping ) : 1u; // default step is +1
        if (statement.step()) {
            step          = statement.step()->evaluate(intern_variable_mapping);
            negative_step = statement.is_negative_step();
        } else {
            negative_step = (to < from);
        }

        const std::string& loop_variable = statement.loop_variable();

        int      j           = negative_step ? (-from) : from;
        int      signed_to   = negative_step ? (-to) : to;
        int      signed_step = negative_step ? (-step) : step;
        unsigned i           = from;
        //for ( unsigned i = from; i <= to; i += step )
        while (j <= signed_to) {
            // adjust loop variable if necessary
            if (!loop_variable.empty()) {
                intern_variable_mapping[loop_variable] = i;
            }

            for (syrec::statement::ptr stat: statement.statements()) {
                if (!on_statement(stat)) return false;
            }

            i += signed_step;
            j += step;
        }

        // clear loop variable if necessary
        if (!loop_variable.empty()) {
            assert(intern_variable_mapping.erase(loop_variable) == 1u);
        }

        return true;
    }

    bool garbagefree_syrec_synthesizer::on_statement(const syrec::call_statement& statement) {
        if (_settings->get<bool>("modules_hierarchy", false)) {
            // Alternative implementation
            //std::cout << "on_call_statement" << std::endl;
            const std::string& module_name = statement.target()->name();
            if (circ().modules().find(module_name) == circ().modules().end()) {
                circuit        c_module;
                syrec::program prog_module;
                prog_module.add_module(statement.target());

                syrec_synthesis_garbagefree(c_module, prog_module, _settings);

                circ().add_module(module_name, c_module);
                get(boost::vertex_name, cct_man.tree)[cct_man.current].circ->add_module(module_name, c_module);
            }

            // create targets, first parameters
            std::vector<unsigned> targets;
            for (const std::string& parameter: statement.parameters()) {
                syrec::variable::ptr var = modules.top()->find_parameter_or_variable(parameter);
                boost::push_back(targets, boost::irange(var_lines()[var], var_lines()[var] + var->bitwidth()));
            }

            // now constant lines (they are always in order)
            const circuit& module = *(circ().modules().find(module_name)->second);
            unsigned       i      = 0u;
            for (const constant& c: module.constants()) {
                //std::cout << module.outputs().at(i) << std::endl;
                // damit keine out-Variablen nochmal hinzugefügt werden! (wires schon?!) stimmt das so?
                if (c && module.garbage().at(i)) {
                    targets += get_reusable_constant_line(*c);
                    used_const_lines.pop();
                }
                ++i;
            }

            append_module(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), module_name, gate::line_container(), targets);

            // reuse lines
            i = 0u;
            // TODO: nicht nur constants durchgucken?
            //std::cout << "module.constants: " << module.constants().size() << std::endl;
            //std::cout << "module.outputs: " << module.outputs().size() << std::endl;
            for (const constant& c: module.constants()) {
                //std::cout << module.outputs().at(i) << std::endl;
                if (c) {
                    if (module.outputs().at(i) == "const_0") {
                        //std::cout << "this is constant 0" << std::endl;
                        release_constant_line(targets.at(i), false);
                    } else if (module.outputs().at(i) == "const_1") {
                        //std::cout << "this is constant 1" << std::endl;
                        release_constant_line(targets.at(i), true);
                    }
                }

                ++i;
            }
        } else {
            // 1. Adjust the references module's parameters to the call arguments
            for (unsigned i = 0u; i < statement.parameters().size(); ++i) {
                const std::string&          parameter        = statement.parameters().at(i);
                const syrec::variable::ptr& module_parameter = statement.target()->parameters().at(i);

                module_parameter->set_reference(modules.top()->find_parameter_or_variable(parameter));
            }

            // 2. Create new lines for the module's variables
            add_variables2(circ(), *this, variable_name_format, statement.target()->variables());

            modules.push(statement.target());
            for (syrec::statement::ptr stat: statement.target()->statements()) {
                if (!on_statement(stat)) return false;
            }
            modules.pop();

            // Map-Einträge löschen damit sie nicht wiederverwendet werden bei erneutem Aufruf des gleichen Moduls
            for (syrec::variable::ptr var: statement.target()->variables()) {
                var_lines().erase(var);
            }
        }

        return true;
    }

    bool garbagefree_syrec_synthesizer::on_statement(const syrec::uncall_statement& statement) {
        using boost::adaptors::reversed;
        using boost::adaptors::transformed;

        // 1. Adjust the references module's parameters to the call arguments
        for (unsigned i = 0u; i < statement.parameters().size(); ++i) {
            const std::string&          parameter        = statement.parameters().at(i);
            const syrec::variable::ptr& module_parameter = statement.target()->parameters().at(i);

            module_parameter->set_reference(modules.top()->find_parameter_or_variable(parameter));
        }

        // 2. Create new lines for the module's variables
        add_variables2(circ(), *this, variable_name_format, statement.target()->variables());

        modules.push(statement.target());
        for (syrec::statement::ptr stat: statement.target()->statements() | reversed | transformed(syrec::reverse_statements())) {
            if (!on_statement(stat)) return false;
        }
        modules.pop();

        for (syrec::variable::ptr var: statement.target()->variables()) {
            var_lines().erase(var);
        }

        return true;
    }

    bool garbagefree_syrec_synthesizer::on_statement(const syrec::skip_statement& statement) {
        UNUSED(statement);
        return true;
    }

    syrec::expression::ptr garbagefree_syrec_synthesizer::evaluate_to_numeric_expression(syrec::expression::ptr expression) {
        if (auto* exp = dynamic_cast<syrec::unary_expression*>(expression.get())) {
            syrec::expression::ptr sub_expression = evaluate_to_numeric_expression(exp->expr());
            if (auto* sub_exp = dynamic_cast<syrec::numeric_expression*>(sub_expression.get())) {
                switch (exp->op()) {
                    case syrec::unary_expression::logical_not: // !
                    {
                        unsigned value = (sub_exp->value()->evaluate(intern_variable_mapping) == 0) ? 1 : 0;
                        return syrec::expression::ptr(new syrec::numeric_expression(std::make_shared<syrec::number>(value), 1));
                    } break;
                    case syrec::unary_expression::bitwise_not: // should not occur
                    {
                        std::cerr << boost::format("Bitwise NOT is undefined for numbers w/o specified bit width: ~%s") % *exp << std::endl;
                        assert(false);
                    } break;
                    default:
                        std::cerr << "Invalid Operator in unary expression" << std::endl;
                        assert(false);
                }
            }
            return syrec::expression::ptr(new syrec::unary_expression(exp->op(), sub_expression));
        } else if (auto* exp = dynamic_cast<syrec::binary_expression*>(expression.get())) {
            syrec::expression::ptr lhs = evaluate_to_numeric_expression(exp->lhs());
            syrec::expression::ptr rhs = evaluate_to_numeric_expression(exp->rhs());
            if (auto* lhs_exp = dynamic_cast<syrec::numeric_expression*>(lhs.get())) {
                if (auto* rhs_exp = dynamic_cast<syrec::numeric_expression*>(rhs.get())) {
                    unsigned lhs_value = lhs_exp->value()->evaluate(intern_variable_mapping);
                    unsigned rhs_value = rhs_exp->value()->evaluate(intern_variable_mapping);
                    unsigned num_value;

                    switch (exp->op()) {
                        case syrec::binary_expression::add: // +
                        {
                            num_value = lhs_value + rhs_value;
                        } break;

                        case syrec::binary_expression::subtract: // -
                        {
                            num_value = lhs_value - rhs_value;
                        } break;

                        case syrec::binary_expression::exor: // ^
                        {
                            num_value = lhs_value ^ rhs_value;
                        } break;

                        case syrec::binary_expression::multiply: // *
                        {
                            num_value = lhs_value * rhs_value;
                        } break;

                        case syrec::binary_expression::divide: // /
                        {
                            num_value = lhs_value / rhs_value;
                        } break;

                        case syrec::binary_expression::modulo: // %
                        {
                            num_value = lhs_value % rhs_value;
                        } break;

                        case syrec::binary_expression::frac_divide: // *>
                        {
                            std::cerr << boost::format("Operator *> is undefined for numbers w/o specified bit width: ( %d *> %d )") % lhs_value % rhs_value << std::endl;
                            assert(false);
                        } break;

                        case syrec::binary_expression::logical_and: // &&
                        {
                            num_value = lhs_value && rhs_value;
                        } break;

                        case syrec::binary_expression::logical_or: // ||
                        {
                            num_value = lhs_value || rhs_value;
                        } break;

                        case syrec::binary_expression::bitwise_and: // &
                        {
                            num_value = lhs_value & rhs_value;
                        } break;

                        case syrec::binary_expression::bitwise_or: // |
                        {
                            num_value = lhs_value | rhs_value;
                        } break;

                        case syrec::binary_expression::less_than: // <
                        {
                            num_value = lhs_value < rhs_value;
                        } break;

                        case syrec::binary_expression::greater_than: // >
                        {
                            num_value = lhs_value > rhs_value;
                        } break;

                        case syrec::binary_expression::equals: // =
                        {
                            num_value = lhs_value == rhs_value;
                        } break;

                        case syrec::binary_expression::not_equals: // !=
                        {
                            num_value = lhs_value != rhs_value;
                        } break;

                        case syrec::binary_expression::less_equals: // <=
                        {
                            num_value = lhs_value <= rhs_value;
                        } break;

                        case syrec::binary_expression::greater_equals: // >=
                        {
                            num_value = lhs_value >= rhs_value;
                        } break;

                        default:
                            std::cerr << "Invalid Operator in binary expression" << std::endl;
                            assert(false);
                    }

                    return syrec::expression::ptr(new syrec::numeric_expression(std::make_shared<syrec::number>(num_value), exp->lhs()->bitwidth()));
                }
            }
            return syrec::expression::ptr(new syrec::binary_expression(lhs, exp->op(), rhs));
        } else if (auto* exp = dynamic_cast<syrec::shift_expression*>(expression.get())) {
            syrec::expression::ptr lhs = evaluate_to_numeric_expression(exp->lhs());
            if (auto* lhs_no = dynamic_cast<syrec::numeric_expression*>(lhs.get())) {
                unsigned value    = lhs_no->value()->evaluate(intern_variable_mapping);
                unsigned shft_amt = exp->rhs()->evaluate(intern_variable_mapping);
                unsigned result;

                switch (exp->op()) {
                    case syrec::shift_expression::left: // <<
                    {
                        result = value << shft_amt;
                    } break;

                    case syrec::shift_expression::right: // >>
                    {
                        result = value >> shft_amt;
                    } break;

                    default:
                        std::cerr << "Invalid operator in shift expression" << std::endl;
                        assert(false);
                }
                return syrec::expression::ptr(new syrec::numeric_expression(std::make_shared<syrec::number>(result), lhs->bitwidth()));
            }
            return syrec::expression::ptr(new syrec::shift_expression(lhs, exp->op(), exp->rhs()));
        } else if (auto* exp = dynamic_cast<syrec::variable_expression*>(expression.get())) {
            return syrec::expression::ptr(new syrec::variable_expression(evaluate_to_numeric_expression(exp->var())));
        }
        // if it's already a numeric expression, there's nothing to do here
        return expression;
    }

    syrec::variable_access::ptr garbagefree_syrec_synthesizer::evaluate_to_numeric_expression(const syrec::variable_access::ptr& var_access) {
        std::vector<std::shared_ptr<syrec::expression>> new_indexes;
        for (const auto& i: var_access->indexes()) {
            new_indexes.push_back(evaluate_to_numeric_expression(i));
        }
        syrec::variable_access::ptr new_var(new syrec::variable_access(var_access->var()));
        new_var->set_range(var_access->range());
        new_var->set_indexes(new_indexes);
        return new_var;
    }

    unsigned garbagefree_syrec_synthesizer::get_reusable_constant_line(bool value) {
        unsigned const_line = 0u;
        bool     inv_used   = false;

        if (!free_const_lines[value].empty()) {
            const_line = free_const_lines[value].back();
            free_const_lines[value].pop_back();
            //used_const_lines[depth][value].push_back( const_line );
        } else if (!free_const_lines[!value].empty()) {
            const_line = free_const_lines[!value].back();
            free_const_lines[!value].pop_back();
            gate::line_container actrls = get(boost::vertex_name, cct_man.tree)[cct_man.current].controls;
            for (unsigned actrl: actrls) {
                remove_active_control(actrl);
            }
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), const_line);
            for (unsigned actrl: actrls) {
                add_active_control(actrl);
            }
            inv_used = true;
            //inv_used_const_lines[depth][value].push_back( const_line );
        } else {
            const_line = add_line_to_circuit(circ(), (std::string("const_") + boost::lexical_cast<std::string>(value)), "garbage", value, true);
            //std::cout << "added const line to circuit" << std::endl;
            //used_const_lines[depth][value].push_back( const_line );
        }
        used_const_lines.push(boost::make_tuple(inv_used, value, const_line));
        // output description is changed to garbage;
        std::vector<std::string> outs = circ().outputs();
        outs.at(const_line)           = "garbage";
        circ().set_outputs(outs);
        //std::cout << "got line " << const_line << " with value " << value << std::endl;
        return const_line;
    }

    bool garbagefree_syrec_synthesizer::get_reusable_constant_lines(unsigned bitwidth, unsigned value, std::vector<unsigned>& lines) {
        boost::dynamic_bitset<> number(bitwidth, value);

        for (unsigned i = 0u; i < bitwidth; ++i) {
            lines += get_reusable_constant_line(number.test(i));
        }

        return true;
    }

    /*
  void garbagefree_syrec_synthesizer::release_inv_constant_lines( std::map< bool, std::vector<unsigned> > const_lines )
  {
    std::vector<unsigned> zero_lines = const_lines[false];
    for (std::vector<unsigned>::const_iterator it = zero_lines.begin(); it != zero_lines.end(); it++ )
    {
      //append_not( circ(), *it );
      release_constant_line( *it, true );
    }
    std::vector<unsigned> one_lines = const_lines[true];
    for (std::vector<unsigned>::const_iterator it = one_lines.begin(); it != one_lines.end(); it++ )
    {
      //append_not( circ(), *it );
      release_constant_line( *it, false );
    }
  }
  
  void garbagefree_syrec_synthesizer::release_constant_lines( std::map< bool, std::vector<unsigned> > const_lines )
  {
    //std::cout << "releasing lines for depth " << depth << std::endl;
    std::vector<unsigned> zero_lines = const_lines[false];
    for (std::vector<unsigned>::const_iterator it = zero_lines.begin(); it != zero_lines.end(); it++ )
    {
      release_constant_line( *it, false );
    }
    std::vector<unsigned> one_lines = const_lines[true];
    for (std::vector<unsigned>::const_iterator it = one_lines.begin(); it != one_lines.end(); it++ )
    {
      release_constant_line( *it, true );
    }
  }
  */

    void garbagefree_syrec_synthesizer::release_constant_lines(const std::list<boost::tuple<bool, bool, unsigned>>& const_lines) {
        for (auto& const_line: const_lines) {
            release_constant_line(const_line);
        }
    }

    void garbagefree_syrec_synthesizer::release_constant_line(boost::tuple<bool, bool, unsigned> const_line) {
        if (const_line.get<0>()) {
            gate::line_container actrls = get(boost::vertex_name, cct_man.tree)[cct_man.current].controls;
            for (unsigned actrl: actrls) {
                remove_active_control(actrl);
            }
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), const_line.get<2>());
            for (unsigned actrl: actrls) {
                add_active_control(actrl);
            }
            release_constant_line(const_line.get<2>(), !const_line.get<1>());
        } else {
            release_constant_line(const_line.get<2>(), const_line.get<1>());
        }
    }

    void garbagefree_syrec_synthesizer::release_constant_line(unsigned index, bool value) {
        free_const_lines[value].push_back(index);

        // output description is changed to constant;
        std::vector<std::string> outs = circ().outputs();
        outs.at(index)                = (std::string("const_") + boost::lexical_cast<std::string>(value));
        circ().set_outputs(outs);
        //std::cout << "released line " << index << " with value " << value << std::endl;
    }

    bool garbagefree_syrec_synthesizer::on_expression(const syrec::binary_expression& expression, std::vector<unsigned>& lines) {
        std::vector<unsigned> lhs, rhs;

        if (!on_expression(expression.lhs(), lhs) || !on_expression(expression.rhs(), rhs)) {
            return false;
        }

        switch (expression.op()) {
            case syrec::binary_expression::add: // +
            {
                get_reusable_constant_lines(expression.bitwidth(), 0u, lines);

                bitwise_cnot(lines, lhs); // duplicate lhs

                increase(lines, rhs);
            } break;

            case syrec::binary_expression::subtract: // -
            {
                get_reusable_constant_lines(expression.bitwidth(), 0u, lines);

                bitwise_cnot(lines, lhs); // duplicate lhs

                decrease(lines, rhs);
            } break;

            case syrec::binary_expression::exor: // ^
            {
                get_reusable_constant_lines(expression.bitwidth(), 0u, lines);

                bitwise_cnot(lines, lhs); // duplicate lhs

                bitwise_cnot(lines, rhs);
            } break;

            case syrec::binary_expression::multiply: // *
            {
                get_reusable_constant_lines(expression.bitwidth(), 0u, lines);

                multiplication(lines, lhs, rhs);
            } break;

            case syrec::binary_expression::divide: // /
            {
                get_reusable_constant_lines(expression.bitwidth(), 0u, lines);

                division(lines, lhs, rhs);
            } break;

            case syrec::binary_expression::modulo: // %
            {
                get_reusable_constant_lines(expression.bitwidth(), 0u, lines);
                std::vector<unsigned> quot;
                get_reusable_constant_lines(expression.bitwidth(), 0u, quot);

                bitwise_cnot(lines, lhs); // duplicate lhs
                modulo(quot, lines, rhs);
            } break;

            case syrec::binary_expression::frac_divide: // *>
            {
                get_reusable_constant_lines(expression.bitwidth(), 0u, lines);
                std::vector<unsigned> product;
                get_reusable_constant_lines(expression.bitwidth(), 0u, product);

                copy(lines.begin(), lines.end(), back_inserter(product));

                multiplication_full(product, lhs, rhs);
            } break;

            case syrec::binary_expression::logical_and: // &&
            {
                lines += get_reusable_constant_line(false);

                conjunction(lines.at(0), lhs.at(0), rhs.at(0));
            } break;

            case syrec::binary_expression::logical_or: // ||
            {
                lines += get_reusable_constant_line(false);

                disjunction(lines.at(0), lhs.at(0), rhs.at(0));
            } break;

            case syrec::binary_expression::bitwise_and: // &
            {
                get_reusable_constant_lines(expression.bitwidth(), 0u, lines);

                bitwise_and(lines, lhs, rhs);
            } break;

            case syrec::binary_expression::bitwise_or: // |
            {
                get_reusable_constant_lines(expression.bitwidth(), 0u, lines);

                bitwise_or(lines, lhs, rhs);
            } break;

            case syrec::binary_expression::less_than: // <
            {
                lines += get_reusable_constant_line(false);

                less_than(lines.at(0), lhs, rhs);
            } break;

            case syrec::binary_expression::greater_than: // >
            {
                lines += get_reusable_constant_line(false);

                greater_than(lines.at(0), lhs, rhs);
            } break;

            case syrec::binary_expression::equals: // =
            {
                lines += get_reusable_constant_line(false);

                equals(lines.at(0), lhs, rhs);
            } break;

            case syrec::binary_expression::not_equals: // !=
            {
                lines += get_reusable_constant_line(false);

                not_equals(lines.at(0), lhs, rhs);
            } break;

            case syrec::binary_expression::less_equals: // <=
            {
                lines += get_reusable_constant_line(false);

                less_equals(lines.at(0), lhs, rhs);
            } break;

            case syrec::binary_expression::greater_equals: // >=
            {
                lines += get_reusable_constant_line(false);

                greater_equals(lines.at(0), lhs, rhs);
            } break;

            default:
                std::cout << "Binary Expression" << std::endl;
                return false;
        }

        return true;
    }

    bool garbagefree_syrec_synthesizer::on_expression(const syrec::numeric_expression& expression, std::vector<unsigned>& lines) {
        return get_reusable_constant_lines(expression.bitwidth(), expression.value()->evaluate(intern_variable_mapping), lines);
    }

    bool garbagefree_syrec_synthesizer::on_expression(const syrec::variable_expression& expression, std::vector<unsigned>& lines) {
        return get_expr_variables(expression.var(), lines);
    }

    bool garbagefree_syrec_synthesizer::on_expression(const syrec::unary_expression& expression, std::vector<unsigned>& lines) {
        std::vector<unsigned> expr;

        if (!on_expression(expression.expr(), expr)) {
            return false;
        }

        switch (expression.op()) {
            case syrec::unary_expression::logical_not: // !
            {
                lines += get_reusable_constant_line(false);
                bitwise_negation(expr);
                gate::line_container controls(expr.begin(), expr.end());
                append_toffoli(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), controls, lines.at(0));
                bitwise_negation(expr);
            } break;

            case syrec::unary_expression::bitwise_not: // ~
            {
                get_reusable_constant_lines(expression.bitwidth(), 0u, lines);
                bitwise_cnot(lines, expr); // duplicate expr TODO nur wenn nötig -> Problem beim zurückrechnen
                bitwise_negation(lines);
            } break;

            default:
                std::cout << "Unary Expression" << std::endl;
                return false;
        }

        return true;
    }

    bool garbagefree_syrec_synthesizer::on_expression(const syrec::shift_expression& expression, std::vector<unsigned>& lines) {
        std::vector<unsigned> lhs;

        if (!on_expression(expression.lhs(), lhs)) {
            return false;
        }

        unsigned rhs = expression.rhs()->evaluate(intern_variable_mapping);

        switch (expression.op()) {
            case syrec::shift_expression::left: // <<
            {
                get_reusable_constant_lines(expression.bitwidth(), 0u, lines);

                left_shift(lines, lhs, rhs);
            } break;

            case syrec::shift_expression::right: // <<
            {
                get_reusable_constant_lines(expression.bitwidth(), 0u, lines);

                right_shift(lines, lhs, rhs);
            } break;

            default:
                std::cout << "Shift Expression" << std::endl;
                return false;
        }

        return true;
    }

    bool garbagefree_syrec_synthesizer::on_expression(const syrec::expression::ptr& expression, std::vector<unsigned>& lines) {
        if (auto* exp = dynamic_cast<syrec::numeric_expression*>(expression.get())) {
            return on_expression(*exp, lines);
        } else if (auto* exp = dynamic_cast<syrec::variable_expression*>(expression.get())) {
            return on_expression(*exp, lines);
        } else if (auto* exp = dynamic_cast<syrec::binary_expression*>(expression.get())) {
            return on_expression(*exp, lines);
        } else if (auto* exp = dynamic_cast<syrec::unary_expression*>(expression.get())) {
            return on_expression(*exp, lines);
        } else if (auto* exp = dynamic_cast<syrec::shift_expression*>(expression.get())) {
            return on_expression(*exp, lines);
        } else {
            std::cout << "Expression" << std::endl;
            return false;
        }
    }

    bool garbagefree_syrec_synthesizer::off_expression(const syrec::binary_expression& expression, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl) {
        // lines that were used for lhs & rhs
        std::vector<unsigned>                          lhs, rhs, extra_lines;
        std::list<boost::tuple<bool, bool, unsigned>>  lhs_cl, rhs_cl, extra_cl;
        std::stack<boost::tuple<bool, bool, unsigned>> rhs_se_cl;

        if ((expression.op() == syrec::binary_expression::modulo) || (expression.op() == syrec::binary_expression::frac_divide)) {
            for (unsigned i = 0u; i < expression.bitwidth(); ++i) {
                boost::tuple<bool, bool, unsigned> const_line = used_const_lines.top();
                used_const_lines.pop();
                extra_cl.push_back(const_line);
                extra_lines.insert(extra_lines.begin(), const_line.get<2>());
            }
        }

        get_expr_lines(expression.rhs(), rhs, rhs_cl);
        //rhs-Anhängsel vom stack nehmen
        if (syrec::variable_expression* exp = dynamic_cast<syrec::variable_expression*>(expression.rhs().get())) {
            syrec::variable_access::ptr var = exp->var();
            var                             = get_dupl(var);

            if (!var->indexes().empty()) {
                // check if it is not all numeric_expressions
                unsigned n = var->var()->dimensions().size(); // dimensions
                if ((unsigned)boost::count_if(var->indexes(), is_type<syrec::numeric_expression>()) != n) {
                    // lines used for evaluating index expressions
                    unsigned j = 1u;
                    for (unsigned i = 0u; i < var->indexes().size(); ++i) {
                        for (unsigned k = 0u; k < j; ++k) {
                            if (!get_subexpr_lines(var->indexes().at(i), rhs_se_cl)) return false;
                        }
                        j *= var->var()->dimensions().at(i);
                    }
                }
            }
        } else if (syrec::binary_expression* exp = dynamic_cast<syrec::binary_expression*>(expression.rhs().get())) {
            if ((exp->op() == syrec::binary_expression::modulo) || (exp->op() == syrec::binary_expression::frac_divide)) {
                for (unsigned i = 0u; i < exp->bitwidth(); ++i) {
                    rhs_se_cl.push(used_const_lines.top());
                    used_const_lines.pop();
                }
            }
            if (!get_subexpr_lines(exp->rhs(), rhs_se_cl) || !get_subexpr_lines(exp->lhs(), rhs_se_cl)) {
                return false;
            }
        } else if (syrec::shift_expression* exp = dynamic_cast<syrec::shift_expression*>(expression.rhs().get())) {
            if (!get_subexpr_lines(exp->lhs(), rhs_se_cl)) {
                return false;
            }
        }
        get_expr_lines(expression.lhs(), lhs, lhs_cl);

        //rhs-anhängsel auf den stack zurücklegen
        while (!rhs_se_cl.empty()) {
            used_const_lines.push(rhs_se_cl.top());
            rhs_se_cl.pop();
        }

        if (garbagefree) {
            switch (expression.op()) {
                case syrec::binary_expression::add: // +
                {
                    decrease(lines, rhs);

                    reverse_bitwise_cnot(lines, lhs);
                } break;

                case syrec::binary_expression::subtract: // -
                {
                    increase(lines, rhs);

                    reverse_bitwise_cnot(lines, lhs);
                } break;

                case syrec::binary_expression::exor: // ^
                {
                    reverse_bitwise_cnot(lines, rhs);
                    reverse_bitwise_cnot(lines, lhs);
                } break;

                case syrec::binary_expression::multiply: // *
                {
                    reverse_multiplication(lines, lhs, rhs);
                } break;

                case syrec::binary_expression::divide: // /
                {
                    reverse_division(lines, lhs, rhs);
                } break;

                case syrec::binary_expression::modulo: // %
                {
                    reverse_modulo(extra_lines, lines, rhs);
                    reverse_bitwise_cnot(lines, lhs);
                } break;

                case syrec::binary_expression::frac_divide: // *>
                {
                    copy(lines.begin(), lines.end(), back_inserter(extra_lines));
                    reverse_multiplication_full(extra_lines, lhs, rhs);
                } break;

                case syrec::binary_expression::logical_and: // &&
                {
                    conjunction(lines.at(0), lhs.at(0), rhs.at(0));
                } break;

                case syrec::binary_expression::logical_or: // ||
                {
                    reverse_disjunction(lines.at(0), lhs.at(0), rhs.at(0));
                } break;

                case syrec::binary_expression::bitwise_and: // &
                {
                    reverse_bitwise_and(lines, lhs, rhs);
                } break;

                case syrec::binary_expression::bitwise_or: // |
                {
                    reverse_bitwise_or(lines, lhs, rhs);
                } break;

                case syrec::binary_expression::less_than: // <
                {
                    reverse_less_than(lines.at(0), lhs, rhs);
                } break;

                case syrec::binary_expression::greater_than: // >
                {
                    reverse_greater_than(lines.at(0), lhs, rhs);
                } break;

                case syrec::binary_expression::equals: // =
                {
                    reverse_equals(lines.at(0), lhs, rhs);
                } break;

                case syrec::binary_expression::not_equals: // !=
                {
                    reverse_not_equals(lines.at(0), lhs, rhs);
                } break;

                case syrec::binary_expression::less_equals: // <=
                {
                    reverse_less_equals(lines.at(0), lhs, rhs);
                } break;

                case syrec::binary_expression::greater_equals: // >=
                {
                    reverse_greater_equals(lines.at(0), lhs, rhs);
                } break;

                default:
                    std::cout << "Binary Expression" << std::endl;
                    return false;
            }

            release_constant_lines(extra_cl);
            release_constant_lines(expr_cl);
        }

        return (off_expression(expression.rhs(), rhs, rhs_cl) && off_expression(expression.lhs(), lhs, lhs_cl));
    }

    bool garbagefree_syrec_synthesizer::off_expression(const syrec::numeric_expression& expression, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl) {
        UNUSED(expression);
        UNUSED(lines);
        release_constant_lines(expr_cl);
        return true;
    }

    bool garbagefree_syrec_synthesizer::off_expression(const syrec::variable_expression& expression, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl) {
        if (!expr_cl.empty() && !unget_expr_variables(expression.var(), lines, expr_cl)) {
            return false;
        }

        return true;
    }

    bool garbagefree_syrec_synthesizer::off_expression(const syrec::unary_expression& expression, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl) {
        std::vector<unsigned>                         exp;
        std::list<boost::tuple<bool, bool, unsigned>> exp_cl;
        get_expr_lines(expression.expr(), exp, exp_cl);

        if (garbagefree) {
            switch (expression.op()) {
                case syrec::unary_expression::logical_not: // !
                {
                    reverse_bitwise_negation(exp);
                    gate::line_container controls(exp.begin(), exp.end());
                    append_toffoli(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), controls, lines.at(0));
                    reverse_bitwise_negation(exp);
                } break;

                case syrec::unary_expression::bitwise_not: // ~
                {
                    reverse_bitwise_negation(lines);
                    reverse_bitwise_cnot(lines, exp);
                } break;

                default:
                    std::cout << "Unary Expression" << std::endl;
                    return false;
            }

            release_constant_lines(expr_cl);
        }

        return off_expression(expression.expr(), exp, exp_cl);
    }

    bool garbagefree_syrec_synthesizer::off_expression(const syrec::shift_expression& expression, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl) {
        // lines that were used for lhs
        std::vector<unsigned>                         lhs;
        std::list<boost::tuple<bool, bool, unsigned>> lhs_cl;
        get_expr_lines(expression.lhs(), lhs, lhs_cl);

        if (garbagefree) {
            unsigned rhs = expression.rhs()->evaluate(intern_variable_mapping); // TODO kommt da immer noch das gleiche raus wie bei on_expression?

            switch (expression.op()) {
                case syrec::shift_expression::left: // <<
                {
                    reverse_left_shift(lines, lhs, rhs);
                } break;

                case syrec::shift_expression::right: // >>
                {
                    reverse_right_shift(lines, lhs, rhs);
                } break;

                default:
                    std::cout << "Shift Expression" << std::endl;
                    return false;
            }

            release_constant_lines(expr_cl);
        }

        return off_expression(expression.lhs(), lhs, lhs_cl);
    }

    bool garbagefree_syrec_synthesizer::off_expression(const syrec::expression::ptr& expression, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl) {
        if (auto* exp = dynamic_cast<syrec::numeric_expression*>(expression.get())) {
            return off_expression(*exp, lines, expr_cl);
        } else if (auto* exp = dynamic_cast<syrec::variable_expression*>(expression.get())) {
            return off_expression(*exp, lines, expr_cl);
        } else if (auto* exp = dynamic_cast<syrec::binary_expression*>(expression.get())) {
            return off_expression(*exp, lines, expr_cl);
        } else if (auto* exp = dynamic_cast<syrec::unary_expression*>(expression.get())) {
            return off_expression(*exp, lines, expr_cl);
        } else if (auto* exp = dynamic_cast<syrec::shift_expression*>(expression.get())) {
            return off_expression(*exp, lines, expr_cl);
        } else {
            std::cout << "Expression" << std::endl;
            return false;
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::get_subexpr_lines(const syrec::binary_expression& expression, std::stack<boost::tuple<bool, bool, unsigned>>& se_cl) {
        unsigned n = expression.bitwidth();
        /*if ( ( expression.op() == syrec::binary_expression::logical_and )
      || ( expression.op() == syrec::binary_expression::logical_or )
      || ( expression.op() == syrec::binary_expression::less_than )
      || ( expression.op() == syrec::binary_expression::greater_than )
      || ( expression.op() == syrec::binary_expression::equals )
      || ( expression.op() == syrec::binary_expression::not_equals )
      || ( expression.op() == syrec::binary_expression::less_equals )
      || ( expression.op() == syrec::binary_expression::greater_equals ) )
    {
      n = 1u;
    }*/
        if ((expression.op() == syrec::binary_expression::modulo) || (expression.op() == syrec::binary_expression::frac_divide)) {
            for (unsigned i = 0u; i < expression.bitwidth(); ++i) {
                se_cl.push(used_const_lines.top());
                used_const_lines.pop();
            }
        }
        for (unsigned i = 0u; i < n; ++i) {
            se_cl.push(used_const_lines.top());
            used_const_lines.pop();
        }
        return (get_subexpr_lines(expression.rhs(), se_cl) && get_subexpr_lines(expression.lhs(), se_cl));
    }

    bool garbagefree_syrec_synthesizer::get_subexpr_lines(const syrec::numeric_expression& expression, std::stack<boost::tuple<bool, bool, unsigned>>& se_cl) {
        for (unsigned i = 0u; i < expression.bitwidth(); ++i) {
            se_cl.push(used_const_lines.top());
            used_const_lines.pop();
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::get_subexpr_lines(const syrec::variable_expression& expression, std::stack<boost::tuple<bool, bool, unsigned>>& se_cl) {
        syrec::variable_access::ptr var = expression.var();
        var                             = get_dupl(var);

        if (!var->indexes().empty()) {
            // check if it is not all numeric_expressions
            unsigned n = var->var()->dimensions().size(); // dimensions
            if ((unsigned)boost::count_if(var->indexes(), is_type<syrec::numeric_expression>()) != n) {
                // lines used for evaluating index expressions
                // TODO funktioniert das mit zB a[b[c+1]]?
                unsigned j = 1u;
                for (unsigned i = 0u; i < var->indexes().size(); ++i) {
                    for (unsigned k = 0u; k < j; ++k) {
                        if (!get_subexpr_lines(var->indexes().at(i), se_cl)) return false;
                    }
                    j *= var->var()->dimensions().at(i);
                }
                // lines used for swapping (storing the variable itself)
                for (unsigned i = 0u; i < var->var()->bitwidth(); ++i) {
                    se_cl.push(used_const_lines.top());
                    used_const_lines.pop();
                }
            }
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::get_subexpr_lines(const syrec::unary_expression& expression, std::stack<boost::tuple<bool, bool, unsigned>>& se_cl) {
        for (unsigned i = 0u; i < expression.bitwidth(); ++i) {
            se_cl.push(used_const_lines.top());
            used_const_lines.pop();
        }
        return get_subexpr_lines(expression.expr(), se_cl);
    }

    bool garbagefree_syrec_synthesizer::get_subexpr_lines(const syrec::shift_expression& expression, std::stack<boost::tuple<bool, bool, unsigned>>& se_cl) {
        for (unsigned i = 0u; i < expression.bitwidth(); ++i) {
            se_cl.push(used_const_lines.top());
            used_const_lines.pop();
        }
        return get_subexpr_lines(expression.lhs(), se_cl);
    }

    bool garbagefree_syrec_synthesizer::get_subexpr_lines(const syrec::expression::ptr& expression, std::stack<boost::tuple<bool, bool, unsigned>>& se_cl) {
        if (auto* exp = dynamic_cast<syrec::numeric_expression*>(expression.get())) {
            return get_subexpr_lines(*exp, se_cl);
        } else if (auto* exp = dynamic_cast<syrec::variable_expression*>(expression.get())) {
            return get_subexpr_lines(*exp, se_cl);
        } else if (auto* exp = dynamic_cast<syrec::binary_expression*>(expression.get())) {
            return get_subexpr_lines(*exp, se_cl);
        } else if (auto* exp = dynamic_cast<syrec::unary_expression*>(expression.get())) {
            return get_subexpr_lines(*exp, se_cl);
        } else if (auto* exp = dynamic_cast<syrec::shift_expression*>(expression.get())) {
            return get_subexpr_lines(*exp, se_cl);
        } else {
            std::cout << "Expression" << std::endl;
            return false;
        }
    }

    //**********************************************************************
    //*****                      Unary Operations                      *****
    //**********************************************************************

    bool garbagefree_syrec_synthesizer::bitwise_negation(const std::vector<unsigned>& dest) {
        for (unsigned idx: dest) {
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), idx);
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::reverse_bitwise_negation(const std::vector<unsigned>& dest) {
        reverse_foreach_(unsigned idx, dest) {
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), idx);
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::decrement(const std::vector<unsigned>& dest) {
        for (unsigned i = 0u; i < dest.size(); ++i) {
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), dest.at(i));
            add_active_control(dest.at(i));
        }

        for (unsigned i = 0; i < dest.size(); ++i) {
            remove_active_control(dest.at(i));
        }

        return true;
    }

    bool garbagefree_syrec_synthesizer::increment(const std::vector<unsigned>& dest) {
        for (unsigned i = 0; i < dest.size(); ++i) {
            add_active_control(dest.at(i));
        }

        for (int i = int(dest.size()) - 1; i >= 0; --i) {
            remove_active_control(dest.at(i));
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), dest.at(i));
        }

        return true;
    }

    //**********************************************************************
    //*****                     Binary Operations                      *****
    //**********************************************************************

    bool garbagefree_syrec_synthesizer::bitwise_cnot(const std::vector<unsigned>& dest, const std::vector<unsigned>& src) {
        for (unsigned i = 0u; i < src.size(); ++i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src.at(i), dest.at(i));
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::conjunction(unsigned dest, unsigned src1, unsigned src2) {
        append_toffoli (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ))(src1, src2)(dest);

        return true;
    }

    bool garbagefree_syrec_synthesizer::bitwise_and(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        bool ok = true;
        for (unsigned i = 0u; i < dest.size(); ++i) {
            ok = ok && conjunction(dest.at(i), src1.at(i), src2.at(i));
        }
        return ok;
    }

    bool garbagefree_syrec_synthesizer::disjunction(unsigned dest, unsigned src1, unsigned src2) {
        append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src1, dest);
        append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src2, dest);
        append_toffoli (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ))(src1, src2)(dest);

        return true;
    }

    bool garbagefree_syrec_synthesizer::bitwise_or(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        bool ok = true;
        for (unsigned i = 0u; i < dest.size(); ++i) {
            ok = ok && disjunction(dest.at(i), src1.at(i), src2.at(i));
        }
        return ok;
    }

    bool garbagefree_syrec_synthesizer::increase(const std::vector<unsigned>& dest, const std::vector<unsigned>& src) {
        unsigned bitwidth = src.size();

        // set module name and check whether it exists already
        std::string module_name          = boost::str(boost::format("increase_%d") % bitwidth);
        bool        circ_has_module      = circ().modules().find(module_name) != circ().modules().end();
        bool        tree_circ_has_module = get(boost::vertex_name, cct_man.tree)[cct_man.current].circ->modules().find(module_name) != get(boost::vertex_name, cct_man.tree)[cct_man.current].circ->modules().end();

        // create module if it does not exist
        if (!circ_has_module || !tree_circ_has_module) {
            // signals src .. dest
            circuit c_increase(bitwidth * 2);
            // TODO inputs and outputs

            for (unsigned i: boost::irange(1u, bitwidth)) {
                append_cnot(c_increase, i, bitwidth + i);
            }

            if (bitwidth >= 2u) {
                reverse_foreach_(unsigned i, boost::irange(1u, bitwidth - 1u)) {
                    append_cnot(c_increase, i, i + 1);
                }
            }

            for (unsigned i: boost::irange(0u, bitwidth - 1u)) {
                append_toffoli(c_increase)(i, bitwidth + i)(i + 1);
            }

            reverse_foreach_(unsigned i, boost::irange(1u, bitwidth)) {
                append_cnot(c_increase, i, bitwidth + i);
                append_toffoli(c_increase)(bitwidth + i - 1, i - 1)(i);
            }

            if (bitwidth >= 2u) {
                for (unsigned i: boost::irange(1u, bitwidth - 1u)) {
                    append_cnot(c_increase, i, i + 1);
                }
            }

            for (unsigned i: boost::irange(0u, bitwidth)) {
                append_cnot(c_increase, i, bitwidth + i);
            }

            if (!circ_has_module) {
                circ().add_module(module_name, c_increase);
            }

            if (!tree_circ_has_module) {
                get(boost::vertex_name, cct_man.tree)[cct_man.current].circ->add_module(module_name, c_increase); // TODO needed?
            }
        }

        std::vector<unsigned> targets;
        boost::push_back(targets, src);
        boost::push_back(targets, dest);
        append_module(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), module_name, gate::line_container(), targets);
        return true;
    }

    bool garbagefree_syrec_synthesizer::increase_with_carry(const std::vector<unsigned>& dest, const std::vector<unsigned>& src, unsigned carry) {
        unsigned bitwidth = src.size();

        if (bitwidth == 0) return true;

        for (unsigned i = 1u; i < bitwidth; ++i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src.at(i), dest.at(i));
        }

        if (bitwidth > 1) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src.at(bitwidth - 1), carry);
        }
        for (int i = (int)bitwidth - 2; i > 0; --i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src.at(i), src.at(i + 1));
        }

        for (unsigned i = 0u; i < bitwidth - 1; ++i) {
            append_toffoli (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ))(src.at(i), dest.at(i))(src.at(i + 1));
        }
        append_toffoli (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ))(src.at(bitwidth - 1), dest.at(bitwidth - 1))(carry);

        for (int i = (int)bitwidth - 1; i > 0; --i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src.at(i), dest.at(i));
            append_toffoli (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ))(dest.at(i - 1), src.at(i - 1))(src.at(i));
        }

        for (unsigned i = 1u; i < bitwidth - 1u; ++i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src.at(i), src.at(i + 1));
        }

        for (unsigned i = 0u; i < bitwidth; ++i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src.at(i), dest.at(i));
        }

        return true;
    }

    bool garbagefree_syrec_synthesizer::decrease(const std::vector<unsigned>& dest, const std::vector<unsigned>& src) {
        unsigned bitwidth = src.size();

        // set module name and check whether it exists already
        std::string module_name          = boost::str(boost::format("decrease_%d") % bitwidth);
        bool        circ_has_module      = circ().modules().find(module_name) != circ().modules().end();
        bool        tree_circ_has_module = get(boost::vertex_name, cct_man.tree)[cct_man.current].circ->modules().find(module_name) != get(boost::vertex_name, cct_man.tree)[cct_man.current].circ->modules().end();

        // create module if it does not exist
        if (!circ_has_module || !tree_circ_has_module) {
            // signals src .. dest
            circuit c_decrease(bitwidth * 2);
            // TODO inputs and outputs

            reverse_foreach_(unsigned i, boost::irange(0u, bitwidth)) {
                append_cnot(c_decrease, i, bitwidth + i);
            }

            if (bitwidth >= 2u) {
                reverse_foreach_(unsigned i, boost::irange(1u, bitwidth - 1u)) {
                    append_cnot(c_decrease, i, i + 1);
                }
            }

            for (unsigned i: boost::irange(1u, bitwidth)) {
                append_toffoli(c_decrease)(bitwidth + i - 1, i - 1)(i);
                append_cnot(c_decrease, i, bitwidth + i);
            }

            reverse_foreach_(unsigned i, boost::irange(0u, bitwidth - 1u)) {
                append_toffoli(c_decrease)(i, bitwidth + i)(i + 1);
            }

            if (bitwidth >= 2u) {
                for (unsigned i: boost::irange(1u, bitwidth - 1u)) {
                    append_cnot(c_decrease, i, i + 1);
                }
            }

            reverse_foreach_(unsigned i, boost::irange(1u, bitwidth)) {
                append_cnot(c_decrease, i, bitwidth + i);
            }

            if (!circ_has_module) {
                circ().add_module(module_name, c_decrease);
            }

            if (!tree_circ_has_module) {
                get(boost::vertex_name, cct_man.tree)[cct_man.current].circ->add_module(module_name, c_decrease); // TODO needed?
            }
        }

        std::vector<unsigned> targets;
        boost::push_back(targets, src);
        boost::push_back(targets, dest);
        append_module(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), module_name, gate::line_container(), targets);
        return true;
    }

    bool garbagefree_syrec_synthesizer::decrease_with_carry(const std::vector<unsigned>& dest, const std::vector<unsigned>& src, unsigned carry) {
        unsigned bitwidth = src.size();

        if (bitwidth == 0) return true;

        for (int i = (int)bitwidth - 1; i >= 0; --i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src.at(i), dest.at(i));
        }

        for (int i = (int)bitwidth - 2; i > 0; --i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src.at(i), src.at(i + 1));
        }

        for (unsigned i = 1u; i < bitwidth; ++i) {
            append_toffoli (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ))(dest.at(i - 1), src.at(i - 1))(src.at(i));
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src.at(i), dest.at(i));
        }

        append_toffoli (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ))(src.at(bitwidth - 1), dest.at(bitwidth - 1))(carry);
        for (int i = (int)bitwidth - 2; i >= 0; --i) {
            append_toffoli (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ))(src.at(i), dest.at(i))(src.at(i + 1));
        }

        for (unsigned i = 1u; i < bitwidth - 1u; ++i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src.at(i), src.at(i + 1));
        }

        if (bitwidth > 1) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src.at(bitwidth - 1), carry);
        }

        for (int i = (int)bitwidth - 1; i > 0; --i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src.at(i), dest.at(i));
        }

        return true;
    }

    bool garbagefree_syrec_synthesizer::less_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if (!less_than(dest, src2, src1)) return false;
        append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), dest);

        return true;
    }

    bool garbagefree_syrec_synthesizer::less_than(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        //return ( decrease_with_carry( src1, src2, dest ) && increase( src1, src2 ) );
        for (unsigned i = 0u; i < src1.size(); ++i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src2.at(i), src1.at(i));
        }

        gate::line_container controls;
        for (int i = int(src1.size()) - 1; i > 0; --i) {
            controls.insert(src1.at(i));
            add_active_control(src2.at(i));
            append_toffoli(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), controls, dest);
            remove_active_control(src2.at(i));
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src1.at(i));
        }
        controls.insert(src1.at(0));
        controls.insert(src2.at(0));
        append_toffoli(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), controls, dest);

        for (unsigned i = 1u; i < src1.size(); ++i) {
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src1.at(i));
        }
        for (int i = int(src1.size()) - 1; i >= 0; --i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src2.at(i), src1.at(i));
        }

        return true;
    }

    bool garbagefree_syrec_synthesizer::equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        for (unsigned i = 0u; i < src1.size(); ++i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src2.at(i), src1.at(i));
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src1.at(i));
        }

        gate::line_container controls(src1.begin(), src1.end());
        append_toffoli(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), controls, dest);

        for (unsigned i = 0u; i < src1.size(); ++i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src2.at(i), src1.at(i));
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src1.at(i));
        }

        return true;
    }

    bool garbagefree_syrec_synthesizer::greater_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if (!less_than(dest, src1, src2)) return false;
        append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), dest);

        return true;
    }

    bool garbagefree_syrec_synthesizer::greater_than(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        return less_than(dest, src2, src1);
    }

    bool garbagefree_syrec_synthesizer::not_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if (!equals(dest, src1, src2)) return false;
        append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), dest);
        return true;
    }

    bool garbagefree_syrec_synthesizer::swap(const std::vector<unsigned>& dest1, const std::vector<unsigned>& dest2) {
        for (unsigned i = 0u; i < dest1.size(); ++i) {
            append_fredkin (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ))()(dest1.at(i), dest2.at(i));
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::multiplication(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if ((src1.size() <= 0) || (dest.size() <= 0)) return true;

        std::vector<unsigned> sum     = dest;
        std::vector<unsigned> partial = src2;

        bool ok = true;

        add_active_control(src1.at(0));
        ok = ok && bitwise_cnot(sum, partial);
        remove_active_control(src1.at(0));

        for (unsigned i = 1; i < dest.size(); ++i) {
            sum.erase(sum.begin());
            partial.pop_back();
            add_active_control(src1.at(i));
            ok = ok && increase(sum, partial);
            remove_active_control(src1.at(i));
        }

        return ok;
    }

    bool garbagefree_syrec_synthesizer::multiplication_full(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if ((src1.size() <= 0) || (src2.size() <= 0)) return true;

        std::vector<unsigned> sum(dest.begin(), dest.begin() + src2.size());
        std::vector<unsigned> partial = src2;

        bool ok = true;

        add_active_control(src1.at(0));
        ok = ok && bitwise_cnot(sum, partial);
        remove_active_control(src1.at(0));

        for (unsigned i = 1; i < src1.size(); ++i) {
            sum.erase(sum.begin());
            sum.push_back(dest.at(src2.size() + i - 1));
            add_active_control(src1.at(i));
            ok = ok && increase_with_carry(sum, partial, dest.at(src2.size() + i));
            remove_active_control(src1.at(i));
        }

        return ok;
    }

    bool garbagefree_syrec_synthesizer::modulo(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        std::vector<unsigned> sum;
        std::vector<unsigned> partial;

        for (unsigned i = 1u; i < src1.size(); ++i) {
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src2.at(i));
        }

        for (unsigned i = 1u; i < src1.size(); ++i) {
            add_active_control(src2.at(i));
        }

        for (int i = int(src1.size()) - 1; i >= 0; --i) {
            partial.push_back(src2.at(src1.size() - 1u - i));
            sum.insert(sum.begin(), src1.at(i));
            decrease_with_carry(sum, partial, dest.at(i));
            add_active_control(dest.at(i));
            increase(sum, partial);
            remove_active_control(dest.at(i));
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), dest.at(i));
            if (i > 0) {
                for (unsigned j = (src1.size() - i); j < src1.size(); ++j) {
                    remove_active_control(src2.at(j));
                }
                append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src2.at(src1.size() - i));
                for (unsigned j = (src1.size() + 1u - i); j < src1.size(); ++j) {
                    add_active_control(src2.at(j));
                }
            }
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::division(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if (!modulo(dest, src1, src2)) return false;

        std::vector<unsigned> sum;
        std::vector<unsigned> partial;

        for (unsigned i = 1u; i < src1.size(); ++i) {
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src2.at(i));
        }

        for (unsigned i = 1u; i < src1.size(); ++i) {
            add_active_control(src2.at(i));
        }

        for (int i = int(src1.size()) - 1; i >= 0; --i) {
            partial.push_back(src2.at(src1.size() - 1u - i));
            sum.insert(sum.begin(), src1.at(i));
            add_active_control(dest.at(i));
            increase(sum, partial);
            remove_active_control(dest.at(i));
            if (i > 0) {
                for (unsigned j = (src1.size() - i); j < src1.size(); ++j) {
                    remove_active_control(src2.at(j));
                }
                append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src2.at(src1.size() - i));
                for (unsigned j = (src1.size() + 1u - i); j < src1.size(); ++j) {
                    add_active_control(src2.at(j));
                }
            }
        }

        return true;
    }

    //**********************************************************************
    //*****                 Reverse Binary Operations                  *****
    //**********************************************************************

    bool garbagefree_syrec_synthesizer::reverse_less_than(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        //return ( decrease( src1, src2 ) && increase_with_carry( src1, src2, dest ) );
        for (unsigned i = 0u; i < src1.size(); ++i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src2.at(i), src1.at(i));
        }

        for (int i = int(src1.size()) - 1; i > 0; --i) {
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src1.at(i));
        }

        gate::line_container controls(src1.begin(), src1.end());
        for (unsigned i = 0u; i < (src1.size() - 1); ++i) {
            add_active_control(src2.at(i));
            append_toffoli(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), controls, dest);
            remove_active_control(src2.at(i));
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src1.at(i + 1));
            controls.erase(src1.at(i));
        }
        controls.insert(src2.back());
        append_toffoli(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), controls, dest);

        for (int i = int(src1.size()) - 1; i >= 0; --i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src2.at(i), src1.at(i));
        }

        return true;
    }

    bool garbagefree_syrec_synthesizer::reverse_less_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), dest);

        return reverse_less_than(dest, src2, src1);
    }

    bool garbagefree_syrec_synthesizer::reverse_greater_than(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        return reverse_less_than(dest, src2, src1);
    }

    bool garbagefree_syrec_synthesizer::reverse_greater_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), dest);

        return reverse_less_than(dest, src1, src2);
    }

    bool garbagefree_syrec_synthesizer::reverse_bitwise_or(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        bool ok = true;
        for (int i = int(dest.size()) - 1; i >= 0; --i) {
            ok = ok && reverse_disjunction(dest.at(i), src1.at(i), src2.at(i));
        }
        return ok;
    }

    bool garbagefree_syrec_synthesizer::reverse_bitwise_and(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        bool ok = true;
        for (int i = int(dest.size()) - 1; i >= 0; --i) {
            ok = ok && conjunction(dest.at(i), src1.at(i), src2.at(i));
        }
        return ok;
    }

    bool garbagefree_syrec_synthesizer::reverse_bitwise_cnot(const std::vector<unsigned>& dest, const std::vector<unsigned>& src) {
        for (int i = int(src.size()) - 1; i >= 0; --i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src.at(i), dest.at(i));
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::reverse_disjunction(unsigned dest, unsigned src1, unsigned src2) {
        append_toffoli (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ))(src1, src2)(dest);
        append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src2, dest);
        append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src1, dest);

        return true;
    }

    bool garbagefree_syrec_synthesizer::reverse_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        for (int i = int(src1.size()) - 1; i >= 0; --i) {
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src1.at(i));
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src2.at(i), src1.at(i));
        }

        gate::line_container controls(src1.begin(), src1.end());
        append_toffoli(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), controls, dest);

        for (int i = int(src1.size()) - 1; i >= 0; --i) {
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src1.at(i));
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src2.at(i), src1.at(i));
        }

        return true;
    }

    bool garbagefree_syrec_synthesizer::reverse_not_equals(unsigned dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), dest);
        return reverse_equals(dest, src1, src2);
    }

    bool garbagefree_syrec_synthesizer::reverse_swap(const std::vector<unsigned>& dest1, const std::vector<unsigned>& dest2) {
        for (int i = int(dest1.size()) - 1; i >= 0; --i) {
            append_fredkin (*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ))()(dest1.at(i), dest2.at(i));
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::reverse_modulo(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        std::vector<unsigned> sum(src1);
        std::vector<unsigned> partial(src2);

        for (unsigned i = 0u; i < src1.size(); ++i) {
            if (i > 0) {
                for (int j = int(src1.size()) - 1; j >= int(src1.size() + 1u - i); --j) {
                    remove_active_control(src2.at(j));
                }
                append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src2.at(src1.size() - i));
                for (int j = int(src1.size()) - 1; j >= int(src1.size() - i); --j) {
                    add_active_control(src2.at(j));
                }
            }
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), dest.at(i));
            add_active_control(dest.at(i));
            decrease(sum, partial);
            remove_active_control(dest.at(i));
            increase_with_carry(sum, partial, dest.at(i));
            sum.erase(sum.begin());
            partial.pop_back();
        }

        for (unsigned i = 1u; i < src1.size(); ++i) {
            remove_active_control(src2.at(i));
        }

        for (int i = int(src1.size()) - 1; i > 0; --i) {
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src2.at(i));
        }

        return true;
    }

    bool garbagefree_syrec_synthesizer::reverse_multiplication(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if ((src1.size() <= 0) || (src2.size() <= 0) || (dest.size() <= 0)) return true;

        std::vector<unsigned> sum(1, dest.at(dest.size() - 1));
        std::vector<unsigned> partial(1, src2.at(0));

        bool ok = true;

        for (int i = int(dest.size()) - 1; i > 0; --i) {
            add_active_control(src1.at(i));
            ok = ok && decrease(sum, partial);
            remove_active_control(src1.at(i));
            partial.push_back(src2.at(dest.size() - i));
            sum.insert(sum.begin(), dest.at(i - 1));
        }

        add_active_control(src1.at(0));
        ok = ok && reverse_bitwise_cnot(sum, partial);
        remove_active_control(src1.at(0));

        return ok;
    }

    bool garbagefree_syrec_synthesizer::reverse_multiplication_full(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        if ((src1.size() <= 0) || (src2.size() <= 0)) return true;

        std::vector<unsigned> sum(dest.begin() + src2.size() - 1, dest.end() - 1);
        std::vector<unsigned> partial = src2;

        bool ok = true;

        for (int i = int(src1.size()) - 1; i > 0; --i) {
            add_active_control(src1.at(i));
            ok = ok && decrease_with_carry(sum, partial, dest.at(src2.size() + i));
            remove_active_control(src1.at(i));
            sum.pop_back();
            sum.insert(sum.begin(), dest.at(i - 1));
        }

        add_active_control(src1.at(0));
        ok = ok && reverse_bitwise_cnot(sum, partial);
        remove_active_control(src1.at(0));

        return ok;
    }

    bool garbagefree_syrec_synthesizer::reverse_division(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, const std::vector<unsigned>& src2) {
        std::vector<unsigned> sum(src1);
        std::vector<unsigned> partial(src2);

        for (unsigned i = 0; i < src1.size(); ++i) {
            if (i > 0) {
                for (unsigned j = (src1.size() + 1u - i); j < src1.size(); ++j) {
                    remove_active_control(src2.at(j));
                }
                append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src2.at(src1.size() - i));
                for (unsigned j = (src1.size() - i); j < src1.size(); ++j) {
                    add_active_control(src2.at(j));
                }
            }
            add_active_control(dest.at(i));
            decrease(sum, partial);
            remove_active_control(dest.at(i));
            sum.erase(sum.begin());
            partial.pop_back();
        }

        for (unsigned i = 1u; i < src1.size(); ++i) {
            remove_active_control(src2.at(i));
        }

        for (int i = int(src1.size()) - 1; i > 0; --i) {
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src2.at(i));
        }

        return reverse_modulo(dest, src1, src2);
    }

    //**********************************************************************
    //*****                      Shift Operations                      *****
    //**********************************************************************

    bool garbagefree_syrec_synthesizer::left_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2) {
        for (unsigned i = 0u; (i + src2) < dest.size(); ++i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src1.at(i), dest.at(i + src2));
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::reverse_left_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2) {
        for (int i = (int(dest.size()) - int(src2)) - 1; i >= 0; --i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src1.at(i), dest.at(i + src2));
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::right_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2) {
        for (unsigned i = src2; i < dest.size(); ++i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src1.at(i), dest.at(i - src2));
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::reverse_right_shift(const std::vector<unsigned>& dest, const std::vector<unsigned>& src1, unsigned src2) {
        for (int i = int(dest.size()) - 1; i >= int(src2); --i) {
            append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), src1.at(i), dest.at(i - src2));
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::on_condition(const syrec::expression::ptr& condition, unsigned result_line) {
        if (syrec::numeric_expression* cond = dynamic_cast<syrec::numeric_expression*>(condition.get())) {
            return on_condition(*cond, result_line);
        } else if (syrec::variable_expression* cond = dynamic_cast<syrec::variable_expression*>(condition.get())) {
            return on_condition(*cond, result_line);
        } else if (syrec::binary_expression* cond = dynamic_cast<syrec::binary_expression*>(condition.get())) {
            return on_condition(*cond, result_line);
        } else if (syrec::unary_expression* cond = dynamic_cast<syrec::unary_expression*>(condition.get())) {
            return on_condition(*cond, result_line);
        } else if (syrec::shift_expression* cond = dynamic_cast<syrec::shift_expression*>(condition.get())) {
            return on_condition(*cond, result_line);
        } else {
            std::cout << "Condition" << std::endl;
            return false;
        }
    }

    bool garbagefree_syrec_synthesizer::on_condition(const syrec::binary_expression& condition, unsigned result_line) {
        std::vector<unsigned> lhs, rhs, lines;
        lines.push_back(result_line);

        if (!on_expression(condition.lhs(), lhs) || !on_expression(condition.rhs(), rhs)) {
            return false;
        }
        switch (condition.op()) {
            case syrec::binary_expression::add: // +
            {
                bitwise_cnot(lines, lhs); // duplicate lhs

                increase(lines, rhs);
            } break;

            case syrec::binary_expression::subtract: // -
            {
                bitwise_cnot(lines, lhs); // duplicate lhs

                decrease(lines, rhs);
            } break;

            case syrec::binary_expression::exor: // ^
            {
                bitwise_cnot(lines, lhs); // duplicate lhs

                bitwise_cnot(lines, rhs);
            } break;

            case syrec::binary_expression::multiply: // *
            {
                multiplication(lines, lhs, rhs);
            } break;

            case syrec::binary_expression::divide: // /
            {
                division(lines, lhs, rhs);
            } break;

            case syrec::binary_expression::modulo: // %
            {
                std::vector<unsigned> lines2;
                get_reusable_constant_lines(condition.bitwidth(), 0u, lines2);
                std::vector<unsigned> quot;
                get_reusable_constant_lines(condition.bitwidth(), 0u, quot);

                bitwise_cnot(lines2, lhs); // duplicate lhs
                modulo(quot, lines2, rhs);
                bitwise_cnot(lines, lines2); // duplicate result

                // additional reverse computing
                std::list<boost::tuple<bool, bool, unsigned>> extra_cl;
                extra_cl.push_back(used_const_lines.top());
                used_const_lines.pop();
                extra_cl.push_back(used_const_lines.top());
                used_const_lines.pop(); // bitwidth shoudln't be more than 1 so this is it
                if (garbagefree) {
                    reverse_modulo(quot, lines2, rhs);
                    reverse_bitwise_cnot(lines2, lhs);
                    release_constant_lines(extra_cl);
                }
            } break;

            case syrec::binary_expression::frac_divide: // *>
            {
                std::vector<unsigned> lines2;
                get_reusable_constant_lines(condition.bitwidth(), 0u, lines2);
                std::vector<unsigned> product;
                get_reusable_constant_lines(condition.bitwidth(), 0u, product);

                copy(lines2.begin(), lines2.end(), back_inserter(product));

                multiplication_full(product, lhs, rhs);
                bitwise_cnot(lines, lines2); // duplicate result_line

                // additional reverse computing
                std::list<boost::tuple<bool, bool, unsigned>> extra_cl;
                extra_cl.push_back(used_const_lines.top());
                used_const_lines.pop();
                extra_cl.push_back(used_const_lines.top());
                used_const_lines.pop(); // bitwidth shoudln't be more than 1 so this is it
                if (garbagefree) {
                    reverse_multiplication_full(product, lhs, rhs);
                    release_constant_lines(extra_cl);
                }
            } break;

            case syrec::binary_expression::logical_and: // &&
            {
                conjunction(lines.at(0), lhs.at(0), rhs.at(0));
            } break;

            case syrec::binary_expression::logical_or: // ||
            {
                disjunction(lines.at(0), lhs.at(0), rhs.at(0));
            } break;

            case syrec::binary_expression::bitwise_and: // &
            {
                bitwise_and(lines, lhs, rhs);
            } break;

            case syrec::binary_expression::bitwise_or: // |
            {
                bitwise_or(lines, lhs, rhs);
            } break;

            case syrec::binary_expression::less_than: // <
            {
                less_than(lines.at(0), lhs, rhs);
            } break;

            case syrec::binary_expression::greater_than: // >
            {
                greater_than(lines.at(0), lhs, rhs);
            } break;

            case syrec::binary_expression::equals: // =
            {
                equals(lines.at(0), lhs, rhs);
            } break;

            case syrec::binary_expression::not_equals: // !=
            {
                not_equals(lines.at(0), lhs, rhs);
            } break;

            case syrec::binary_expression::less_equals: // <=
            {
                less_equals(lines.at(0), lhs, rhs);
            } break;

            case syrec::binary_expression::greater_equals: // >=
            {
                greater_equals(lines.at(0), lhs, rhs);
            } break;

            default:
                std::cout << "Binary Expression" << std::endl;
                return false;
        }

        std::vector<unsigned>                          lhs2, rhs2;
        std::list<boost::tuple<bool, bool, unsigned>>  lhs_cl, rhs_cl;
        std::stack<boost::tuple<bool, bool, unsigned>> rhs_se_cl;
        get_expr_lines(condition.rhs(), rhs2, rhs_cl);
        //rhs-Anhängsel vom stack nehmen
        if (syrec::variable_expression* exp = dynamic_cast<syrec::variable_expression*>(condition.rhs().get())) {
            syrec::variable_access::ptr var = exp->var();
            var                             = get_dupl(var);

            if (!var->indexes().empty()) {
                // check if it is not all numeric_expressions
                unsigned n = var->var()->dimensions().size(); // dimensions
                if ((unsigned)boost::count_if(var->indexes(), is_type<syrec::numeric_expression>()) != n) {
                    // lines used for evaluating index expressions
                    unsigned j = 1u;
                    for (unsigned i = 0u; i < var->indexes().size(); ++i) {
                        for (unsigned k = 0u; k < j; ++k) {
                            if (!get_subexpr_lines(var->indexes().at(i), rhs_se_cl)) return false;
                        }
                        j *= var->var()->dimensions().at(i);
                    }
                }
            }
        } else if (syrec::binary_expression* exp = dynamic_cast<syrec::binary_expression*>(condition.rhs().get())) {
            if (!get_subexpr_lines(exp->rhs(), rhs_se_cl) || !get_subexpr_lines(exp->lhs(), rhs_se_cl)) {
                return false;
            }
        } else if (syrec::shift_expression* exp = dynamic_cast<syrec::shift_expression*>(condition.rhs().get())) {
            if (!get_subexpr_lines(exp->lhs(), rhs_se_cl)) {
                return false;
            }
        }
        get_expr_lines(condition.lhs(), lhs2, lhs_cl);
        //rhs-anhängsel auf den stack zurücklegen
        while (!rhs_se_cl.empty()) {
            used_const_lines.push(rhs_se_cl.top());
            rhs_se_cl.pop();
        }
        return (off_expression(condition.rhs(), rhs2, rhs_cl) && off_expression(condition.lhs(), lhs2, lhs_cl));
    }

    bool garbagefree_syrec_synthesizer::on_condition(const syrec::numeric_expression& condition, unsigned result_line) {
        // sollte nur bei fi auftreten
        bool cond_value = condition.value()->evaluate(intern_variable_mapping);
        if (cond_value) {
            append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), result_line);
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::on_condition(const syrec::variable_expression& condition, unsigned result_line) {
        syrec::variable_access::ptr var = condition.var();
        std::vector<unsigned>       lines;
        bool                        copied_array = false;

        var = get_dupl(var);

        unsigned offset = var_lines()[var->var()];

        if (!var->indexes().empty()) {
            // change offset

            // check if it is all numeric_expressions
            unsigned n = var->var()->dimensions().size(); // dimensions
            if ((unsigned)boost::count_if(var->indexes(), is_type<syrec::numeric_expression>()) == n) {
                for (unsigned i = 0u; i < n; ++i) {
                    offset += dynamic_cast<syrec::numeric_expression*>(var->indexes().at(i).get())->value()->evaluate(intern_variable_mapping) *
                              std::accumulate(var->var()->dimensions().begin() + i + 1u, var->var()->dimensions().end(), 1u, std::multiplies<unsigned>()) *
                              var->var()->bitwidth();
                }
            } else {
                get_reusable_constant_lines(var->var()->bitwidth(), 0u, lines);
                if (!array_copying(offset, var->var()->dimensions(), var->indexes(), var->var()->bitwidth(), lines)) return false;
                copied_array = true;
            }
        }

        if (var->range()) {
            syrec::number::ptr nfirst, nsecond;
            boost::tie(nfirst, nsecond) = *var->range();

            unsigned first  = nfirst->evaluate(intern_variable_mapping);
            unsigned second = nsecond->evaluate(intern_variable_mapping);

            assert(first == second);
            UNUSED(second);
            if (copied_array) {
                append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), lines.at(first), result_line);
            } else {
                append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), (offset + first), result_line);
            }
        } else {
            if (copied_array) {
                append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), lines.at(0), result_line);
            } else {
                append_cnot(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), offset, result_line);
            }
        }

        if (copied_array) {
            return reverse_array_copying(offset, var->var()->dimensions(), var->indexes(), var->var()->bitwidth(), lines);
        }

        return true;
    }

    bool garbagefree_syrec_synthesizer::on_condition(const syrec::unary_expression& condition, unsigned result_line) {
        std::vector<unsigned> expr, expr2, lines;
        lines.push_back(result_line);

        if (!on_expression(condition.expr(), expr)) {
            return false;
        }

        switch (condition.op()) {
            case syrec::unary_expression::logical_not: // !
            {
                bitwise_negation(expr);
                gate::line_container controls(expr.begin(), expr.end());
                append_toffoli(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), controls, result_line);
                bitwise_negation(expr);
            } break;

            case syrec::unary_expression::bitwise_not: // ~
            {
                bitwise_cnot(lines, expr); // duplicate expr TODO nur wenn nötig -> Problem beim zurückrechnen
                bitwise_negation(lines);
            } break;

            default:
                std::cout << "Unary Expression" << std::endl;
                return false;
        }

        std::list<boost::tuple<bool, bool, unsigned>> expr_cl;
        get_expr_lines(condition.expr(), expr2, expr_cl);
        return off_expression(condition.expr(), expr2, expr_cl);
    }

    bool garbagefree_syrec_synthesizer::on_condition(const syrec::shift_expression& condition, unsigned result_line) {
        std::vector<unsigned> lhs, lhs2, lines;
        lines.push_back(result_line);

        if (!on_expression(condition.lhs(), lhs)) {
            return false;
        }

        unsigned rhs = condition.rhs()->evaluate(intern_variable_mapping);

        switch (condition.op()) {
            case syrec::shift_expression::left: // <<
            {
                left_shift(lines, lhs, rhs);
            } break;

            case syrec::shift_expression::right: // <<
            {
                right_shift(lines, lhs, rhs);
            } break;

            default:
                std::cout << "Shift Expression" << std::endl;
                return false;
        }

        std::list<boost::tuple<bool, bool, unsigned>> lhs_cl;
        get_expr_lines(condition.lhs(), lhs2, lhs_cl);
        return off_expression(condition.lhs(), lhs2, lhs_cl);
    }

    bool garbagefree_syrec_synthesizer::get_variables(syrec::variable_access::ptr var, std::vector<unsigned>& lines) {
        std::vector<unsigned> swap_lines;
        bool                  swapped_array = false;

        // std::cout << "getting var " << var->var()->name();
        var = get_dupl(var);
        // std::cout << " found " << var->var()->name() << std::endl;

        unsigned offset = var_lines()[var->var()];

        if (!var->indexes().empty()) {
            // change offset

            // check if it is all numeric_expressions
            unsigned n = var->var()->dimensions().size(); // dimensions
            if ((unsigned)boost::count_if(var->indexes(), is_type<syrec::numeric_expression>()) == n) {
                for (unsigned i = 0u; i < n; ++i) {
                    offset += dynamic_cast<syrec::numeric_expression*>(var->indexes().at(i).get())->value()->evaluate(intern_variable_mapping) *
                              std::accumulate(var->var()->dimensions().begin() + i + 1u, var->var()->dimensions().end(), 1u, std::multiplies<unsigned>()) *
                              var->var()->bitwidth();
                }
            } else {
                get_reusable_constant_lines(var->var()->bitwidth(), 0u, swap_lines);
                // lines auf denen die variable liegt werden auf den stack nach oben gelegt, um sie später wiederzufinden
                std::stack<boost::tuple<bool, bool, unsigned>> var_const_lines;
                for (unsigned i = 0u; i < var->var()->bitwidth(); ++i) {
                    var_const_lines.push(used_const_lines.top());
                    used_const_lines.pop();
                }
                if (!array_swapping(offset, var->var()->dimensions(), var->indexes(), var->var()->bitwidth(), swap_lines)) return false;
                swapped_array = true;
                while (!var_const_lines.empty()) {
                    used_const_lines.push(var_const_lines.top());
                    var_const_lines.pop();
                }
            }
        }

        if (var->range()) {
            syrec::number::ptr nfirst, nsecond;
            boost::tie(nfirst, nsecond) = *var->range();

            unsigned first  = nfirst->evaluate(intern_variable_mapping);
            unsigned second = nsecond->evaluate(intern_variable_mapping);

            if (first < second) {
                for (unsigned i = first; i <= second; ++i) {
                    if (swapped_array) {
                        lines += swap_lines.at(i);
                    } else {
                        lines += offset + i;
                    }
                }
            } else {
                for (int i = (int)first; i >= (int)second; --i) {
                    if (swapped_array) {
                        lines += swap_lines.at(i);
                    } else {
                        lines += offset + i;
                    }
                }
            }
        } else {
            for (unsigned i = 0u; i < var->var()->bitwidth(); ++i) {
                if (swapped_array) {
                    lines += swap_lines.at(i);
                } else {
                    lines += offset + i;
                }
            }
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::get_dupl_variables(syrec::variable_access::ptr var, std::vector<unsigned>& lines, std::map<std::pair<syrec::variable_access::ptr, unsigned>, syrec::variable_access::ptr, cmp_vptr> dupl_mapping) {
        // RANGE ignorieren
        // bei array-swap alles kopieren
        // TODO macht das Sinn???

        // std::cout << "getting var " << var->var()->name() << std::endl;
        for (unsigned i = depth; i > 0; --i) {
            if (dupl_mapping.find(std::make_pair(var, i)) != dupl_mapping.end()) {
                var = dupl_mapping.find(std::make_pair(var, i))->second;
            }
        }
        // std::cout << " found " << var->var()->name() << std::endl;

        unsigned limit  = var->var()->bitwidth();
        unsigned offset = var_lines()[var->var()];

        if (!var->indexes().empty()) {
            // change offset

            // check if it is all numeric_expressions
            bool     all_numbers = true;
            unsigned n           = var->var()->dimensions().size(); // dimensions
            if ((unsigned)boost::count_if(var->indexes(), is_type<syrec::numeric_expression>()) == n) {
                for (unsigned i = 0u; i < n; ++i) {
                    all_numbers &= dynamic_cast<syrec::numeric_expression*>(var->indexes().at(i).get())->value()->is_constant();
                }
                if (all_numbers) {
                    for (unsigned i = 0u; i < n; ++i) {
                        offset += dynamic_cast<syrec::numeric_expression*>(var->indexes().at(i).get())->value()->evaluate(intern_variable_mapping) *
                                  std::accumulate(var->var()->dimensions().begin() + i + 1u, var->var()->dimensions().end(), 1u, std::multiplies<unsigned>()) *
                                  var->var()->bitwidth();
                    }
                }
            } else {
                all_numbers = false;
            }
            if (!all_numbers) {
                limit = std::accumulate(var->var()->dimensions().begin(), var->var()->dimensions().end(), 1u, std::multiplies<unsigned>()) *
                        var->var()->bitwidth();
            }
        }

        for (unsigned i = 0u; i < limit; ++i) {
            lines += offset + i;
        }

        return true;
    }

    bool garbagefree_syrec_synthesizer::get_expr_variables(syrec::variable_access::ptr var, std::vector<unsigned>& lines) {
        std::vector<unsigned> copy_lines;
        bool                  copied_array = false;

        var = get_dupl(var);

        unsigned offset = var_lines()[var->var()];

        if (!var->indexes().empty()) {
            // change offset

            // check if it is all numeric_expressions
            unsigned n = var->var()->dimensions().size(); // dimensions
            if ((unsigned)boost::count_if(var->indexes(), is_type<syrec::numeric_expression>()) == n) {
                for (unsigned i = 0u; i < n; ++i) {
                    offset += dynamic_cast<syrec::numeric_expression*>(var->indexes().at(i).get())->value()->evaluate(intern_variable_mapping) *
                              std::accumulate(var->var()->dimensions().begin() + i + 1u, var->var()->dimensions().end(), 1u, std::multiplies<unsigned>()) *
                              var->var()->bitwidth();
                }
            } else {
                get_reusable_constant_lines(var->var()->bitwidth(), 0u, copy_lines);
                std::stack<boost::tuple<bool, bool, unsigned>> var_const_lines;
                for (unsigned i = 0u; i < var->var()->bitwidth(); ++i) {
                    var_const_lines.push(used_const_lines.top());
                    used_const_lines.pop();
                }
                if (!array_copying(offset, var->var()->dimensions(), var->indexes(), var->var()->bitwidth(), copy_lines)) return false;
                copied_array = true;
                while (!var_const_lines.empty()) {
                    used_const_lines.push(var_const_lines.top());
                    var_const_lines.pop();
                }
            }
        }

        if (var->range()) {
            syrec::number::ptr nfirst, nsecond;
            boost::tie(nfirst, nsecond) = *var->range();

            unsigned first  = nfirst->evaluate(intern_variable_mapping);
            unsigned second = nsecond->evaluate(intern_variable_mapping);

            if (first < second) {
                for (unsigned i = first; i <= second; ++i) {
                    if (copied_array) {
                        lines += copy_lines.at(i);
                    } else {
                        lines += offset + i;
                    }
                }
            } else {
                for (int i = (int)first; i >= (int)second; --i) {
                    if (copied_array) {
                        lines += copy_lines.at(i);
                    } else {
                        lines += offset + i;
                    }
                }
            }
        } else {
            for (unsigned i = 0u; i < var->var()->bitwidth(); ++i) {
                if (copied_array) {
                    lines += copy_lines.at(i);
                } else {
                    lines += offset + i;
                }
            }
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::unget_variables(syrec::variable_access::ptr var, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl) {
        var = get_dupl(var);

        if (!var->indexes().empty()) {
            // check if it is not all numeric_expressions
            unsigned n = var->var()->dimensions().size(); // dimensions
            if ((unsigned)boost::count_if(var->indexes(), is_type<syrec::numeric_expression>()) != n) {
                unsigned offset = var_lines()[var->var()];

                // das muss immer gemacht werden, auch wenn nicht garbagefree, der Unterschied ist nur in off-expression
                if (!reverse_array_swapping(offset, var->var()->dimensions(), var->indexes(), var->var()->bitwidth(), lines)) return false;
                release_constant_lines(expr_cl);
            }
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::unget_expr_variables(syrec::variable_access::ptr var, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl) {
        var = get_dupl(var);

        if (!var->indexes().empty()) {
            // check if it is not all numeric_expressions
            unsigned n = var->var()->dimensions().size(); // dimensions
            if ((unsigned)boost::count_if(var->indexes(), is_type<syrec::numeric_expression>()) != n) {
                unsigned offset = var_lines()[var->var()];

                if (!reverse_array_copying(offset, var->var()->dimensions(), var->indexes(), var->var()->bitwidth(), lines)) return false;
                if (garbagefree) {
                    release_constant_lines(expr_cl);
                }
            }
        }
        return true;
    }

    syrec::variable_access::ptr garbagefree_syrec_synthesizer::get_dupl(syrec::variable_access::ptr var) {
        /* std::cout << "get dupl for " << var->var()->name();
      if( var->range() ){
	syrec::number::ptr nfirst, nsecond;
        boost::tie( nfirst, nsecond ) = *var->range();
	std::cout << "." << *nfirst << ":" << *nsecond;
      }
      if( var->var()->dimensions().size() > 0 ){
	for ( unsigned i = 0; i < var->var()->dimensions().size(); ++i )
	{
	  std::cout << "[" << var->indexes().at( i ) << "]";
	}
      }
      std::cout << std::endl;*/
        for (unsigned i = depth; i > 0; --i) {
            if (dupl_if_var_mapping.find(std::make_pair(var, i)) != dupl_if_var_mapping.end()) {
                syrec::variable_access::ptr var_dupl = dupl_if_var_mapping.find(std::make_pair(var, i))->second;
                var_dupl->set_range(var->range());
                var_dupl->set_indexes(var->indexes());
                /*std::cout << "found " << var_dupl->var()->name();
      if( var_dupl->range() ){
	syrec::number::ptr nfirst, nsecond;
        boost::tie( nfirst, nsecond ) = *var_dupl->range();
	std::cout << "." << *nfirst << ":" << *nsecond;
      }
      if( var_dupl->var()->dimensions().size() > 0 ){
	for ( unsigned i = 0; i < var_dupl->var()->dimensions().size(); ++i )
	{
	  std::cout << "[" << var_dupl->indexes().at( i ) << "]";
	}
      }
      std::cout << std::endl;*/
                return var_dupl;
            }
        }
        return var;
    }

    // speichert in lines & expr_cl die für diesen Ausdruck benutzten Lines, entfernt sie in used_const_lines
    void garbagefree_syrec_synthesizer::get_expr_lines(syrec::expression::ptr expression, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl) {
        if (syrec::variable_expression* var = dynamic_cast<syrec::variable_expression*>(expression.get())) {
            return get_expr_lines(var->var(), lines, expr_cl);
        }

        std::stack<boost::tuple<bool, bool, unsigned>> extra_cl;
        unsigned                                       n = expression->bitwidth();
        if (syrec::binary_expression* expr = dynamic_cast<syrec::binary_expression*>(expression.get())) {
            /*if ( ( expr->op() == syrec::binary_expression::logical_and )
        || ( expr->op() == syrec::binary_expression::logical_or )
        || ( expr->op() == syrec::binary_expression::less_than )
        || ( expr->op() == syrec::binary_expression::greater_than )
        || ( expr->op() == syrec::binary_expression::equals )
        || ( expr->op() == syrec::binary_expression::not_equals )
        || ( expr->op() == syrec::binary_expression::less_equals )
        || ( expr->op() == syrec::binary_expression::greater_equals ) )
      {
        n = 1u;
      }
      else*/
            if ((expr->op() == syrec::binary_expression::modulo) || (expr->op() == syrec::binary_expression::frac_divide)) {
                for (unsigned i = 0u; i < expr->bitwidth(); ++i) {
                    extra_cl.push(used_const_lines.top());
                    used_const_lines.pop();
                }
            }
        }

        for (unsigned i = 0u; i < n; ++i) {
            boost::tuple<bool, bool, unsigned> const_line = used_const_lines.top();
            used_const_lines.pop();
            expr_cl.push_back(const_line);
            lines.insert(lines.begin(), const_line.get<2>());
        }

        while (!extra_cl.empty()) {
            used_const_lines.push(extra_cl.top());
            extra_cl.pop();
        }
    }

    void garbagefree_syrec_synthesizer::get_expr_lines(syrec::variable_access::ptr var, std::vector<unsigned>& lines, std::list<boost::tuple<bool, bool, unsigned>>& expr_cl) {
        bool expr_used_cl = true;

        var = get_dupl(var);

        unsigned offset = var_lines()[var->var()];

        if (!var->indexes().empty()) {
            // check if it is all numeric_expressions
            unsigned n = var->var()->dimensions().size();
            if ((unsigned)boost::count_if(var->indexes(), is_type<syrec::numeric_expression>()) == n) {
                expr_used_cl = false;
                for (unsigned i = 0u; i < n; ++i) {
                    offset += dynamic_cast<syrec::numeric_expression*>(var->indexes().at(i).get())->value()->evaluate(intern_variable_mapping) *
                              std::accumulate(var->var()->dimensions().begin() + i + 1u, var->var()->dimensions().end(), 1u, std::multiplies<unsigned>()) *
                              var->var()->bitwidth();
                }
            }
        } else {
            expr_used_cl = false;
        }

        if (var->range()) {
            syrec::number::ptr nfirst, nsecond;
            boost::tie(nfirst, nsecond) = *var->range();

            unsigned first  = nfirst->evaluate(intern_variable_mapping);
            unsigned second = nsecond->evaluate(intern_variable_mapping);

            if (first < second) {
                for (unsigned i = first; i <= second; ++i) {
                    if (expr_used_cl) {
                        boost::tuple<bool, bool, unsigned> const_line = used_const_lines.top();
                        used_const_lines.pop();
                        expr_cl.push_back(const_line);
                        lines.insert(lines.begin(), const_line.get<2>());
                    } else {
                        lines += offset + i;
                    }
                }
            } else {
                for (int i = (int)first; i >= (int)second; --i) {
                    if (expr_used_cl) {
                        boost::tuple<bool, bool, unsigned> const_line = used_const_lines.top();
                        used_const_lines.pop();
                        expr_cl.push_back(const_line);
                        lines.insert(lines.begin(), const_line.get<2>());
                    } else {
                        lines += offset + i;
                    }
                }
            }
            return;
        }

        for (unsigned i = 0u; i < var->var()->bitwidth(); ++i) {
            if (expr_used_cl) {
                boost::tuple<bool, bool, unsigned> const_line = used_const_lines.top();
                used_const_lines.pop();
                expr_cl.push_back(const_line);
                lines.insert(lines.begin(), const_line.get<2>());
            } else {
                lines += offset + i;
            }
        }
    }

    // geklaute Helper function
    unsigned getLowestOneBit2(unsigned number) {
        for (unsigned i = 0u; number > 0; ++i) {
            if (number % 2) return i;
            number /= 2;
        }
        return 0;
    }

    /**
   * Function to access array variables
   *
   * The array variable that corresponds to the given indexes is exchanged (via swap operations) with some given helper lines
   *
   * \param offset is the first line number associated to the array
   * \param dimensions is the dimensions of the array
   * \param indexes is the indexes of the array
   * \param bitwidth is the bitwidth of the variables within the array
   * \param lines is the destination, where
   */
    bool garbagefree_syrec_synthesizer::array_swapping(unsigned offset, std::vector<unsigned> dimensions, std::vector<std::shared_ptr<syrec::expression>> indexes, unsigned bitwidth, std::vector<unsigned>& lines) {
        if (indexes.empty()) {
            std::vector<unsigned> dest_lines;
            for (unsigned i = 0; i < bitwidth; ++i) {
                dest_lines += offset + i;
            }
            return swap(dest_lines, lines);
        }

        if (dynamic_cast<syrec::numeric_expression*>(indexes.at(0).get())) //( is_type<syrec::numeric_expression>( indexes.at( 0 ) ) )
        {
            offset += dynamic_cast<syrec::numeric_expression*>(indexes.at(0).get())->value()->evaluate(intern_variable_mapping) *
                      std::accumulate(dimensions.begin() + 1u, dimensions.end(), 1u, std::multiplies<unsigned>()) *
                      bitwidth;
            dimensions.erase(dimensions.begin());
            indexes.erase(indexes.begin());
            array_swapping(offset, dimensions, indexes, bitwidth, lines);
        } else {
            unsigned dimension = dimensions.at(0);
            dimensions.erase(dimensions.begin());
            std::vector<unsigned> select_lines;
            if (!on_expression(indexes.at(0), select_lines)) return false;
            indexes.erase(indexes.begin());

            unsigned max_dimension    = unsigned(pow(2, select_lines.size()));
            unsigned current_subarray = max_dimension - 1u;
            for (unsigned i = 1u; i <= max_dimension; ++i) {
                if (current_subarray < dimension) {
                    // activate controls (select_lines)
                    for (unsigned j = 0u; j < select_lines.size(); ++j) {
                        add_active_control(select_lines.at(j));
                    }

                    if (!array_swapping(offset + current_subarray * std::accumulate(dimensions.begin(), dimensions.end(), 1u, std::multiplies<unsigned>()) *
                                                         bitwidth,
                                        dimensions, indexes, bitwidth, lines)) return false;

                    // deactivate controls
                    for (unsigned j = 0u; j < select_lines.size(); ++j) {
                        remove_active_control(select_lines.at(j));
                    }
                }

                append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), select_lines.at((i < max_dimension ? getLowestOneBit2(i) : (select_lines.size() - 1u))));
                current_subarray ^= unsigned(pow(2, getLowestOneBit2(i)));
            }
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::array_copying(unsigned offset, std::vector<unsigned> dimensions, std::vector<std::shared_ptr<syrec::expression>> indexes, unsigned bitwidth, std::vector<unsigned>& lines) {
        if (indexes.empty()) {
            std::vector<unsigned> dest_lines;
            for (unsigned i = 0; i < bitwidth; ++i) {
                dest_lines += offset + i;
            }
            return bitwise_cnot(lines, dest_lines);
        }

        if (dynamic_cast<syrec::numeric_expression*>(indexes.at(0).get())) //( is_type<syrec::numeric_expression>( indexes.at( 0 ) ) )
        {
            offset += dynamic_cast<syrec::numeric_expression*>(indexes.at(0).get())->value()->evaluate(intern_variable_mapping) *
                      std::accumulate(dimensions.begin() + 1u, dimensions.end(), 1u, std::multiplies<unsigned>()) *
                      bitwidth;
            dimensions.erase(dimensions.begin());
            indexes.erase(indexes.begin());
            array_copying(offset, dimensions, indexes, bitwidth, lines);
        } else {
            unsigned dimension = dimensions.at(0);
            dimensions.erase(dimensions.begin());
            std::vector<unsigned> select_lines;
            if (!on_expression(indexes.at(0), select_lines)) return false;
            indexes.erase(indexes.begin());

            unsigned max_dimension    = unsigned(pow(2, select_lines.size()));
            unsigned current_subarray = max_dimension - 1u;
            for (unsigned i = 1u; i <= max_dimension; ++i) {
                if (current_subarray < dimension) {
                    // activate controls (select_lines)
                    for (unsigned j = 0u; j < select_lines.size(); ++j) {
                        add_active_control(select_lines.at(j));
                    }

                    if (!array_copying(offset + current_subarray * std::accumulate(dimensions.begin(), dimensions.end(), 1u, std::multiplies<unsigned>()) *
                                                        bitwidth,
                                       dimensions, indexes, bitwidth, lines)) return false;

                    // deactivate controls
                    for (unsigned j = 0u; j < select_lines.size(); ++j) {
                        remove_active_control(select_lines.at(j));
                    }
                }

                append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), select_lines.at((i < max_dimension ? getLowestOneBit2(i) : (select_lines.size() - 1u))));
                current_subarray ^= unsigned(pow(2, getLowestOneBit2(i)));
            }
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::reverse_array_swapping(unsigned offset, std::vector<unsigned> dimensions, std::vector<std::shared_ptr<syrec::expression>> indexes, unsigned bitwidth, std::vector<unsigned>& lines) {
        if (indexes.empty()) {
            std::vector<unsigned> dest_lines;
            for (unsigned i = 0; i < bitwidth; ++i) {
                dest_lines += offset + i;
            }
            return reverse_swap(dest_lines, lines);
        }

        if (dynamic_cast<syrec::numeric_expression*>(indexes.at(0).get())) //( is_type<syrec::numeric_expression>( indexes.at( 0 ) ) )
        {
            offset += dynamic_cast<syrec::numeric_expression*>(indexes.at(0).get())->value()->evaluate(intern_variable_mapping) *
                      std::accumulate(dimensions.begin() + 1u, dimensions.end(), 1u, std::multiplies<unsigned>()) *
                      bitwidth;
            dimensions.erase(dimensions.begin());
            indexes.erase(indexes.begin());
            reverse_array_swapping(offset, dimensions, indexes, bitwidth, lines);
        } else {
            syrec::expression::ptr index = indexes.at(0);
            indexes.erase(indexes.begin());
            unsigned dimension = dimensions.at(0);
            dimensions.erase(dimensions.begin());
            std::vector<unsigned>                          select_lines;
            std::list<boost::tuple<bool, bool, unsigned>>  index_cl;
            std::stack<boost::tuple<bool, bool, unsigned>> se_cl;
            unsigned                                       j = dimension; // TODO macht das Sinn?
            for (unsigned i = 0u; i < indexes.size(); ++i) {
                for (unsigned k = 0u; k < j; ++k) {
                    if (!get_subexpr_lines(indexes.at(i), se_cl)) return false;
                }
                j *= dimensions.at(i);
            }
            get_expr_lines(index, select_lines, index_cl);
            while (!se_cl.empty()) {
                used_const_lines.push(se_cl.top());
                se_cl.pop();
            }

            unsigned max_dimension    = unsigned(pow(2, select_lines.size()));
            unsigned current_subarray = max_dimension + (max_dimension / 2) - 1; // TODO stimmt das?

            for (unsigned i = max_dimension; i > 0u; --i) {
                append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), select_lines.at((i < max_dimension ? getLowestOneBit2(i) : (select_lines.size() - 1u))));
                current_subarray ^= unsigned(pow(2, getLowestOneBit2(i)));

                if (current_subarray < dimension) {
                    // activate controls (select_lines)
                    for (unsigned k = 0u; k < select_lines.size(); ++k) {
                        add_active_control(select_lines.at(k));
                    }

                    if (!reverse_array_swapping(offset + current_subarray * std::accumulate(dimensions.begin(), dimensions.end(), 1u, std::multiplies<unsigned>()) *
                                                                 bitwidth,
                                                dimensions, indexes, bitwidth, lines)) return false;

                    // deactivate controls
                    for (unsigned k = 0u; k < select_lines.size(); ++k) {
                        remove_active_control(select_lines.at(k));
                    }
                }
            }

            return off_expression(index, select_lines, index_cl);
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::reverse_array_copying(unsigned offset, std::vector<unsigned> dimensions, std::vector<std::shared_ptr<syrec::expression>> indexes, unsigned bitwidth, std::vector<unsigned>& lines) {
        if (indexes.empty()) {
            std::vector<unsigned> dest_lines;
            for (unsigned i = 0; i < bitwidth; ++i) {
                dest_lines += offset + i;
            }
            return (!garbagefree || reverse_bitwise_cnot(lines, dest_lines));
        }

        if (dynamic_cast<syrec::numeric_expression*>(indexes.at(0).get())) //( is_type<syrec::numeric_expression>( indexes.at( 0 ) ) )
        {
            offset += dynamic_cast<syrec::numeric_expression*>(indexes.at(0).get())->value()->evaluate(intern_variable_mapping) *
                      std::accumulate(dimensions.begin() + 1u, dimensions.end(), 1u, std::multiplies<unsigned>()) *
                      bitwidth;
            dimensions.erase(dimensions.begin());
            indexes.erase(indexes.begin());
            reverse_array_copying(offset, dimensions, indexes, bitwidth, lines);
        } else {
            syrec::expression::ptr index = indexes.at(0);
            indexes.erase(indexes.begin());
            unsigned dimension = dimensions.at(0);
            dimensions.erase(dimensions.begin());
            std::vector<unsigned>                          select_lines;
            std::list<boost::tuple<bool, bool, unsigned>>  index_cl;
            std::stack<boost::tuple<bool, bool, unsigned>> se_cl;
            unsigned                                       j = dimension; // TODO macht das Sinn?
            for (unsigned i = 0u; i < indexes.size(); ++i) {
                for (unsigned k = 0u; k < j; ++k) {
                    if (!get_subexpr_lines(indexes.at(i), se_cl)) return false;
                }
                j *= dimensions.at(i);
            }
            get_expr_lines(index, select_lines, index_cl);
            while (!se_cl.empty()) {
                used_const_lines.push(se_cl.top());
                se_cl.pop();
            }

            unsigned max_dimension    = unsigned(pow(2, select_lines.size()));
            unsigned current_subarray = max_dimension + (max_dimension / 2) - 1; // TODO stimmt das?

            for (unsigned i = max_dimension; i > 0u; --i) {
                if (garbagefree) {
                    append_not(*(get(boost::vertex_name, cct_man.tree)[cct_man.current].circ), select_lines.at((i < max_dimension ? getLowestOneBit2(i) : (select_lines.size() - 1u))));
                }
                current_subarray ^= unsigned(pow(2, getLowestOneBit2(i)));

                if (current_subarray < dimension) {
                    // activate controls (select_lines)
                    for (unsigned k = 0u; k < select_lines.size(); ++k) {
                        add_active_control(select_lines.at(k));
                    }

                    if (!reverse_array_copying(offset + current_subarray * std::accumulate(dimensions.begin(), dimensions.end(), 1u, std::multiplies<unsigned>()) *
                                                                bitwidth,
                                               dimensions, indexes, bitwidth, lines)) return false;

                    // deactivate controls
                    for (unsigned k = 0u; k < select_lines.size(); ++k) {
                        remove_active_control(select_lines.at(k));
                    }
                }
            }

            return off_expression(index, select_lines, index_cl);
        }
        return true;
    }

    //**********************************************************************
    //*****                     Efficient Controls                     *****
    //**********************************************************************

    bool garbagefree_syrec_synthesizer::assemble_circuit(const cct_node& current) {
        // leaf
        if (out_edges(current, cct_man.tree).first == out_edges(current, cct_man.tree).second /*get( boost::vertex_name, cct_man.tree )[current].circ.get()->num_gates() > 0u*/) {
            // std::cout << "leaf ";
            // std::cout << ( *( ( get( boost::vertex_name, cct_man.tree )[current].circ ).get() ) ).num_gates() << std::endl;
            append_circuit(circ(), *((get(boost::vertex_name, cct_man.tree)[current].circ).get()), get(boost::vertex_name, cct_man.tree)[current].controls);
            return true;
        }

        if (optimization_decision(current)) {
            // optimize the controlled cascade of the current node
            get_reusable_constant_line(false);
            boost::tuple<bool, bool, unsigned> helper_line = used_const_lines.top();
            used_const_lines.pop();
            gate::line_container controls;
            controls.insert(helper_line.get<2>());
            append_toffoli(circ(), get(boost::vertex_name, cct_man.tree)[current].controls, helper_line.get<2>());
            for (boost::graph_traits<cct>::out_edge_iterator edge_it = out_edges(current, cct_man.tree).first; edge_it != out_edges(current, cct_man.tree).second; ++edge_it) {
                if (!assemble_circuit(circ(), target(*edge_it, cct_man.tree), controls)) return false;
            }
            append_toffoli(circ(), get(boost::vertex_name, cct_man.tree)[current].controls, helper_line.get<2>());
            release_constant_line(helper_line);
        } else {
            // assemble optimized circuits of successors
            for (boost::graph_traits<cct>::out_edge_iterator edge_it = out_edges(current, cct_man.tree).first; edge_it != out_edges(current, cct_man.tree).second; ++edge_it) {
                if (!assemble_circuit(target(*edge_it, cct_man.tree))) return false;
            }
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::assemble_circuit(circuit& circ, const cct_node& current, gate::line_container controls) {
        // leaf
        if (out_edges(current, cct_man.tree).first == out_edges(current, cct_man.tree).second /*get( boost::vertex_name, cct_man.tree )[current].circ->num_gates() > 0*/) {
            // std::cout << " stdleaf ";
            // std::cout << ( *( ( get( boost::vertex_name, cct_man.tree )[current].circ ).get() ) ).num_gates() << std::endl;
            append_circuit(circ, *((get(boost::vertex_name, cct_man.tree)[current].circ).get()), controls);
            return true;
        }

        // std::cout << " stdsuccs";

        controls.insert(get(boost::vertex_name, cct_man.tree)[current].control);
        for (boost::graph_traits<cct>::out_edge_iterator edge_it = out_edges(current, cct_man.tree).first; edge_it != out_edges(current, cct_man.tree).second; ++edge_it) {
            if (!assemble_circuit(circ, target(*edge_it, cct_man.tree), controls)) return false;
        }
        return true;
    }

    bool garbagefree_syrec_synthesizer::optimization_decision(const cct_node& current) {
        if (efficient_controls) {
            return (optimizationCost(current) == bestCost(current)); // TODO: nur, wenn optimizationCost wirklich guenstiger
        }
        return false;
    }

    void garbagefree_syrec_synthesizer::initialize_changing_variables(const syrec::program& program) {
        // Compute changed variable map for if_realization via duplication
        if (if_realization == syrec_synthesis_if_realization_duplication) {
            compute_changing_variables(program, _changing_variables);
        }
    }

    void garbagefree_syrec_synthesizer::compute_changing_variables(const syrec::program& program, std::map<const syrec::statement*, var_set>& changing_variables) {
        for (syrec::module::ptr mod: program.modules()) {
            compute_changing_variables(mod, changing_variables);
        }
    }

    void garbagefree_syrec_synthesizer::compute_changing_variables(const syrec::module::ptr& module, std::map<const syrec::statement*, var_set>& changing_variables) {
        for (syrec::statement::ptr stat: module->statements()) {
            compute_changing_variables(stat, changing_variables);
        }
    }

    void garbagefree_syrec_synthesizer::compute_changing_variables(const syrec::statement::ptr statement, std::map<const syrec::statement*, var_set>& changing_variables) {
        var_set changed_variables;
        if (syrec::swap_statement* stat = dynamic_cast<syrec::swap_statement*>(statement.get())) {
            changed_variables.insert(stat->lhs());
            changed_variables.insert(stat->rhs());
            // changing_variables.insert( std::make_pair( stat, changed_variables ) );
            // return;
        } else if (syrec::unary_statement* stat = dynamic_cast<syrec::unary_statement*>(statement.get())) {
            changed_variables.insert(stat->var());
            // changing_variables.insert( std::make_pair( stat, changed_variables ) );
            // return;
        } else if (syrec::assign_statement* stat = dynamic_cast<syrec::assign_statement*>(statement.get())) {
            changed_variables.insert(stat->lhs());
            // changing_variables.insert( std::make_pair( stat, changed_variables ) );
            // return;
        } else if (syrec::if_statement* stat = dynamic_cast<syrec::if_statement*>(statement.get())) {
            for (syrec::statement::ptr s: stat->then_statements()) {
                compute_changing_variables(s, changing_variables);
                changed_variables.insert(changing_variables.find(s.get())->second.begin(), changing_variables.find(s.get())->second.end());
            }
            for (syrec::statement::ptr s: stat->else_statements()) {
                compute_changing_variables(s, changing_variables);
                changed_variables.insert(changing_variables.find(s.get())->second.begin(), changing_variables.find(s.get())->second.end());
            }
            // changing_variables.insert( std::make_pair( stat, changed_variables ) );
            // return;
        } else if (syrec::for_statement* stat = dynamic_cast<syrec::for_statement*>(statement.get())) {
            for (syrec::statement::ptr s: stat->statements()) {
                compute_changing_variables(s, changing_variables);
                changed_variables.insert(changing_variables.find(s.get())->second.begin(), changing_variables.find(s.get())->second.end());
            }
            // changing_variables.insert( std::make_pair( stat, changed_variables ) );
            // return;
        } else if (syrec::call_statement* stat = dynamic_cast<syrec::call_statement*>(statement.get())) {
            for (syrec::statement::ptr s: stat->target()->statements()) {
                compute_changing_variables(s, changing_variables);
                changed_variables.insert(changing_variables.find(s.get())->second.begin(), changing_variables.find(s.get())->second.end());
            }
            // changing_variables.insert( std::make_pair( stat, changed_variables ) );
            // return;
        } else if (syrec::uncall_statement* stat = dynamic_cast<syrec::uncall_statement*>(statement.get())) {
            for (syrec::statement::ptr s: stat->target()->statements()) {
                compute_changing_variables(s, changing_variables);
                changed_variables.insert(changing_variables.find(s.get())->second.begin(), changing_variables.find(s.get())->second.end());
            }
            // changing_variables.insert( std::make_pair( stat, changed_variables ) );
            // return;
        } else if (/*syrec::skip_statement* stat = */ dynamic_cast<syrec::skip_statement*>(statement.get())) {
            // changing_variables.insert( std::make_pair( stat, changed_variables ) );
            // return;
        } else {
            //std::cout << "compute_changing_variables" << std::endl;
            return;
        }
        //std::cout << "--- changing variables for statement " << *statement << std::endl;
        changing_variables.insert(std::make_pair(statement.get(), changed_variables));

        /*foreach_( syrec::variable_access::ptr v, changed_variables )
    {
      //std::cout << v->var()->name();
      //if( v->range() ){
	//syrec::number::ptr nfirst, nsecond;
      //  boost::tie( nfirst, nsecond ) = *v->range();
	//std::cout << "." << *nfirst << ":" << *nsecond;
      //}
      if( v->var()->dimensions().size() > 0 ){
	for ( unsigned i = 0; i < v->var()->dimensions().size(); ++i )
	{
	  std::cout << "[" << v->indexes().at( i ) << "]";
	}
      }
      std::cout << std::endl;
    }*/
    }

    bool syrec_synthesis_garbagefree(circuit& circ, const syrec::program& program, properties::ptr settings, properties::ptr statistics) {
        // just copied syrec_synthesis

        // Settings parsing
        std::string                   variable_name_format  = get<std::string>(settings, "variable_name_format", "%1$s%3$s.%2$d");
        std::string                   main_module           = get<std::string>(settings, "main_module", std::string());
        garbagefree_syrec_synthesizer statement_synthesizer = get<garbagefree_syrec_synthesizer>(settings, "statement_synthesizer", garbagefree_syrec_synthesizer(circ, program));

        statement_synthesizer.set_settings(settings);

        // Run-time measuring
        timer<properties_timer> t;

        if (statistics) {
            properties_timer rt(statistics);
            t.start(rt);
        }

        statement_synthesizer.initialize_changing_variables(program);
        //std::cout << "computed changing variables" << std::endl;

        // get the main module
        syrec::module::ptr main;

        if (!main_module.empty()) {
            main = program.find_module(main_module);
            if (!main) {
                std::cerr << "Program has no module: " << main_module << std::endl;
                return false;
            }
        } else {
            main = program.find_module("main");
            if (!main) {
                main = program.modules().front();
            }
        }

        // declare as top module
        statement_synthesizer.set_main_module(main);

        // create lines for global variables
        add_variables2(circ, statement_synthesizer, variable_name_format, main->parameters());
        add_variables2(circ, statement_synthesizer, variable_name_format, main->variables());

        // synthesize the statements
        return statement_synthesizer.on_module(main);
    }

} // namespace revkit
