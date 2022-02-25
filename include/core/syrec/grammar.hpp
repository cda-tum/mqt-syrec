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

/** @cond 0 */
#ifndef GRAMMAR_HPP
#define GRAMMAR_HPP

#include <boost/config/warning_disable.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/variant.hpp>
#include <iostream>

// Custom Parser iterator
namespace revkit::syrec::parser {
            BOOST_SPIRIT_TERMINAL(iter_pos)
        } // namespace revkit

namespace boost::spirit {
        template<>
        struct use_terminal<qi::domain, revkit::syrec::parser::tag::iter_pos>: mpl::true_ {};
    } // namespace boost

namespace revkit::syrec::parser {
            struct iter_pos_parser: boost::spirit::qi::primitive_parser<iter_pos_parser> {
                template<typename Context, typename Iterator>
                struct attribute {
                    typedef Iterator type;
                };

                template<typename Iterator, typename Context, typename Skipper, typename Attribute>
                bool parse(Iterator& first, Iterator const& last, Context&, Skipper const& skipper, Attribute& attr) const {
                    boost::spirit::qi::skip_over(first, last, skipper);
                    boost::spirit::traits::assign_to(first, attr);
                    return true;
                }

                template<typename Context>
                boost::spirit::info what(Context&) const {
                    return boost::spirit::info("iter_pos");
                }
            };
        } // namespace revkit

namespace boost::spirit::qi {
            template<typename Modifiers>
            struct [[maybe_unused]] make_primitive<revkit::syrec::parser::tag::iter_pos, Modifiers> {
                typedef revkit::syrec::parser::iter_pos_parser result_type;

                result_type operator()(unused_type, unused_type) const {
                    return {};
                }
            };
        } // namespace boost

namespace revkit::syrec {
        namespace qi    = boost::spirit::qi;
        namespace ascii = boost::spirit::ascii;

        struct ast_variable;
        struct ast_number_expression;
        struct ast_binary_expression;
        struct ast_unary_expression;
        struct ast_shift_expression;
        struct ast_if_statement;
        struct ast_for_statement;

        typedef std::string::const_iterator                                                                                                                                                                                         ast_iterator;
        typedef boost::variant<unsigned, boost::recursive_wrapper<ast_variable>, std::string, boost::recursive_wrapper<ast_number_expression>>                                                                                      ast_number;
        typedef boost::optional<boost::fusion::vector<revkit::syrec::ast_number, boost::optional<revkit::syrec::ast_number>>>                                                                                                       ast_range;
        typedef boost::variant<ast_number, boost::recursive_wrapper<ast_variable>, boost::recursive_wrapper<ast_binary_expression>, boost::recursive_wrapper<ast_unary_expression>, boost::recursive_wrapper<ast_shift_expression>> ast_expression;
        typedef boost::fusion::vector<ast_variable, ast_variable>                                                                                                                                                                   ast_swap_statement;
        typedef boost::fusion::vector<std::string, ast_variable>                                                                                                                                                                    ast_unary_statement;
        typedef boost::fusion::vector<ast_variable, char, ast_expression>                                                                                                                                                           ast_assign_statement;
        typedef boost::fusion::vector<std::string, std::string, std::vector<std::string>>                                                                                                                                           ast_call_statement;
        typedef boost::fusion::vector<ast_iterator, boost::variant<ast_swap_statement,
                                                                   ast_unary_statement,
                                                                   ast_assign_statement,
                                                                   boost::recursive_wrapper<ast_if_statement>,
                                                                   boost::recursive_wrapper<ast_for_statement>,
                                                                   ast_call_statement,
                                                                   std::string>>
                                                                                                                                                   ast_statement;
        typedef boost::fusion::vector<std::string, std::vector<unsigned>, boost::optional<unsigned>>                                               ast_variable_declaration;
        typedef boost::fusion::vector<std::string, std::vector<ast_variable_declaration>>                                                          ast_variable_declarations;
        typedef boost::fusion::vector<std::string, ast_variable_declaration>                                                                       ast_parameter;
        typedef boost::fusion::vector<std::string, std::vector<ast_parameter>, std::vector<ast_variable_declarations>, std::vector<ast_statement>> ast_module;
        typedef std::vector<ast_module>                                                                                                            ast_program;

