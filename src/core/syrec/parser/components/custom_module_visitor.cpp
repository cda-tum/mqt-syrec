#include "core/syrec/parser/components/custom_module_visitor.hpp"

#include "core/syrec/module.hpp"
#include "core/syrec/parser/components/custom_statement_visitor.hpp"

using namespace syrecParser;

std::optional<std::shared_ptr<syrec::Program>> CustomModuleVisitor::parseProgram(TSyrecParser::ProgramContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Program>(context);
}

// START OF NON-PUBLIC FUNCTIONALITY
std::optional<std::shared_ptr<syrec::Program>> CustomModuleVisitor::visitProgramTyped(TSyrecParser::ProgramContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Program>(context);
}

std::optional<syrec::Module::ptr> CustomModuleVisitor::visitModuleTyped(TSyrecParser::ModuleContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Module>(context);
}

std::optional<std::vector<syrec::Variable::ptr>> CustomModuleVisitor::visitParameterListTyped(TSyrecParser::ParameterListContext* context) {
    return visitNonTerminalSymbolWithManyResults<syrec::Variable>(context);
}

std::optional<syrec::Variable::ptr> CustomModuleVisitor::visitParameterTyped(TSyrecParser::ParameterContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Variable>(context);
}

std::optional<std::vector<syrec::Variable::ptr>> CustomModuleVisitor::visitSignalListTyped(TSyrecParser::SignalListContext* context) {
    return visitNonTerminalSymbolWithManyResults<syrec::Variable>(context);
}

std::optional<syrec::Variable::ptr> CustomModuleVisitor::visitSignalDeclarationTyped(TSyrecParser::SignalDeclarationContext* context) {
    return visitNonTerminalSymbolWithSingleResult<syrec::Variable>(context);
}

std::optional<std::vector<syrec::Statement::ptr>> CustomModuleVisitor::visitStatementListTyped(TSyrecParser::StatementListContext* context) {
    return visitNonTerminalSymbolWithManyResults<syrec::Statement>(context);
}

bool CustomModuleVisitor::doVariablesMatch(const syrec::Variable& lVariable, const syrec::Variable& rVariable) {
    return lVariable.name == rVariable.name
        && std::equal(lVariable.dimensions.cbegin(), lVariable.dimensions.cend(), rVariable.dimensions.cbegin(), rVariable.dimensions.cend())
        && lVariable.bitwidth == rVariable.bitwidth;
}

bool CustomModuleVisitor::doVariableCollectionsMatch(const syrec::Variable::vec& lVariableCollection, const syrec::Variable::vec& rVariableCollection) {
    return std::equal(
            lVariableCollection.cbegin(),
            lVariableCollection.cend(),
            rVariableCollection.cbegin(),
            rVariableCollection.cend(),
            [](const syrec::Variable::ptr& lVariable, const syrec::Variable::ptr& rVariable) {
                return lVariable && rVariable && doVariablesMatch(*lVariable, *rVariable);
            });
}

