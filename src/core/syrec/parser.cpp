#include "core/syrec/parser.hpp"

#include <fstream>
#include <memory>
#include <optional>

namespace syrec {

    struct ParseNumberVisitor {
        explicit ParseNumberVisitor(const Module& proc, ParserContext& context):
            proc(proc),
            context(context) {}

        Number::ptr operator()(unsigned value) const {
            return std::make_shared<Number>(value);
        }

        Number::ptr operator()(const boost::recursive_wrapper<ast_variable>& astVar) const {
            Variable::ptr var = proc.findParameterOrVariable(astVar.get().name);
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
            ast_number  astNo1 = astNe.get().operand1;
            std::string astOp  = astNe.get().op;
            ast_number  astNo2 = astNe.get().operand2;

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

            Number::ptr lhs = parseNumber(astNo1, proc, context);
            if (!lhs) {
                return {};
            }

            Number::ptr rhs = parseNumber(astNo2, proc, context);
            if (!rhs) {
                return {};
            }
            unsigned num = 0;
            if (lhs->isConstant() && rhs->isConstant()) {
                unsigned lhsValue = lhs->evaluate(Number::loop_variable_mapping());
                unsigned rhsValue = rhs->evaluate(Number::loop_variable_mapping());
                unsigned numValue = 0;

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
            return std::make_shared<Number>(num);
        }

    private:
        const Module&  proc;
        ParserContext& context;
    };

    Number::ptr parseNumber(const ast_number& astNum, const Module& proc, ParserContext& context) {
        return std::visit(ParseNumberVisitor(proc, context), astNum);
    }

