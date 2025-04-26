/*
 * Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
 * Copyright (c) 2025 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#include "core/syrec/parser.hpp"

#include "core/syrec/expression.hpp"
#include "core/syrec/grammar.hpp"
#include "core/syrec/module.hpp"
#include "core/syrec/number.hpp"
#include "core/syrec/statement.hpp"
#include "core/syrec/variable.hpp"

#include <algorithm>
#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <boost/variant/detail/apply_visitor_unary.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace syrec {

    struct ParseNumberVisitor {
        explicit ParseNumberVisitor(const Module& proc, ParserContext& context):
            proc(proc),
            context(context) {}

        Number::ptr operator()(unsigned value) const {
            return std::make_shared<Number>(value);
        }

        Number::ptr operator()(const boost::recursive_wrapper<ast_variable>& astVar) const {
            const auto var = proc.findParameterOrVariable(astVar.get().name);
            if (!var) {
                context.errorMessage = "Unknown variable " + astVar.get().name;
                return {};
            }

            return std::make_shared<Number>(var->bitwidth);
        }

        Number::ptr operator()(const std::string& loopVariable) const {
            if (std::find(context.loopVariables.begin(), context.loopVariables.end(), loopVariable) != context.loopVariables.end()) {
                return std::make_shared<Number>(loopVariable);
            }
            context.errorMessage = "Unknown loop variable $" + loopVariable;
            return {};
        }

        Number::ptr operator()(const boost::recursive_wrapper<ast_number_expression>& astNe) const {
            const auto& astNo1 = astNe.get().operand1;
            const auto& astOp  = astNe.get().op;
            const auto& astNo2 = astNe.get().operand2;

            unsigned op = 0U;
            if (astOp == "+") {
                op = NumericExpression::Add;
            } else if (astOp == "-") {
                op = NumericExpression::Subtract;
            } else if (astOp == "*") {
                op = NumericExpression::Multiply;
            } else if (astOp == "/") {
                op = NumericExpression::Divide;
            } else if (astOp == "%") {
                op = NumericExpression::Modulo;
            } else if (astOp == "&&") {
                op = NumericExpression::LogicalAnd;
            }

            else if (astOp == "||") {
                op = NumericExpression::LogicalOr;
            } else if (astOp == "&") {
                op = NumericExpression::BitwiseAnd;
            } else if (astOp == "|") {
                op = NumericExpression::BitwiseOr;
            } else if (astOp == ">") {
                op = NumericExpression::GreaterThan;
            } else if (astOp == "<") {
                op = NumericExpression::LessThan;
            } else if (astOp == ">=") {
                op = NumericExpression::GreaterEquals;
            } else if (astOp == "<=") {
                op = NumericExpression::LessEquals;
            } else if (astOp == "==") {
                op = NumericExpression::Equals;
            } else if (astOp == "!=") {
                op = NumericExpression::NotEquals;
            }

            const auto& lhs = parseNumber(astNo1, proc, context);
            if (!lhs) {
                return {};
            }

            const auto& rhs = parseNumber(astNo2, proc, context);
            if (!rhs) {
                return {};
            }
            if (lhs->isConstant() && rhs->isConstant()) {
                const auto lhsValue = lhs->evaluate(Number::loop_variable_mapping());
                const auto rhsValue = rhs->evaluate(Number::loop_variable_mapping());
                unsigned   numValue = 0;

                switch (op) {
                    case NumericExpression::Add: // +
                    {
                        numValue = lhsValue + rhsValue;
                    } break;

                    case NumericExpression::Subtract: // -
                    {
                        numValue = lhsValue - rhsValue;
                    } break;

                    case NumericExpression::Multiply: // *
                    {
                        numValue = lhsValue * rhsValue;
                    } break;

                    case NumericExpression::Divide: // /
                    {
                        numValue = lhsValue / rhsValue;
                    } break;

                    case NumericExpression::Modulo: // /
                    {
                        numValue = lhsValue % rhsValue;
                    } break;

                    case NumericExpression::LogicalAnd: // /
                    {
                        numValue = static_cast<unsigned>((lhsValue != 0U) && (rhsValue != 0U));
                    } break;

                    case NumericExpression::LogicalOr: // /
                    {
                        numValue = static_cast<unsigned>((lhsValue != 0U) || (rhsValue != 0U));
                    } break;

                    case NumericExpression::BitwiseAnd: // /
                    {
                        numValue = lhsValue & rhsValue;
                    } break;

                    case NumericExpression::BitwiseOr: // /
                    {
                        numValue = lhsValue | rhsValue;
                    } break;

                    case NumericExpression::LessThan: // /
                    {
                        numValue = static_cast<unsigned int>(lhsValue < rhsValue);
                    } break;

                    case NumericExpression::GreaterThan: // /
                    {
                        numValue = static_cast<unsigned int>(lhsValue > rhsValue);
                    } break;

                    case NumericExpression::GreaterEquals: // /
                    {
                        numValue = static_cast<unsigned int>(lhsValue >= rhsValue);
                    } break;

                    case NumericExpression::LessEquals: // /
                    {
                        numValue = static_cast<unsigned int>(lhsValue <= rhsValue);
                    } break;

                    case NumericExpression::Equals: // /
                    {
                        numValue = static_cast<unsigned int>(lhsValue == rhsValue);
                    } break;

                    case NumericExpression::NotEquals: // /
                    {
                        numValue = static_cast<unsigned int>(lhsValue != rhsValue);
                    } break;

                    default:
                        return {};
                }

                return std::make_shared<Number>(numValue);
            }
            return std::make_shared<Number>(0);
        }

    private:
        const Module&  proc;    // NOLINT(*-avoid-const-or-ref-data-members)
        ParserContext& context; // NOLINT(*-avoid-const-or-ref-data-members)
    };

    Number::ptr parseNumber(const ast_number& astNum, const Module& proc, ParserContext& context) {
        return std::visit(ParseNumberVisitor(proc, context), astNum);
    }

    VariableAccess::ptr parseVariableAccess(const ast_variable& astVar, const Module& proc, ParserContext& context) {
        const auto& var = proc.findParameterOrVariable(astVar.name);

        if (!var) {
            context.errorMessage = "Unknown variable %s" + astVar.name;
            return {};
        }

        VariableAccess::ptr va(new VariableAccess());
        va->setVar(var);

        std::optional<std::pair<Number::ptr, Number::ptr>> varRange;

        if (const auto& range = astVar.range) {
            const auto& first = parseNumber(boost::fusion::at_c<0>(*range), proc, context);
            if (!first) {
                return {};
            }

            // is in range?
            if (!first->isLoopVariable()) {
                const auto bound = first->evaluate(Number::loop_variable_mapping());
                if (bound >= var->bitwidth) {
                    context.errorMessage = "Bound " + std::to_string(bound) + " out of range in variable " + var->name + "(" + std::to_string(var->bitwidth) + ")";
                    return {};
                }
            }

            Number::ptr second = first;

            if (boost::fusion::at_c<1>(*range)) {
                second = parseNumber(*boost::fusion::at_c<1>(*range), proc, context);
                if (!second) {
                    return {};
                }

                // is in range?
                if (!second->isLoopVariable()) {
                    const auto bound = second->evaluate(Number::loop_variable_mapping());
                    if (bound >= var->bitwidth) {
                        context.errorMessage = "Bound " + std::to_string(bound) + " out of range in variable " + var->name + "(" + std::to_string(var->bitwidth) + ")";
                        return {};
                    }
                }
            }

            varRange = std::make_pair(first, second);
        }

        va->range = varRange;

        // indexes
        if (var->dimensions.size() != astVar.indexes.size()) {
            context.errorMessage = "Invalid number of array indexes in variable " + var->name + ". Expected " + std::to_string(var->dimensions.size()) + ", got " + std::to_string(astVar.indexes.size());
            return {};
        }

        Expression::vec indexes;
        for (const ast_expression& astExp: astVar.indexes) {
            const auto index = parseExpression(astExp, proc, var->bitwidth, context);
            if (!index) {
                return {};
            }
            indexes.emplace_back(index);
        }
        va->indexes = indexes;

        return va;
    }

    struct ExpressionVisitor {
        ExpressionVisitor(const Module& proc, unsigned bitwidth, ParserContext& context):
            proc(proc),
            bitwidth(bitwidth),
            context(context) {}

        Expression* operator()(const ast_number& astNum) const {
            const auto num = parseNumber(astNum, proc, context);
            if (!num) {
                return nullptr;
            }
            return new NumericExpression(num, bitwidth);
        }

        Expression* operator()(const ast_variable& astVar) const {
            const auto access = parseVariableAccess(astVar, proc, context);
            if (!access) {
                return nullptr;
            }
            return new VariableExpression(access);
        }

        Expression* operator()(const ast_binary_expression& astExp) const {
            const auto& astExp1 = astExp.operand1;
            const auto& astOp   = astExp.op;
            const auto& astExp2 = astExp.operand2;

            unsigned op = 0U;
            if (astOp == "+") {
                op = BinaryExpression::Add;
            } else if (astOp == "-") {
                op = BinaryExpression::Subtract;
            } else if (astOp == "^") {
                op = BinaryExpression::Exor;
            } else if (astOp == "*") {
                op = BinaryExpression::Multiply;
            } else if (astOp == "/") {
                op = BinaryExpression::Divide;
            } else if (astOp == "%") {
                op = BinaryExpression::Modulo;
            } else if (astOp == "*>") {
                op = BinaryExpression::FracDivide;
            } else if (astOp == "&") {
                op = BinaryExpression::BitwiseAnd;
            } else if (astOp == "|") {
                op = BinaryExpression::BitwiseOr;
            } else if (astOp == "&&") {
                op = BinaryExpression::LogicalAnd;
            } else if (astOp == "||") {
                op = BinaryExpression::LogicalOr;
            } else if (astOp == "<") {
                op = BinaryExpression::LessThan;
            } else if (astOp == ">") {
                op = BinaryExpression::GreaterThan;
            } else if (astOp == "=") {
                op = BinaryExpression::Equals;
            } else if (astOp == "!=") {
                op = BinaryExpression::NotEquals;
            } else if (astOp == "<=") {
                op = BinaryExpression::LessEquals;
            } else if (astOp == ">=") {
                op = BinaryExpression::GreaterEquals;
            }

            const auto& lhs = parseExpression(astExp1, proc, 0U, context);
            if (!lhs) {
                return nullptr;
            }

            const auto& rhs = parseExpression(astExp2, proc, lhs->bitwidth(), context);
            if (!rhs) {
                return nullptr;
            }

            return new BinaryExpression(lhs, op, rhs);
        }

        Expression* operator()(const ast_shift_expression& astExp) const {
            const auto& astExp1 = astExp.operand1;
            const auto& astOp   = astExp.op;
            const auto& astNum  = astExp.operand2;

            unsigned op = 0U;
            if (astOp == "<<") {
                op = ShiftExpression::Left;
            } else if (astOp == ">>") {
                op = ShiftExpression::Right;
            }

            const auto& lhs = parseExpression(astExp1, proc, bitwidth, context);
            if (!lhs) {
                return nullptr;
            }

            const auto& rhs = parseNumber(astNum, proc, context);
            if (!rhs) {
                return nullptr;
            }

            if (auto const* lhsNo = dynamic_cast<NumericExpression*>(lhs.get())) {
                if (lhsNo->value->isConstant() && rhs->isConstant()) {
                    const auto& value   = lhsNo->value->evaluate(Number::loop_variable_mapping());
                    const auto& shftAmt = rhs->evaluate(Number::loop_variable_mapping());
                    unsigned    result  = 0;

                    switch (op) {
                        case ShiftExpression::Left: // <<
                        {
                            result = value << shftAmt;
                        } break;

                        case ShiftExpression::Right: // >>
                        {
                            result = value >> shftAmt;
                        } break;

                        default:
                            std::cerr << "Invalid operator in shift expression\n";
                            assert(false);
                    }
                    return new NumericExpression(std::make_shared<Number>(result), lhs->bitwidth());
                }
            }

            return new ShiftExpression(lhs, op, rhs);
        }

    private:
        const Module&  proc; // NOLINT(*-avoid-const-or-ref-data-members)
        unsigned       bitwidth;
        ParserContext& context; // NOLINT(*-avoid-const-or-ref-data-members)
    };

    Expression::ptr parseExpression(const ast_expression& astExp, const Module& proc, unsigned bitwidth, ParserContext& context) {
        return Expression::ptr(boost::apply_visitor(ExpressionVisitor(proc, bitwidth, context), astExp));
    }

    struct StatementVisitor {
        StatementVisitor(const Program& prog, const Module& proc, ParserContext& context):
            prog(prog),
            proc(proc),
            context(context) {}

        Statement::ptr operator()(const ast_swap_statement& astSwapStat) const {
            const auto& astVar1 = boost::fusion::at_c<0>(astSwapStat);
            const auto& astVar2 = boost::fusion::at_c<1>(astSwapStat);

            const auto& va1 = parseVariableAccess(astVar1, proc, context);
            if (!va1) {
                return nullptr;
            }
            const auto& va2 = parseVariableAccess(astVar2, proc, context);
            if (!va2) {
                return nullptr;
            }

            if (va1->bitwidth() != va2->bitwidth()) {
                std::cerr << "Different bit-widths in ↔ statement: " + va1->getVar()->name + " (" + std::to_string(va1->bitwidth()) + "), " + va2->getVar()->name + " (" + std::to_string(va2->bitwidth()) + ")\n";
                assert(false);
                return nullptr;
            }

            return std::make_shared<SwapStatement>(va1, va2);
        }

        Statement::ptr operator()(const ast_unary_statement& astUnaryStat) const {
            const auto& astOp  = boost::fusion::at_c<0>(astUnaryStat);
            const auto& astVar = boost::fusion::at_c<1>(astUnaryStat);

            const auto& var = parseVariableAccess(astVar, proc, context);
            if (!var) {
                return nullptr;
            }

            unsigned op = 0U;

            if (astOp == "~") {
                op = UnaryStatement::Invert;
            } else if (astOp == "++") {
                op = UnaryStatement::Increment;
            } else if (astOp == "--") {
                op = UnaryStatement::Decrement;
            }

            return std::make_shared<UnaryStatement>(op, var);
        }

        Statement::ptr operator()(const ast_assign_statement& astAssignStat) const {
            const auto& astVar = boost::fusion::at_c<0>(astAssignStat);
            const auto& astOp  = boost::fusion::at_c<1>(astAssignStat);
            const auto& astExp = boost::fusion::at_c<2>(astAssignStat);

            const auto& lhs = parseVariableAccess(astVar, proc, context);
            if (!lhs) {
                return nullptr;
            }

            unsigned op{};
            if (astOp == '+') {
                op = AssignStatement::Add;
            } else if (astOp == '-') {
                op = AssignStatement::Subtract;
            } else {
                op = AssignStatement::Exor;
            }

            const auto& rhs = parseExpression(astExp, proc, lhs->bitwidth(), context);
            if (!rhs) {
                return nullptr;
            }

            if (lhs->bitwidth() != rhs->bitwidth()) {
                context.errorMessage = "Wrong bit-width in assignment to " + lhs->getVar()->name;
                return nullptr;
            }

            return std::make_shared<AssignStatement>(lhs, op, rhs);
        }

        Statement::ptr operator()(const ast_if_statement& astIfStat) const {
            auto ifStat = std::make_shared<IfStatement>();

            const auto& condition = parseExpression(astIfStat.condition, proc, 1U, context);
            if (!condition) {
                return nullptr;
            }
            ifStat->setCondition(condition);

            const auto& fiCondition = parseExpression(astIfStat.fiCondition, proc, 1U, context);
            if (!fiCondition) {
                return nullptr;
            }
            ifStat->setFiCondition(fiCondition);

            for (const ast_statement& astStat: astIfStat.ifStatement) {
                const auto& stat = parseStatement(astStat, prog, proc, context);
                if (!stat) {
                    return nullptr;
                }
                ifStat->addThenStatement(stat);
            }

            for (const ast_statement& astStat: astIfStat.elseStatement) {
                const auto& stat = parseStatement(astStat, prog, proc, context);
                if (!stat) {
                    return nullptr;
                }
                ifStat->addElseStatement(stat);
            }

            return ifStat;
        }

        Statement::ptr operator()(const ast_for_statement& astForStat) const {
            auto forStat = std::make_shared<ForStatement>();

            Number::ptr from;
            const auto& to = parseNumber(astForStat.to, proc, context);
            if (!to) {
                return nullptr;
            }

            std::string loopVariable;
            if (astForStat.from) {
                from = parseNumber(boost::fusion::at_c<1>(*astForStat.from), proc, context);
                if (!from) {
                    return nullptr;
                }

                // is there a loop variable?
                if (boost::fusion::at_c<0>(*astForStat.from)) {
                    loopVariable = *boost::fusion::at_c<0>(*astForStat.from);

                    // does the loop variable exist already?
                    if (std::find(context.loopVariables.begin(), context.loopVariables.end(), loopVariable) != context.loopVariables.end()) {
                        context.errorMessage = "Redefinition of loop variable $" + loopVariable;
                        return nullptr;
                    }

                    forStat->loopVariable = loopVariable;

                    context.loopVariables.emplace_back(loopVariable);
                }
            }

            forStat->range = std::make_pair(from, to);

            // step
            for (const ast_statement& astStat: astForStat.doStatement) {
                const auto& stat = parseStatement(astStat, prog, proc, context);
                if (!stat) {
                    return nullptr;
                }
                forStat->addStatement(stat);
            }

            if (!loopVariable.empty()) {
                // release loop variable
                context.loopVariables.erase(std::remove_if(context.loopVariables.begin(), context.loopVariables.end(), [&loopVariable](const auto& s) { return s == loopVariable; }), context.loopVariables.end());
            }

            return forStat;
        }

        Statement::ptr operator()(const ast_call_statement& astCallStat) const {
            const auto& procName  = boost::fusion::at_c<1>(astCallStat);
            const auto& otherProc = prog.findModule(procName);

            // found no module
            if (!static_cast<bool>(otherProc.get())) {
                context.errorMessage = "Unknown module " + procName;
                return nullptr;
            }

            const std::vector<std::string>& parameters = boost::fusion::at_c<2>(astCallStat);

            // wrong number of parameters
            if (parameters.size() != otherProc->parameters.size()) {
                context.errorMessage = "Wrong number of arguments in (un)call of " + otherProc->name + ". Expected " + std::to_string(otherProc->parameters.size()) + ", got " + std::to_string(parameters.size());
                return nullptr;
            }

            // unknown variable name in parameters
            for (const std::string& parameter: parameters) {
                if (!proc.findParameterOrVariable(parameter)) {
                    context.errorMessage = "Unknown variable " + parameter + " in (un)call of " + otherProc->name;
                    return nullptr;
                }
            }

            // check whether bit-width fits
            for (unsigned i = 0; i < parameters.size(); ++i) {
                const auto& vOther    = otherProc->parameters.at(i);
                const auto& parameter = proc.findParameterOrVariable(parameters.at(i)); // must exist (see above)

                if (vOther->bitwidth != parameter->bitwidth) {
                    context.errorMessage = std::to_string(i + 1) + ". parameter (" + parameters.at(i) + ") in (un)call of " + otherProc->name + " has bit-width of " + std::to_string(parameter->bitwidth) + ", but " + std::to_string(vOther->bitwidth) + " is required";
                    return nullptr;
                }
            }

            if (boost::fusion::at_c<0>(astCallStat) == "call") {
                return std::make_shared<CallStatement>(otherProc, parameters);
            }
            return std::make_shared<UncallStatement>(otherProc, parameters);
        }

        Statement::ptr operator()(const std::string& astSkipStat [[maybe_unused]]) const {
            return std::make_shared<SkipStatement>();
        }

    private:
        const Program& prog;    // NOLINT(*-avoid-const-or-ref-data-members)
        const Module&  proc;    // NOLINT(*-avoid-const-or-ref-data-members)
        ParserContext& context; // NOLINT(*-avoid-const-or-ref-data-members)
    };

    Statement::ptr parseStatement(const ast_statement& astStat, const Program& prog, const Module& proc, ParserContext& context) {
        if (auto stat = boost::apply_visitor(StatementVisitor(prog, proc, context), boost::fusion::at_c<1>(astStat))) {
            context.currentLineNumber = static_cast<unsigned>(std::count(context.begin, boost::fusion::at_c<0>(astStat), '\n')) + 1U;
            stat->lineNumber          = context.currentLineNumber;
            return {stat};
        }
        return {};
    }

    unsigned parseVariableType(const std::string& name) {
        if (name == "in") {
            return Variable::In;
        }
        if (name == "out") {
            return Variable::Out;
        }
        if (name == "inout") {
            return Variable::Inout;
        }
        if (name == "state") {
            return Variable::State;
        }
        if (name == "wire") {
            return Variable::Wire;
        }

        assert(false);
        return 0U;
    }

    bool parseModule(Module& proc, const ast_module& astProc, const Program& prog, ParserContext& context) {
        std::set<std::string> variableNames;

        for (const ast_parameter& astParam: boost::fusion::at_c<1>(astProc)) {
            const std::string& variableName = boost::fusion::at_c<0>(boost::fusion::at_c<1>(astParam));

            if (variableNames.find(variableName) != variableNames.end()) {
                context.errorMessage = "Redefinition of variable " + variableName;
                return false;
            }
            variableNames.emplace(variableName);

            const auto& type = parseVariableType(boost::fusion::at_c<0>(astParam));
            proc.addParameter(std::make_shared<Variable>(
                    type,
                    variableName,
                    boost::fusion::at_c<1>(boost::fusion::at_c<1>(astParam)),
                    boost::fusion::at_c<2>(boost::fusion::at_c<1>(astParam)).get_value_or(context.settings.defaultBitwidth)));
        }

        for (const ast_statement& astStat: boost::fusion::at_c<3>(astProc)) {
            const auto& stat = parseStatement(astStat, prog, proc, context);
            if (!stat) {
                return false;
            }
            proc.addStatement(stat);
        }

        return true;
    }

    bool Program::readProgramFromString(const std::string& content, const ReadProgramSettings& settings, std::string& error) {
        ast_program astProg;
        if (!parseString(astProg, content)) {
            error = "PARSE_STRING_FAILED";
            return false;
        }

        ParserContext context(settings);
        context.begin = content.begin();

        // Modules
        for (const ast_module& astProc: astProg) {
            const auto& proc(std::make_shared<Module>(boost::fusion::at_c<0>(astProc)));
            if (!parseModule(*proc, astProc, *this, context)) {
                error = "In line " + std::to_string(context.currentLineNumber) + ": " + context.errorMessage;
                return false;
            }
            addModule(proc);
        }

        return true;
    }
} // namespace syrec
