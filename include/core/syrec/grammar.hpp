#pragma once

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/qi.hpp>
#include <iostream>
#include <optional>
#include <variant>

// Custom Parser iterator
namespace syrec::parser {
    BOOST_SPIRIT_TERMINAL(iter_pos)
} // namespace syrec::parser

namespace boost::spirit {
    template<>
    struct use_terminal<qi::domain, syrec::parser::tag::iter_pos>: mpl::true_ {};
} // namespace boost::spirit

namespace syrec::parser {
    struct IterPosParser: boost::spirit::qi::primitive_parser<IterPosParser> {
        template<typename Context, typename Iterator>
        // NOLINTNEXTLINE (readability-identifier-naming) this is required by boost
        struct attribute {
            using type = Iterator;
        };

        template<typename Iterator, typename Context, typename Skipper, typename Attribute>
        bool parse(Iterator& first, Iterator const& last, [[maybe_unused]] Context& contextUnused, Skipper const& skipper, Attribute& attr) const {
            boost::spirit::qi::skip_over(first, last, skipper);
            boost::spirit::traits::assign_to(first, attr);
            return true;
        }

        template<typename Context>
        boost::spirit::info what([[maybe_unused]] Context& contextUnused) const {
            return boost::spirit::info("iter_pos");
        }
    };
} // namespace syrec::parser

namespace boost::spirit::qi {
    template<typename Modifiers>
    struct make_primitive<syrec::parser::tag::iter_pos, Modifiers> {
        using result_type = syrec::parser::IterPosParser;

        result_type operator()([[maybe_unused]] unused_type unusedVarOne, [[maybe_unused]] unused_type unusedVarTwo) const {
            return {};
        }
    };
} // namespace boost::spirit::qi

namespace syrec {
    namespace qi    = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    struct ast_variable;
    struct ast_number_expression;
    struct ast_binary_expression;
    struct ast_shift_expression;
    struct ast_if_statement;
    struct ast_for_statement;

    using ast_iterator              = std::string::const_iterator;
    using ast_number                = std::variant<unsigned int, boost::recursive_wrapper<ast_variable>, std::string, boost::recursive_wrapper<ast_number_expression>>;
    using ast_range                 = boost::optional<boost::fusion::vector<syrec::ast_number, std::optional<syrec::ast_number>>>;
    using ast_expression            = boost::variant<ast_number, boost::recursive_wrapper<ast_variable>, boost::recursive_wrapper<ast_binary_expression>, boost::recursive_wrapper<ast_shift_expression>>;
    using ast_swap_statement        = boost::fusion::vector<ast_variable, ast_variable>;
    using ast_unary_statement       = boost::fusion::vector<std::string, ast_variable>;
    using ast_assign_statement      = boost::fusion::vector<ast_variable, char, ast_expression>;
    using ast_call_statement        = boost::fusion::vector<std::string, std::string, std::vector<std::string>>;
    using ast_statement             = boost::fusion::vector<ast_iterator, boost::variant<ast_swap_statement, ast_unary_statement, ast_assign_statement, boost::recursive_wrapper<ast_if_statement>, boost::recursive_wrapper<ast_for_statement>, ast_call_statement, std::string>>;
    using ast_variable_declaration  = boost::fusion::vector<std::string, std::vector<unsigned int>, boost::optional<unsigned int>>;
    using ast_variable_declarations = boost::fusion::vector<std::string, std::vector<ast_variable_declaration>>;
    using ast_parameter             = boost::fusion::vector<std::string, ast_variable_declaration>;
    using ast_module                = boost::fusion::vector<std::string, std::vector<ast_parameter>, std::vector<ast_variable_declarations>, std::vector<ast_statement>>;
    using ast_program               = std::vector<ast_module>;

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

    struct ast_shift_expression {
        ast_expression operand1;
        std::string    op;
        ast_number     operand2;
    };

    struct ast_if_statement {
        ast_expression             condition;
        std::vector<ast_statement> ifStatement;
        std::vector<ast_statement> elseStatement;
        ast_expression             fiCondition;
    };

    struct ast_for_statement {
        using from_t = boost::optional<boost::fusion::vector<std::optional<std::string>, ast_number>>;
        using step_t = boost::optional<boost::fusion::vector<std::optional<char>, ast_number>>;

        from_t                     from;
        ast_number                 to;
        step_t                     step;
        std::vector<ast_statement> doStatement;
    };
} // namespace syrec