    VariableAccess::ptr parseVariableAccess(const ast_variable& astVar, const Module& proc, ParserContext& context) {
        Variable::ptr var = proc.findParameterOrVariable(astVar.name);

        if (!var) {
            context.errorMessage = "Unknown variable %s" + astVar.name;
            return {};
        }

        VariableAccess::ptr va(new VariableAccess());
        va->setVar(var);

        std::optional<std::pair<Number::ptr, Number::ptr>> varRange;

        ast_range range = astVar.range;
        if (range) {
            Number::ptr first = parseNumber(boost::fusion::at_c<0>(*range), proc, context);
            if (!first) {
                return {};
            }

            // is in range?
            if (!first->isLoopVariable()) {
                unsigned bound = first->evaluate(Number::loop_variable_mapping());
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
                    unsigned bound = second->evaluate(Number::loop_variable_mapping());
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

        expression::vec indexes;
        for (const ast_expression& astExp: astVar.indexes) {
            expression::ptr index = parseExpression(astExp, proc, var->bitwidth, context);
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

        expression* operator()(const ast_number& astNum) const {
            Number::ptr num = parseNumber(astNum, proc, context);
            if (!num) {
                return nullptr;
            }
            return new NumericExpression(num, bitwidth);
        }

        expression* operator()(const ast_variable& astVar) const {
            VariableAccess::ptr access = parseVariableAccess(astVar, proc, context);
            if (!access) {
                return nullptr;
            }
            return new VariableExpression(access);
        }

        expression* operator()(const ast_binary_expression& astExp) const {
            ast_expression astExp1 = astExp.operand1;
            std::string    astOp   = astExp.op;
            ast_expression astExp2 = astExp.operand2;

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

            expression::ptr lhs = parseExpression(astExp1, proc, 0U, context);
            if (!lhs) {
                return nullptr;
            }

            expression::ptr rhs = parseExpression(astExp2, proc, lhs->bitwidth(), context);
            if (!rhs) {
                return nullptr;
            }

            return new BinaryExpression(lhs, op, rhs);
        }

        expression* operator()(const ast_shift_expression& astExp) const {
            ast_expression astExp1 = astExp.operand1;
            std::string    astOp   = astExp.op;
            ast_number     astNum  = astExp.operand2;

            unsigned op = 0U;
            if (astOp == "<<") {
                op = ShiftExpression::Left;
            } else if (astOp == ">>") {
                op = ShiftExpression::Right;
            }

            expression::ptr lhs = parseExpression(astExp1, proc, bitwidth, context);
            if (!lhs) {
                return nullptr;
            }

            Number::ptr rhs = parseNumber(astNum, proc, context);
            if (!rhs) {
                return nullptr;
            }

            if (auto const* lhsNo = dynamic_cast<NumericExpression*>(lhs.get())) {
                if (lhsNo->value->isConstant() && rhs->isConstant()) {
                    unsigned value   = lhsNo->value->evaluate(Number::loop_variable_mapping());
                    unsigned shftAmt = rhs->evaluate(Number::loop_variable_mapping());
                    unsigned result  = 0;

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
                            std::cerr << "Invalid operator in shift expression" << std::endl;
                            assert(false);
                    }
                    return new NumericExpression(std::make_shared<Number>(result), lhs->bitwidth());
                }
            }

            return new ShiftExpression(lhs, op, rhs);
        }

    private:
        const Module&  proc;
        unsigned       bitwidth;
        ParserContext& context;
    };

    expression::ptr parseExpression(const ast_expression& astExp, const Module& proc, unsigned bitwidth, ParserContext& context) {
        return expression::ptr(boost::apply_visitor(ExpressionVisitor(proc, bitwidth, context), astExp));
    }

    struct StatementVisitor {
        StatementVisitor(const program& prog, const Module& proc, ParserContext& context):
            prog(prog),
            proc(proc),
            context(context) {}

        Statement::ptr operator()(const ast_swap_statement& astSwapStat) const {
            const ast_variable& astVar1 = boost::fusion::at_c<0>(astSwapStat);
            const ast_variable& astVar2 = boost::fusion::at_c<1>(astSwapStat);

            VariableAccess::ptr va1 = parseVariableAccess(astVar1, proc, context);
            if (!va1) {
                return nullptr;
            }
            VariableAccess::ptr va2 = parseVariableAccess(astVar2, proc, context);
            if (!va2) {
                return nullptr;
            }

            if (va1->bitwidth() != va2->bitwidth()) {
                std::cerr << "Different bit-widths in <=> statement: " + va1->getVar()->name + " (" + std::to_string(va1->bitwidth()) + "), " + va2->getVar()->name + " (" + std::to_string(va2->bitwidth()) + ")" << std::endl;
                assert(false);
                return nullptr;
            }

            return std::make_shared<SwapStatement>(va1, va2);
        }

        Statement::ptr operator()(const ast_unary_statement& astUnaryStat) const {
            const std::string&  astOp  = boost::fusion::at_c<0>(astUnaryStat);
            const ast_variable& astVar = boost::fusion::at_c<1>(astUnaryStat);

            VariableAccess::ptr var = parseVariableAccess(astVar, proc, context);
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
            const ast_variable&   astVar = boost::fusion::at_c<0>(astAssignStat);
            char                  astOp  = boost::fusion::at_c<1>(astAssignStat);
            const ast_expression& astExp = boost::fusion::at_c<2>(astAssignStat);

            VariableAccess::ptr lhs = parseVariableAccess(astVar, proc, context);
            if (!lhs) {
                return nullptr;
            }

            unsigned op = astOp == '+' ? AssignStatement::Add :
                                         (astOp == '-' ? AssignStatement::Subtract : AssignStatement::Exor);

            expression::ptr rhs = parseExpression(astExp, proc, lhs->bitwidth(), context);
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

            expression::ptr condition = parseExpression(astIfStat.condition, proc, 1U, context);
            if (!condition) {
                return nullptr;
            }
            ifStat->setCondition(condition);

            expression::ptr fiCondition = parseExpression(astIfStat.fiCondition, proc, 1U, context);
            if (!fiCondition) {
                return nullptr;
            }
            ifStat->setFiCondition(fiCondition);

            for (const ast_statement& astStat: astIfStat.ifStatement) {
                Statement::ptr stat = parseStatement(astStat, prog, proc, context);
                if (!stat) {
                    return nullptr;
                }
                ifStat->addThenStatement(stat);
            }

            for (const ast_statement& astStat: astIfStat.elseStatement) {
                Statement::ptr stat = parseStatement(astStat, prog, proc, context);
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
            Number::ptr to = parseNumber(astForStat.to, proc, context);
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
                Statement::ptr stat = parseStatement(astStat, prog, proc, context);
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
            std::string procName  = boost::fusion::at_c<1>(astCallStat);
            Module::ptr otherProc = prog.findModule(procName);

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
                Variable::ptr vOther    = otherProc->parameters.at(i);
                Variable::ptr parameter = proc.findParameterOrVariable(parameters.at(i)); // must exist (see above)

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
        const program& prog;
        const Module&  proc;
        ParserContext& context;
    };

    Statement::ptr parseStatement(const ast_statement& astStat, const program& prog, const Module& proc, ParserContext& context) {
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

    bool parseModule(Module& proc, const ast_module& astProc, const program& prog, ParserContext& context) {
        std::set<std::string> variableNames;

        for (const ast_parameter& astParam: boost::fusion::at_c<1>(astProc)) {
            const std::string& variableName = boost::fusion::at_c<0>(boost::fusion::at_c<1>(astParam));

            if (variableNames.find(variableName) != variableNames.end()) {
                context.errorMessage = "Redefinition of variable " + variableName;
                return false;
            }
            variableNames.emplace(variableName);

            unsigned type = parseVariableType(boost::fusion::at_c<0>(astParam));
            proc.addParameter(std::make_shared<Variable>(
                    type,
                    variableName,
                    boost::fusion::at_c<1>(boost::fusion::at_c<1>(astParam)),
                    boost::fusion::at_c<2>(boost::fusion::at_c<1>(astParam)).get_value_or(context.settings.defaultBitwidth)));
        }

        for (const ast_statement& astStat: boost::fusion::at_c<3>(astProc)) {
            Statement::ptr stat = parseStatement(astStat, prog, proc, context);
            if (!stat) {
                return false;
            }
            proc.addStatement(stat);
        }

        return true;
    }

    bool program::readProgramFromString(const std::string& content, const ReadProgramSettings& settings, std::string* error) {
        ast_program astProg;
        if (!parseString(astProg, content)) {
            *error = "PARSE_STRING_FAILED";
            return false;
        }

        ParserContext context(settings);
        context.begin = content.begin();

        // Modules
        for (const ast_module& astProc: astProg) {
            Module::ptr proc(std::make_shared<Module>(boost::fusion::at_c<0>(astProc)));
            if (!parseModule(*proc, astProc, *this, context)) {
                if (error != nullptr) {
                    *error = "In line " + std::to_string(context.currentLineNumber) + ": " + context.errorMessage;
                }
                return false;
            }
            addModule(proc);
        }

        return true;
    }
} // namespace syrec