std::any CustomModuleVisitor::visitProgram(TSyrecParser::ProgramContext* context) {
    if (!context)
        return std::nullopt;

    syrec::Module::vec parsedUserDefinedModules;
    parsedUserDefinedModules.reserve(context->module().size());

    auto generatedProgram = std::make_shared<syrec::Program>();
    for (const auto& antlrModuleContext: context->module())
        if (const std::optional<syrec::Module::ptr>& parsedModule = visitModuleTyped(antlrModuleContext); parsedModule.has_value())
            generatedProgram->addModule(*parsedModule);

    std::shared_ptr<const syrec::Module> definedMainModule;
    if (symbolTable->existsModuleForName("main"))
        definedMainModule = symbolTable->getModulesByName("main").front();
    else
        definedMainModule = parsedUserDefinedModules.back();

    // We are not requiring a C89 style def-before use for both call-/uncall statements, thus we need to perform overload resolution for any of these statements after the whole program was processed.
    const std::vector<CustomStatementVisitor::NotOverloadResolutedCallStatementScope> callStatementsScopeForWhichOverloadResolutionShouldBePerformed = statementVisitorInstance->getCallStatementsWithNotPerformedOverloadResolution();
    for (const auto& scope : callStatementsScopeForWhichOverloadResolutionShouldBePerformed) {
        for (const CustomStatementVisitor::NotOverloadResolutedCallStatementScope::CallStatementData& callStatementVariant : scope.callStatementsToPerformOverloadResolutionOn) {
            const utils::BaseSymbolTable::ModuleOverloadResolutionResult overloadResolutionResult = symbolTable->getModulesMatchingSignature(callStatementVariant.calledModuleIdentifier, callStatementVariant.symbolTableEntriesForCallerArguments);
            const auto                                                   semanticErrorPosition    = Message::Position(callStatementVariant.linePositionOfModuleIdentifier, callStatementVariant.columnPositionOfModuleIdentifier);

            if (definedMainModule->name == "main" && definedMainModule->name == callStatementVariant.calledModuleIdentifier) {
                recordSemanticError<SemanticError::CannotCallMainModule>(semanticErrorPosition);
                continue;
            }

            if (!symbolTable->existsModuleForName(callStatementVariant.calledModuleIdentifier)) {
                recordSemanticError<SemanticError::NoModuleMatchingIdentifier>(semanticErrorPosition, callStatementVariant.calledModuleIdentifier);
                continue;
            }

            if (overloadResolutionResult.resolutionResult == utils::BaseSymbolTable::ModuleOverloadResolutionResult::Result::MultipleMatchesFound)
                // TODO: Should we log the user provided parameter structure
                recordSemanticError<SemanticError::NoModuleMatchingCallSignature>(semanticErrorPosition);
            else if (overloadResolutionResult.resolutionResult == utils::BaseSymbolTable::ModuleOverloadResolutionResult::Result::SingleMatchFound) {
                if (doVariableCollectionsMatch(scope.signatureOfModuleContainingCallStatement.parameters, callStatementVariant.symbolTableEntriesForCallerArguments))
                    recordSemanticError<SemanticError::SelfRecursionNotAllowed>(semanticErrorPosition);
                // TODO: We should not have to test that the defined main module is defined but we for now leave this fail-safe check in.
                else if (definedMainModule && doVariableCollectionsMatch(definedMainModule->parameters, callStatementVariant.symbolTableEntriesForCallerArguments))
                    recordSemanticError<SemanticError::CannotCallMainModule>(semanticErrorPosition);
            }
        }
    }
    return generatedProgram;
}

// TODO: Add tests for this behaviour
// TODO: Cannot call module with identifier 'main' if such a module was defined (cannot be determined here)
// TODO: Cannot call main module last defined in syrec module (cannot be determined here)
// TODO: Cannot perform recursive call to itself
std::any CustomModuleVisitor::visitModule(TSyrecParser::ModuleContext* context) {
    if (!context)
        return std::nullopt;

    std::optional<std::string> moduleIdentifier;
    if (context->IDENT()) {
        moduleIdentifier = context->IDENT()->getText();
        if (moduleIdentifier == "main" && symbolTable->existsModuleForName(*moduleIdentifier))
            recordSemanticError<SemanticError::DuplicateMainModuleDefinition>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()));
    }

    auto generatedModule = std::make_shared<syrec::Module>(moduleIdentifier.value_or(""));
    symbolTable->openTemporaryScope();
    generatedModule->parameters = visitNonTerminalSymbolWithManyResults<syrec::Variable>(context->parameterList()).value_or(syrec::Variable::vec());

    for (const auto& antlrLocalVariableContexts: context->signalList())
        if (const std::optional<syrec::Variable::vec> localVariableDefinitions = visitNonTerminalSymbolWithManyResults<syrec::Variable>(antlrLocalVariableContexts); localVariableDefinitions.has_value())
            generatedModule->variables.insert(generatedModule->variables.end(), localVariableDefinitions->cbegin(), localVariableDefinitions->cend());

    statementVisitorInstance->openNewScopeToRecordCallStatementsInModule(CustomStatementVisitor::NotOverloadResolutedCallStatementScope::DeclaredModuleSignature(generatedModule->name, generatedModule->parameters));
    generatedModule->statements = visitNonTerminalSymbolWithManyResults<syrec::Statement>(context->statementList()).value_or(syrec::Statement::vec());

    if (moduleIdentifier.has_value() && *moduleIdentifier != "main") {
        utils::BaseSymbolTable::ModuleOverloadResolutionResult moduleOverloadResolutionCall;
        if (context->parameterList() && !context->parameterList()->parameter().empty())
            moduleOverloadResolutionCall = symbolTable->getModulesMatchingSignature(*moduleIdentifier, generatedModule->parameters);
        else
            moduleOverloadResolutionCall = utils::BaseSymbolTable::ModuleOverloadResolutionResult({symbolTable->existsModuleForName(*moduleIdentifier) 
                ? utils::BaseSymbolTable::ModuleOverloadResolutionResult::Result::SingleMatchFound
                : utils::BaseSymbolTable::ModuleOverloadResolutionResult::Result::NoMatchFound, std::nullopt});

        if (moduleOverloadResolutionCall.resolutionResult != utils::BaseSymbolTable::ModuleOverloadResolutionResult::Result::CallerArgumentsInvalid && moduleOverloadResolutionCall.resolutionResult != utils::BaseSymbolTable::ModuleOverloadResolutionResult::NoMatchFound)
            recordSemanticError<SemanticError::DuplicateModuleDeclaration>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()), *moduleIdentifier);
    }
    symbolTable->closeTemporaryScope();
    return generatedModule;
}

