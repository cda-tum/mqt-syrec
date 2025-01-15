#ifndef CORE_SYREC_PARSER_COMPONENTS_CUSTOM_STATEMENT_VISITOR_HPP
#define CORE_SYREC_PARSER_COMPONENTS_CUSTOM_STATEMENT_VISITOR_HPP
#pragma once

#include "core/syrec/variable.hpp"

#include <core/syrec/statement.hpp>
#include <core/syrec/parser/components/custom_expression_visitor.hpp>

namespace syrecParser {
    class CustomStatementVisitor: protected CustomBaseVisitor {
    public:
        struct NotOverloadResolutedCallStatementScope {
            using CallStatementInstanceVariant = std::variant<std::shared_ptr<syrec::CallStatement>, std::shared_ptr<syrec::UncallStatement>>;

            struct DeclaredModuleSignature {
                std::string          moduleIdentifier;
                syrec::Variable::vec parameters;

                explicit DeclaredModuleSignature(std::string moduleIdentifier, syrec::Variable::vec parameters):
                    moduleIdentifier(std::move(moduleIdentifier)), parameters(std::move(parameters)) {}
            };

            struct CallStatementData {
                CallStatementInstanceVariant callStatementVariantInstance;
                std::string                  calledModuleIdentifier;
                syrec::Variable::vec         symbolTableEntriesForCallerArguments;
                std::size_t                  linePositionOfModuleIdentifier;
                std::size_t                  columnPositionOfModuleIdentifier;

                explicit CallStatementData(CallStatementInstanceVariant callStatementVariantInstance, std::string calledModuleIdentifier, syrec::Variable::vec symbolTableEntriesForCallerArguments, std::size_t linePositionOfModuleIdentifier, std::size_t columnPositionOfModuleIdentifier):
                    callStatementVariantInstance(std::move(callStatementVariantInstance)),
                    calledModuleIdentifier(std::move(calledModuleIdentifier)), symbolTableEntriesForCallerArguments(std::move(symbolTableEntriesForCallerArguments)),
                    linePositionOfModuleIdentifier(linePositionOfModuleIdentifier), columnPositionOfModuleIdentifier(columnPositionOfModuleIdentifier) {}
            };

            DeclaredModuleSignature        signatureOfModuleContainingCallStatement;
            std::vector<CallStatementData> callStatementsToPerformOverloadResolutionOn;

            explicit NotOverloadResolutedCallStatementScope(DeclaredModuleSignature signatureOfModuleContainingCallStatement):
                signatureOfModuleContainingCallStatement(std::move(signatureOfModuleContainingCallStatement)) {}
        };

        CustomStatementVisitor(const std::shared_ptr<ParserMessagesContainer>& sharedMessagesContainerInstance):
            CustomBaseVisitor(sharedMessagesContainerInstance),
            expressionVisitorInstance(std::make_unique<CustomExpressionVisitor>(sharedMessagesContainerInstance)) {}
 
        [[nodiscard]] std::optional<syrec::Statement::vec>        visitStatementListTyped(const TSyrecParser::StatementListContext* ctx);
        [[nodiscard]] std::optional<syrec::Statement::ptr>        visitStatementTyped(TSyrecParser::StatementContext* ctx);
        [[nodiscard]] std::optional<syrec::Statement::ptr>        visitCallStatementTyped(TSyrecParser::CallStatementContext* ctx);
        [[nodiscard]] std::optional<syrec::Statement::ptr>        visitForStatementTyped(TSyrecParser::ForStatementContext* ctx);
        [[nodiscard]] std::optional<syrec::Statement::ptr>        visitIfStatementTyped(const TSyrecParser::IfStatementContext* ctx);
        [[nodiscard]] std::optional<syrec::Statement::ptr>        visitUnaryStatementTyped(TSyrecParser::UnaryStatementContext* ctx) const;
        [[nodiscard]] std::optional<syrec::Statement::ptr>        visitAssignStatementTyped(TSyrecParser::AssignStatementContext* ctx) const;
        [[nodiscard]] std::optional<syrec::Statement::ptr>        visitSwapStatementTyped(const TSyrecParser::SwapStatementContext* ctx) const;
        [[nodiscard]] static std::optional<syrec::Statement::ptr> visitSkipStatementTyped(TSyrecParser::SkipStatementContext* ctx);

        [[nodiscard]] std::vector<NotOverloadResolutedCallStatementScope> getCallStatementsWithNotPerformedOverloadResolution() const;
        void                                                              openNewScopeToRecordCallStatementsInModule(const NotOverloadResolutedCallStatementScope::DeclaredModuleSignature& enclosingModuleSignature);

    protected:
        std::unique_ptr<CustomExpressionVisitor>                   expressionVisitorInstance;
        std::vector<NotOverloadResolutedCallStatementScope>        callStatementsWithNotPerformedOverloadResolutionScopes;

        struct LoopStepsizeDefinition {
            bool               hasMinusPrefix;
            syrec::Number::ptr stepsize;
        };

        [[nodiscard]] std::optional<std::string>            visitLoopVariableDefinitionTyped(TSyrecParser::LoopVariableDefinitionContext* ctx) const;
        [[nodiscard]] std::optional<LoopStepsizeDefinition> visitLoopStepsizeDefinitionTyped(TSyrecParser::LoopStepsizeDefinitionContext* ctx) const;

        void                                                  recordErrorIfAssignmentToReadonlyVariableIsPerformed(const syrec::Variable& accessedVariable, const antlr4::Token& reportedErrorPosition) const;
        [[nodiscard]] NotOverloadResolutedCallStatementScope* getActiveModuleScopeRecordingCallStatements();

        [[nodiscard]] static std::optional<syrec::AssignStatement::AssignOperation> deserializeAssignmentOperationFromString(const std::string_view& stringifiedAssignmentOperation);
        [[nodiscard]] static std::optional<syrec::UnaryStatement::UnaryOperation>   deserializeUnaryAssignmentOperationFromString(const std::string_view& stringifiedUnaryAssignmentOperation);
        [[nodiscard]] static bool                                                   doesVariableTypeAllowAssignment(const syrec::Variable::Type variableType) noexcept {
            return variableType == syrec::Variable::Type::Inout || variableType == syrec::Variable::Type::Out || variableType == syrec::Variable::Type::Wire;
        }
    };
} // namespace TSyrecParser
#endif