BOOST_FUSION_ADAPT_STRUCT(
        syrec::ast_variable,
        (std::string, name)(std::vector<syrec::ast_expression>, indexes)(syrec::ast_range, range))

BOOST_FUSION_ADAPT_STRUCT(
        syrec::ast_number_expression,
        (syrec::ast_number, operand1)(std::string, op)(syrec::ast_number, operand2))

BOOST_FUSION_ADAPT_STRUCT(
        syrec::ast_binary_expression,
        (syrec::ast_expression, operand1)(std::string, op)(syrec::ast_expression, operand2))

BOOST_FUSION_ADAPT_STRUCT(
        syrec::ast_shift_expression,
        (syrec::ast_expression, operand1)(std::string, op)(syrec::ast_number, operand2))

BOOST_FUSION_ADAPT_STRUCT(
        syrec::ast_if_statement,
        (syrec::ast_expression, condition)(std::vector<syrec::ast_statement>, ifStatement)(std::vector<syrec::ast_statement>, elseStatement)(syrec::ast_expression, fiCondition))

BOOST_FUSION_ADAPT_STRUCT(
        syrec::ast_for_statement,
        (syrec::ast_for_statement::from_t, from)(syrec::ast_number, to)(syrec::ast_for_statement::step_t, step)(std::vector<syrec::ast_statement>, doStatement))

namespace syrec {

    template<typename Iterator>
    struct SyrecSkipParser: qi::grammar<Iterator> {
        explicit SyrecSkipParser():
            SyrecSkipParser::base_type(baseRule) {
            using ascii::space;
            using qi::char_;
            using qi::eol;
            using qi::lit;

            baseRule = lit(';') | space | singleLineCommentRule | multiLineCommentRule;

            singleLineCommentRule = "//" >> *(char_ - eol) >> eol;

            multiLineCommentRule = "/*" >> *(char_ - "*/") >> "*/";
        }

        qi::rule<Iterator> baseRule;
        qi::rule<Iterator> singleLineCommentRule;
        qi::rule<Iterator> multiLineCommentRule;
    };

    template<typename Iterator, typename SpaceT>
    struct SyrecParser: qi::grammar<Iterator, ast_program(), qi::locals<std::string>, SpaceT> {
        [[maybe_unused]] explicit SyrecParser():
            SyrecParser::base_type(programRule, "syrec") {
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

            programRule %= +moduleRule;

            variableDeclarationRule %= (identifier - lit("module")) >> *("[" >> uint_ >> "]") >> -("(" >> uint_ >> ")");

            variableDeclarationsRule %= (string("state") | string("wire")) >> (variableDeclarationRule % ',');

            moduleRule %= lit("module") > identifier >> '(' >> -(parameterRule % ',') >> ')' >> *variableDeclarationsRule >> +statementRule;

            parameterRule %= (string("inout") | string("in") | string("out")) >> variableDeclarationRule;

            swapStatementRule %= variableRule >> "<=>" >> variableRule;

            unaryStatementRule %= (string("~") | string("++") | string("--")) >> '=' >> variableRule;

            assignStatementRule %= variableRule >> char_("^+-") >> lit('=') >> expressionRule;

            ifStatementRule %= "if" >> expressionRule >> "then" >> +statementRule >> "else" >> +statementRule >> "fi" >> expressionRule;

            forStatementRule %= "for" >> -(-("$" >> identifier >> "=") >> numberRule >> "to") >> numberRule >> -("step" >> -char_('-') >> numberRule) >> "do" >> +statementRule >> "rof";

            callStatementRule %= (string("call") | string("uncall")) >> identifier >> -("(" >> -(identifier % ",") >> ")");

            skipRule %= string("skip");

            statementRule %= iter_pos >> (swapStatementRule | unaryStatementRule | assignStatementRule | ifStatementRule | forStatementRule | callStatementRule | skipRule);

            expressionRule %= numberRule | variableRule | binaryExpressionRule | shiftExpressionRule;

            binaryExpressionRule %= '(' >> expressionRule >> (string("+") | string("-") | string("^") | string("*") | string("/") | string("%") | string("&&") | string("||") | string("&") | string("|") | string("<=") | string(">=") | string("=") | string("!=") | string("<") | string(">")) >> expressionRule >> ')';

            shiftExpressionRule %= '(' >> expressionRule >> (string("<<") | string(">>")) >> numberRule >> ')';

            variableRule %= (identifier - lit("module")) >> *("[" >> expressionRule >> "]") >> -('.' >> numberRule >> -(':' >> numberRule));

            numberRule %= uint_ | ('#' >> variableRule) | ('$' >> identifier) | numberExpressionRule;

            numberExpressionRule %= '(' >> numberRule >> (string("+") | string("-") | string("*") | string("/") | string("%") | string("&&") | string("||") | string("&") | string("|") | string(">=") | string("<=") | string(">") | string("<") | string("==") | string("!=")) >> numberRule >> ')';

            programRule.name("program");
            variableDeclarationRule.name("variable_declaration");
            variableDeclarationsRule.name("variable_declarations");
            moduleRule.name("module");
            parameterRule.name("parameter");
            swapStatementRule.name("swap_statement");
            unaryStatementRule.name("unary_statement");
            assignStatementRule.name("assign_statement");
            ifStatementRule.name("if_statement");
            forStatementRule.name("for_statement");
            callStatementRule.name("call_statement");
            skipRule.name("skip");
            statementRule.name("statement");
            expressionRule.name("expression");
            binaryExpressionRule.name("binary_expression");
            shiftExpressionRule.name("shift_expression");
            variableRule.name("variable");
            numberRule.name("number");
            numberExpressionRule.name("number_expression");
            identifier.name("identifier");

            using qi::fail;
            using qi::on_error;

            using namespace qi::labels;
            using boost::phoenix::construct;
            using boost::phoenix::val;
        }