std::any CustomModuleVisitor::visitParameterList(TSyrecParser::ParameterListContext* context) {
    if (!context)
        return std::nullopt;

    std::optional<utils::TemporaryVariableScope::ptr> activeTemporaryVariableScope = symbolTable->getActiveTemporaryScope();

    syrec::Variable::vec processedParameterInstances;
    processedParameterInstances.reserve(context->parameter().size());
    for (const auto& antlrParameterContext: context->parameter()) {
        // Not recording declared parameters will lead to overload resolution reporting 'false' errors (or fails to emit them when the semantically or syntactically incorrect module is not recorded in the symbol table)
        // which is an acceptable behaviour in case that the user failed to provide a valid module declaration
        if (const std::optional<syrec::Variable::ptr>& generatedParameterInstance = visitNonTerminalSymbolWithSingleResult<syrec::Variable>(antlrParameterContext); generatedParameterInstance.has_value()) {
            processedParameterInstances.emplace_back(*generatedParameterInstance);

            if (activeTemporaryVariableScope.has_value())
                activeTemporaryVariableScope->get()->recordVariable(*generatedParameterInstance);
        }
    }
    return processedParameterInstances;
}

std::any CustomModuleVisitor::visitParameter(TSyrecParser::ParameterContext* context) {
    if (!context)
        return std::nullopt;

    std::optional<syrec::Variable::Type> parameterVariableType;
    if (context->VAR_TYPE_IN())
        parameterVariableType = syrec::Variable::Type::In;
    else if (context->VAR_TYPE_INOUT())
        parameterVariableType = syrec::Variable::Type::Inout;
    else if (context->VAR_TYPE_OUT())
        parameterVariableType = syrec::Variable::Type::Out;

    if (const std::optional<syrec::Variable::ptr>& generatedParameterInstance = visitNonTerminalSymbolWithSingleResult<syrec::Variable>(context->signalDeclaration()); generatedParameterInstance.has_value() && parameterVariableType.has_value()) {
        generatedParameterInstance->get()->type = *parameterVariableType;
        return generatedParameterInstance;
    }
    return std::nullopt;
}

std::any CustomModuleVisitor::visitSignalList(TSyrecParser::SignalListContext* context) {
    if (!context)
        return std::nullopt;

    std::optional<syrec::Variable::Type> localVariableType;
    if (context->VAR_TYPE_STATE())
        localVariableType = syrec::Variable::Type::State;
    if (context->VAR_TYPE_WIRE())
        localVariableType = syrec::Variable::Type::Wire;

    syrec::Variable::vec processedLocalVariables;
    processedLocalVariables.reserve(context->signalDeclaration().size());

    for (const auto& antlrVariableDeclarationToken : context->signalDeclaration()) {
        if (const std::optional<syrec::Variable::ptr>& generatedLocalVariableDeclaration = visitNonTerminalSymbolWithSingleResult<syrec::Variable>(antlrVariableDeclarationToken); generatedLocalVariableDeclaration.has_value() && localVariableType.has_value()) {
            generatedLocalVariableDeclaration->get()->type = *localVariableType;
            return generatedLocalVariableDeclaration;
        }
    }
    return processedLocalVariables;
}