        struct ast_variable {
            std::string                 name;
            std::vector<ast_expression> indexes;
            ast_range                   range;
        };

        struct ast_number_expression {
            ast_number  operand1;
            std::string op;
            ast_number  operand2;
        };

        struct ast_binary_expression {
            ast_expression operand1;
            std::string    op;
            ast_expression operand2;
        };

        struct ast_unary_expression {
            std::string    op;
            ast_expression operand;
        };

        struct ast_shift_expression {
            ast_expression operand1;
            std::string    op;
            ast_number     operand2;
        };

        struct ast_if_statement {
            ast_expression             condition;
            std::vector<ast_statement> if_statement;
            std::vector<ast_statement> else_statement;
            ast_expression             fi_condition;
        };

        struct ast_for_statement {
            typedef boost::optional<boost::fusion::vector<boost::optional<std::string>, ast_number>> from_t;
            typedef boost::optional<boost::fusion::vector<boost::optional<char>, ast_number>>        step_t;

            from_t                     from;
            ast_number                 to;
            step_t                     step;
            std::vector<ast_statement> do_statement;
        };
    } // namespace revkit

BOOST_FUSION_ADAPT_STRUCT(
        revkit::syrec::ast_variable,
        (std::string, name)(std::vector<revkit::syrec::ast_expression>, indexes)(revkit::syrec::ast_range, range))

BOOST_FUSION_ADAPT_STRUCT(
        revkit::syrec::ast_number_expression,
        (revkit::syrec::ast_number, operand1)(std::string, op)(revkit::syrec::ast_number, operand2))

BOOST_FUSION_ADAPT_STRUCT(
        revkit::syrec::ast_binary_expression,
        (revkit::syrec::ast_expression, operand1)(std::string, op)(revkit::syrec::ast_expression, operand2))

BOOST_FUSION_ADAPT_STRUCT(
        revkit::syrec::ast_unary_expression,
        (std::string, op)(revkit::syrec::ast_expression, operand))

BOOST_FUSION_ADAPT_STRUCT(
        revkit::syrec::ast_shift_expression,
        (revkit::syrec::ast_expression, operand1)(std::string, op)(revkit::syrec::ast_number, operand2))

BOOST_FUSION_ADAPT_STRUCT(
        revkit::syrec::ast_if_statement,
        (revkit::syrec::ast_expression, condition)(std::vector<revkit::syrec::ast_statement>, if_statement)(std::vector<revkit::syrec::ast_statement>, else_statement)(revkit::syrec::ast_expression, fi_condition))

BOOST_FUSION_ADAPT_STRUCT(
        revkit::syrec::ast_for_statement,
        (revkit::syrec::ast_for_statement::from_t, from)(revkit::syrec::ast_number, to)(revkit::syrec::ast_for_statement::step_t, step)(std::vector<revkit::syrec::ast_statement>, do_statement))

namespace revkit {
    namespace syrec {

        template<typename Iterator>
        struct syrec_skip_parser: qi::grammar<Iterator> {
            explicit syrec_skip_parser():
                syrec_skip_parser::base_type(base_rule) {
                using ascii::space;
                using qi::char_;
                using qi::eol;
                using qi::lit;

                base_rule = lit(';') | space | single_line_comment_rule | multi_line_comment_rule;

                single_line_comment_rule = "//" >> *(char_ - eol) >> eol;

                multi_line_comment_rule = "/*" >> *(char_ - "*/") >> "*/";
            }

