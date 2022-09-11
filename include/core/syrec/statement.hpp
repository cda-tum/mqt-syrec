#pragma once

#include "core/syrec/expression.hpp"
#include "core/syrec/module.hpp"
#include "core/syrec/variable.hpp"

#include <iostream>
#include <memory>
#include <utility>
#include <vector>

namespace syrec {

    struct Module;

    /**
     * @brief Abstract base class for all SyReC statements
     *
     * All statement classes are derived from this abstract class.
     * Each class has to implement the print() method. Otherwise,
     * the different classes are solely used for distinction.
     */
    struct Statement {
        /**
       * @brief Smart pointer
       */
        using ptr = std::shared_ptr<Statement>;

        /**
       * @brief Vector of smart pointers
       */
        using vec = std::vector<ptr>;

        /**
       * @brief Standard constructor
       *
       * Initializes default values
       */
        Statement() = default;

        /**
       * @brief Deconstructor
       */
        virtual ~Statement() = default;

        unsigned lineNumber{};

        virtual Statement::ptr reverse() {
            return std::make_shared<Statement>(*this);
        };
    };
    using skip_statement = Statement;

    /**
     * @brief SWAP Statement
     *
     * This class represents the SyReC SWAP Statement (<=>)
     * between two variables lhs() and rhs().
     */
    struct SwapStatement: public Statement {
        /**
       * @brief Constructor
       *
       * @param lhs Variable access on left hand side
       * @param rhs Variable access on right hand side
       */
        SwapStatement(VariableAccess::ptr lhs,
                      VariableAccess::ptr rhs):
            lhs(std::move(lhs)),
            rhs(std::move(rhs)) {}

        VariableAccess::ptr lhs;
        VariableAccess::ptr rhs;
    };

    /**
     * @brief Unary Statement
     *
     * This class represents the SyReC Unary statements (++, --, ~)
     * on the variable access var().
     */
    struct UnaryStatement: public Statement {
        /**
       * @brief Type of the statement
       */
        enum {
            /**
         * @brief Inversion of the variable
         */
            Invert,

            /**
         * @brief Increment of the variable by 1
         */
            Increment,

            /**
         * @brief Decrement of the variable by 1
         */
            Decrement
        };

        /**
       * @brief Constructor
       *
       * @param op Operation
       * @param var Variable access to be transformed
       */
        UnaryStatement(unsigned            op,
                       VariableAccess::ptr var):
            op(op),
            var(std::move(var)) {}

        Statement::ptr reverse() override {
            switch (op) {
                case UnaryStatement::Increment:
                    return std::make_shared<UnaryStatement>(Decrement, var);

                case UnaryStatement::Decrement:
                    return std::make_shared<UnaryStatement>(Increment, var);

                case UnaryStatement::Invert:
                default:
                    return std::make_shared<UnaryStatement>(*this);
            }
        }

        unsigned            op{};
        VariableAccess::ptr var{};
    };

    /**
     * @brief Assignment Statement
     *
     * This class represents the SyReC assignment statements (+=, -=, ^=)
     * of the expression rhs() to the variable access lhs().
     */
    struct AssignStatement: public Statement {
        /**
       * @brief Type of assignment
       */
        enum {
            /**
         * @brief Addition to itself
         */
            Add,

            /**
         * @brief Subtraction from itself
         */
            Subtract,

            /**
         * @brief Reflexive EXOR operation
         */
            Exor
        };

        /**
       * @brief Constructor
       *
       * @param lhs Variable access to which the operation is applied
       * @param op Operation to be applied
       * @param rhs Expression to be evaluated
       */
        AssignStatement(VariableAccess::ptr lhs,
                        unsigned            op,
                        expression::ptr     rhs):
            lhs(std::move(lhs)),
            op(op), rhs(std::move(rhs)) {}

        Statement::ptr reverse() override {
            switch (op) {
                case AssignStatement::Add:
                    return std::make_shared<AssignStatement>(lhs, Subtract, rhs);

                case AssignStatement::Subtract:
                    return std::make_shared<AssignStatement>(lhs, Add, rhs);

                case AssignStatement::Exor:
                default:
                    return std::make_shared<AssignStatement>(*this);
            }
        }

        VariableAccess::ptr lhs{};
        unsigned            op{};
        expression::ptr     rhs{};
    };