        qi::rule<Iterator, ast_program(), qi::locals<std::string>, SpaceT>               programRule;
        qi::rule<Iterator, ast_variable_declaration(), qi::locals<std::string>, SpaceT>  variableDeclarationRule;
        qi::rule<Iterator, ast_variable_declarations(), qi::locals<std::string>, SpaceT> variableDeclarationsRule;
        qi::rule<Iterator, ast_module(), qi::locals<std::string>, SpaceT>                moduleRule;
        qi::rule<Iterator, ast_parameter(), qi::locals<std::string>, SpaceT>             parameterRule;
        qi::rule<Iterator, ast_swap_statement(), qi::locals<std::string>, SpaceT>        swapStatementRule;
        qi::rule<Iterator, ast_unary_statement(), qi::locals<std::string>, SpaceT>       unaryStatementRule;
        qi::rule<Iterator, ast_assign_statement(), qi::locals<std::string>, SpaceT>      assignStatementRule;
        qi::rule<Iterator, ast_if_statement(), qi::locals<std::string>, SpaceT>          ifStatementRule;
        qi::rule<Iterator, ast_for_statement(), qi::locals<std::string>, SpaceT>         forStatementRule;
        qi::rule<Iterator, ast_call_statement(), qi::locals<std::string>, SpaceT>        callStatementRule;
        qi::rule<Iterator, std::string(), qi::locals<std::string>, SpaceT>               skipRule;
        qi::rule<Iterator, ast_statement(), qi::locals<std::string>, SpaceT>             statementRule;
        qi::rule<Iterator, ast_expression(), qi::locals<std::string>, SpaceT>            expressionRule;
        qi::rule<Iterator, ast_binary_expression(), qi::locals<std::string>, SpaceT>     binaryExpressionRule;
        qi::rule<Iterator, ast_shift_expression(), qi::locals<std::string>, SpaceT>      shiftExpressionRule;
        qi::rule<Iterator, ast_variable(), qi::locals<std::string>, SpaceT>              variableRule;
        qi::rule<Iterator, ast_number(), qi::locals<std::string>, SpaceT>                numberRule;
        qi::rule<Iterator, ast_number_expression(), qi::locals<std::string>, SpaceT>     numberExpressionRule;

        qi::rule<Iterator, std::string(), qi::locals<std::string>, SpaceT> identifier;
    };

    template<typename Iterator>
    bool parse(ast_program& prog, Iterator first, Iterator last) {
        SyrecParser<Iterator, SyrecSkipParser<Iterator>> parser;
        SyrecSkipParser<Iterator>                        skipParser;

        bool r = qi::phrase_parse(first, last,
                                  parser,
                                  skipParser,
                                  prog);

        if (!r || first != last) {
            std::cerr << "ERROR AT: " << std::string(first, last) << std::endl;
            return false;
        }

        return true;
    }

    inline bool parseString(ast_program& prog, const std::string& program) {
        return parse(prog, program.begin(), program.end());
    }

    class program;
} // namespace syrec