            qi::rule<Iterator> base_rule;
            qi::rule<Iterator> single_line_comment_rule;
            qi::rule<Iterator> multi_line_comment_rule;
        };

        template<typename Iterator, typename SpaceT>
        struct syrec_parser: qi::grammar<Iterator, ast_program(), qi::locals<std::string>, SpaceT> {
            explicit syrec_parser():
                syrec_parser::base_type(program_rule, "syrec") {
                using ascii::alnum;
                using parser::iter_pos;
                using qi::_1;
                using qi::_val;
                using qi::char_;
                using qi::eol;
                using qi::lexeme;
                using qi::lit;
                using qi::string;
                using qi::uint_;

                // Helpers
                identifier %= lexeme[+(alnum | '_')];

                program_rule %= +module_rule;

                variable_declaration_rule %= (identifier - lit("module")) >> *("[" >> uint_ >> "]") >> -("(" >> uint_ >> ")");

                variable_declarations_rule %= (string("state") | string("wire")) >> (variable_declaration_rule % ',');

                module_rule %= lit("module") > identifier >> '(' >> -(parameter_rule % ',') >> ')' >> *variable_declarations_rule >> +statement_rule;

                parameter_rule %= (string("inout") | string("in") | string("out")) >> variable_declaration_rule;

                swap_statement_rule %= variable_rule >> "<=>" >> variable_rule;

                unary_statement_rule %= (string("~") | string("++") | string("--")) >> '=' >> variable_rule;

                assign_statement_rule %= variable_rule >> char_("^+-") >> lit('=') >> expression_rule;

                if_statement_rule %= "if" >> expression_rule >> "then" >> +statement_rule >> "else" >> +statement_rule >> "fi" >> expression_rule;

                for_statement_rule %= "for" >> -(-("$" >> identifier >> "=") >> number_rule >> "to") >> number_rule >> -("step" >> -char_('-') >> number_rule) >> "do" >> +statement_rule >> "rof";

                call_statement_rule %= (string("call") | string("uncall")) >> identifier >> -("(" >> -(identifier % ",") >> ")");

                skip_rule %= string("skip");

                statement_rule %= iter_pos >> (swap_statement_rule | unary_statement_rule | assign_statement_rule | if_statement_rule | for_statement_rule | call_statement_rule | skip_rule);

                expression_rule %= number_rule | variable_rule | binary_expression_rule | unary_expression_rule | shift_expression_rule;

                binary_expression_rule %= '(' >> expression_rule >> (string("+") | string("-") | string("^") | string("*>") | string("*") | string("/") | string("%") | string("&&") | string("||") | string("&") | string("|") | string("<=") | string(">=") | string("=") | string("!=") | string("<") | string(">")) >> expression_rule >> ')';

                unary_expression_rule %= (string("!") | string("~")) >> expression_rule;

                shift_expression_rule %= '(' >> expression_rule >> (string("<<") | string(">>")) >> number_rule >> ')';

                variable_rule %= (identifier - lit("module")) >> *("[" >> expression_rule >> "]") >> -('.' >> number_rule >> -(':' >> number_rule));

                number_rule %= uint_ | ('#' >> variable_rule) | ('$' >> identifier) | number_expression_rule;

                number_expression_rule %= '(' >> number_rule >> (string("+") | string("-") | string("*") | string("/")) >> number_rule >> ')';

                program_rule.name("program");
                variable_declaration_rule.name("variable_declaration");
                variable_declarations_rule.name("variable_declarations");
                module_rule.name("module");
                parameter_rule.name("parameter");
                swap_statement_rule.name("swap_statement");
                unary_statement_rule.name("unary_statement");
                assign_statement_rule.name("assign_statement");
                if_statement_rule.name("if_statement");
                for_statement_rule.name("for_statement");
                call_statement_rule.name("call_statement");
                skip_rule.name("skip");
                statement_rule.name("statement");
                expression_rule.name("expression");
                binary_expression_rule.name("binary_expression");
                unary_expression_rule.name("unary_expression");
                shift_expression_rule.name("shift_expression");
                variable_rule.name("variable");
                number_rule.name("number");
                number_expression_rule.name("number_expression");
                identifier.name("identifier");

                using qi::fail;
                using qi::on_error;

                using namespace qi::labels;
                using boost::phoenix::construct;
                using boost::phoenix::val;

                /*on_error<fail>
        (
         program_rule,
         std::cout << val( "Error! Expecting " ) << _4 << val( " here: \"" ) << construct<std::string>( _3, _2 ) << val( "\"" ) << std::endl
         );*/
            }