std::any CustomModuleVisitor::visitSignalDeclaration(TSyrecParser::SignalDeclarationContext* context) {
    if (!context)
        return std::nullopt;

    const std::optional<utils::TemporaryVariableScope::ptr> activeSymbolTableScope = symbolTable->getActiveTemporaryScope();
    const std::optional<std::string> variableIdentifier = context->IDENT() ? std::make_optional(context->IDENT()->getText()) : std::nullopt;
    if (variableIdentifier.has_value() && activeSymbolTableScope.has_value() && activeSymbolTableScope->get()->existsVariableForName(*variableIdentifier))
        recordSemanticError<SemanticError::DuplicateVariableDeclaration>(mapTokenPositionToMessagePosition(*context->IDENT()->getSymbol()), *variableIdentifier);

    std::vector<unsigned int> declaredNumberOfValuesPerDimension;
    declaredNumberOfValuesPerDimension.reserve(context->dimensionTokens.size());

    for (const auto& antlrTokenForDeclaredNumberOfValuesOfDimension: context->dimensionTokens) {
        if (!antlrTokenForDeclaredNumberOfValuesOfDimension)
            continue;

        bool                              didIntegerConstantDeserializationFailToDueOverflow = false;
        const std::optional<unsigned int> parsedIntegerConstantFromAntlrToken                = deserializeConstantFromString(antlrTokenForDeclaredNumberOfValuesOfDimension->getText(), &didIntegerConstantDeserializationFailToDueOverflow);
        
        if (parsedIntegerConstantFromAntlrToken.has_value()) {
            if (didIntegerConstantDeserializationFailToDueOverflow) {
                recordSemanticError<SemanticError::ValueOverflowDueToNoImplicitTruncationPerformed>(mapTokenPositionToMessagePosition(*antlrTokenForDeclaredNumberOfValuesOfDimension), antlrTokenForDeclaredNumberOfValuesOfDimension->getText(), UINT_MAX);
                declaredNumberOfValuesPerDimension.emplace_back(UINT_MAX);
            } else {
                declaredNumberOfValuesPerDimension.emplace_back(*parsedIntegerConstantFromAntlrToken);
            }
        }
    }

    // TODO: Due do the signal bitwidth being an optional part of the signal declaration, the user somehow must supply a default variable bitwidth that should be assumed in that case (or let the parser assume some since the SyReC specification does not provide such a value)
    unsigned int variableBitwidth = 0;
    if (context->signalWidthToken) {
        bool                              didIntegerConstantDeserializationFailToDueOverflow = false;
        const std::optional<unsigned int> parsedIntegerConstantFromAntlrToken                = deserializeConstantFromString(context->signalWidthToken->getText(), &didIntegerConstantDeserializationFailToDueOverflow);
        if (didIntegerConstantDeserializationFailToDueOverflow)
            recordSemanticError<SemanticError::ValueOverflowDueToNoImplicitTruncationPerformed>(mapTokenPositionToMessagePosition(*context->signalWidthToken), context->signalWidthToken->getText(), UINT_MAX);

        if (parsedIntegerConstantFromAntlrToken.has_value())
            variableBitwidth = *parsedIntegerConstantFromAntlrToken;
    }

    // Add an implicit declaration if the user does not define the optional number of dimensions for a variable
    if (context->dimensionTokens.empty())
        declaredNumberOfValuesPerDimension.emplace_back(1);

    if (variableIdentifier.has_value() && !declaredNumberOfValuesPerDimension.empty())
        return std::make_shared<syrec::Variable>(syrec::Variable::Type::In, *variableIdentifier, declaredNumberOfValuesPerDimension, variableBitwidth);
    return std::nullopt;
}

std::any CustomModuleVisitor::visitStatementList(TSyrecParser::StatementListContext* context) {
    if (!context)
        return std::nullopt;

    syrec::Statement::vec processedStatementList;
    processedStatementList.reserve(context->stmts.size());

    for (const auto& antlrContextForStmt: context->stmts)
        if (const std::optional<syrec::Statement::ptr> generatedStmtInstance = statementVisitorInstance->visitStatementTyped(antlrContextForStmt); generatedStmtInstance.has_value())
            processedStatementList.emplace_back(*generatedStmtInstance);

    return processedStatementList;
}