    /**
     * @brief IF Statement
     *
     * This class represents the SyReC \b if statement
     */
    struct IfStatement: public Statement {
        /**
       * @brief Standard constructor
       *
       * Initializes default values
       */
        IfStatement() = default;

        /**
       * @brief Sets the condition for the execution of the then_statements()
       *
       * The expression \p condition is assumed to have a bit-width of 1 bit.
       *
       * @param condition Expression
       */
        void setCondition(expression::ptr cond) {
            condition = std::move(cond);
        }

        /**
       * @brief Adds a statement to the then branch
       *
       * @param then_statement Statement to be executed in the if branch
       */
        void addThenStatement(const Statement::ptr& thenStatement) {
            thenStatements.emplace_back(thenStatement);
        }

        /**
       * @brief Adds a statement to the else branch
       *
       * @param else_statement Statement to be executed in the else branch
       */
        void addElseStatement(const Statement::ptr& elseStatement) {
            elseStatements.emplace_back(elseStatement);
        }

        /**
       * @brief Sets the reverse condition for the execution of the if_statements()
       *
       * The expression \p fi_condition is assumed to have a bit-width of 1 bit.
       * The reverse condition is checked in order the if statement is uncalled,
       * i.e. executed reversed. Usually it is the same has the condition(), unless
       * the evaluation of the condition does not change in one of the branches.
       *
       * @param fi_condition Expression
       */
        void setFiCondition(expression::ptr fiCond) {
            fiCondition = std::move(fiCond);
        }

        Statement::ptr reverse() override {
            auto fi = std::make_shared<IfStatement>();
            fi->setFiCondition(condition);
            fi->setCondition(fiCondition);
            for (auto it = thenStatements.rbegin(); it != thenStatements.rend(); ++it) {
                fi->addThenStatement(*it);
            }
            for (auto it = elseStatements.rbegin(); it != elseStatements.rend(); ++it) {
                fi->addElseStatement(*it);
            }
            return fi;
        }

        expression::ptr condition{};
        Statement::vec  thenStatements{};
        Statement::vec  elseStatements{};
        expression::ptr fiCondition{};
    };

    /**
     * @brief FOR Statement
     *
     * This class represents the SyReC \b for statement
     */
    struct ForStatement: public Statement {
        /**
       * @brief Standard constructor
       *
       * Initializes default values
       */
        ForStatement() = default;

        /**
       * @brief Adds a statement to be executed in the loop
       *
       * @param statement Statement
       */
        void addStatement(const Statement::ptr& statement) {
            statements.emplace_back(statement);
        }

        Statement::ptr reverse() override {
            auto forStat          = std::make_shared<ForStatement>();
            forStat->loopVariable = loopVariable;
            forStat->range        = std::make_pair(range.second, range.first);
            for (auto it = statements.rbegin(); it != statements.rend(); ++it) {
                forStat->addStatement(*it);
            }
            return forStat;
        }

        std::string                         loopVariable{};
        std::pair<Number::ptr, Number::ptr> range{};
        Number::ptr                         step{};
        Statement::vec                      statements{};
    };

    struct uncall_statement;

    /**
     * @brief CALL Statement
     *
     * This class represents the SyReC \b call statement to call a module.
     */
    struct CallStatement: public Statement {
        /**
       * @brief Constructor with module and parameters
       *
       * @param target Module to call
       * @param parameters Parameters to assign
       */
        CallStatement(std::shared_ptr<Module> target, std::vector<std::string> parameters):
            target(std::move(target)), parameters(std::move(parameters)) {}

        Statement::ptr reverse() override;

        std::shared_ptr<Module>  target{};
        std::vector<std::string> parameters{};
    };

    /**
     * @brief UNCALL Statement
     *
     * This class represents the SyReC \b uncall statement to uncall a module.
     */
    struct uncall_statement: public Statement {
        /**
       * @brief Constructor with module and parameters
       *
       * @param target Module to uncall
       * @param parameters Parameters to assign
       */
        uncall_statement(std::shared_ptr<Module> target, std::vector<std::string> parameters):
            target(std::move(target)), parameters(std::move(parameters)) {}

        Statement::ptr reverse() override {
            return std::make_shared<CallStatement>(target, parameters);
        }

        std::shared_ptr<Module>  target{};
        std::vector<std::string> parameters{};
    };

    inline Statement::ptr CallStatement::reverse() {
        return std::make_shared<uncall_statement>(target, parameters);
    }

} // namespace syrec