            qi::rule<Iterator, ast_program(), qi::locals<std::string>, SpaceT>               program_rule;
            qi::rule<Iterator, ast_variable_declaration(), qi::locals<std::string>, SpaceT>  variable_declaration_rule;
            qi::rule<Iterator, ast_variable_declarations(), qi::locals<std::string>, SpaceT> variable_declarations_rule;
            qi::rule<Iterator, ast_module(), qi::locals<std::string>, SpaceT>                module_rule;
            qi::rule<Iterator, ast_parameter(), qi::locals<std::string>, SpaceT>             parameter_rule;
            qi::rule<Iterator, ast_swap_statement(), qi::locals<std::string>, SpaceT>        swap_statement_rule;
            qi::rule<Iterator, ast_unary_statement(), qi::locals<std::string>, SpaceT>       unary_statement_rule;
            qi::rule<Iterator, ast_assign_statement(), qi::locals<std::string>, SpaceT>      assign_statement_rule;
            qi::rule<Iterator, ast_if_statement(), qi::locals<std::string>, SpaceT>          if_statement_rule;
            qi::rule<Iterator, ast_for_statement(), qi::locals<std::string>, SpaceT>         for_statement_rule;
            qi::rule<Iterator, ast_call_statement(), qi::locals<std::string>, SpaceT>        call_statement_rule;
            qi::rule<Iterator, std::string(), qi::locals<std::string>, SpaceT>               skip_rule;
            qi::rule<Iterator, ast_statement(), qi::locals<std::string>, SpaceT>             statement_rule;
            qi::rule<Iterator, ast_expression(), qi::locals<std::string>, SpaceT>            expression_rule;
            qi::rule<Iterator, ast_binary_expression(), qi::locals<std::string>, SpaceT>     binary_expression_rule;
            qi::rule<Iterator, ast_unary_expression(), qi::locals<std::string>, SpaceT>      unary_expression_rule;
            qi::rule<Iterator, ast_shift_expression(), qi::locals<std::string>, SpaceT>      shift_expression_rule;
            qi::rule<Iterator, ast_variable(), qi::locals<std::string>, SpaceT>              variable_rule;
            qi::rule<Iterator, ast_number(), qi::locals<std::string>, SpaceT>                number_rule;
            qi::rule<Iterator, ast_number_expression(), qi::locals<std::string>, SpaceT>     number_expression_rule;

            qi::rule<Iterator, std::string(), qi::locals<std::string>, SpaceT> identifier;
        };

        template<typename Iterator>
        bool parse(ast_program& prog, Iterator first, Iterator last) {
            syrec_parser<Iterator, syrec_skip_parser<Iterator>> parser;
            syrec_skip_parser<Iterator>                         skip_parser;

            bool r = qi::phrase_parse(first, last,
                                      parser,
                                      skip_parser,
                                      prog);

            if (!r || first != last) {
                std::cerr << "ERROR AT: " << std::string(first, last) << std::endl;
                return false;
            }

            return true;
        }
    } // namespace syrec

    bool parse(syrec::ast_program& prog, const std::string& filename);
    bool parse_string(syrec::ast_program& prog, const std::string& program);

} // namespace revkit

/** @endcond */

#endif /* GRAMMAR_HPP */
