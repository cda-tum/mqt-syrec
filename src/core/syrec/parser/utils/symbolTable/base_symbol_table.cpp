#include <algorithm>
#include <set>
#include "core/syrec/parser/utils/symbolTable/base_symbol_table.hpp"
#include "core/syrec/parser/utils/variable_assignability_check.hpp"

bool utils::BaseSymbolTable::insertModule(const syrec::Module::ptr& module) {
    if (!module || module->name.empty())
        return false;

    if (declaredModules.find(module->name) == declaredModules.end())
        declaredModules.insert({module->name, syrec::Module::vec()});

    declaredModules.at(module->name).emplace_back(module);
    return true;
}

syrec::Module::vec utils::BaseSymbolTable::getModulesByName(const std::string_view& accessedModuleIdentifier) const {
    auto modulesMatchingIdentifier = declaredModules.find(accessedModuleIdentifier);
    if (modulesMatchingIdentifier == declaredModules.end())
        return {};

    return modulesMatchingIdentifier->second;
}

syrec::Module::vec utils::BaseSymbolTable::getModulesMatchingSignature(const std::string_view& accessedModuleIdentifier, const syrec::Variable::vec& callerArguments) const {
    // TODO: Should we validate the caller arguments or should it be the responsibility of the caller to provide valid arguments?
    bool                                    areCallerArgumentsUsable = true;
    std::set<std::string_view, std::less<>> callerArgumentsIdentifierLookup;
    for (const auto& callerArgument : callerArguments) {
        areCallerArgumentsUsable &= callerArgument && !callerArgument->name.empty() && !callerArgumentsIdentifierLookup.count(callerArgument->name);
        callerArgumentsIdentifierLookup.insert(callerArgument->name);
    }

    syrec::Module::vec modulesMatchingIdentifier = areCallerArgumentsUsable ? getModulesByName(accessedModuleIdentifier) : syrec::Module::vec();
    modulesMatchingIdentifier.erase(
        std::remove_if(
            modulesMatchingIdentifier.begin(),
            modulesMatchingIdentifier.end(),
            [&callerArguments](const syrec::Module::ptr& moduleMatchingIdentifier) {
                        return moduleMatchingIdentifier->parameters.size() != callerArguments.size()
                            || std::find_first_of(
                                moduleMatchingIdentifier->parameters.cbegin(),
                                moduleMatchingIdentifier->parameters.cend(),
                                callerArguments.cbegin(),
                                callerArguments.cend(),
                                [](const syrec::Variable::ptr& moduleParameter, const syrec::Variable::ptr& callerArgument) {
                                    return moduleParameter->bitwidth != callerArgument->bitwidth
                                    || moduleParameter->dimensions.size() != callerArgument->dimensions.size()
                                    || std::find_first_of(
                                        moduleParameter->dimensions.cbegin(),
                                        moduleParameter->dimensions.cend(),
                                        callerArgument->dimensions.cbegin(),
                                        callerArgument->dimensions.cend(),
                                        [](const auto moduleParameterNumValuesOfDimension, const auto callerArgumentNumValuesOfDimension) {
                                            return moduleParameterNumValuesOfDimension == callerArgumentNumValuesOfDimension;
                                        }) != moduleParameter->dimensions.cend()
                                    || !variableAssignability::doesModuleParameterTypeAllowAssignmentFromVariableType(moduleParameter->type, callerArgument->type);
                        }) != moduleMatchingIdentifier->parameters.cend();
            }),
            modulesMatchingIdentifier.end());
    return modulesMatchingIdentifier;
}

std::optional<std::shared_ptr<utils::TemporaryVariableScope>> utils::BaseSymbolTable::getActiveTemporaryScope() const {
    return temporaryVariableScopes.empty() ? std::nullopt : std::make_optional(temporaryVariableScopes.back());
}

std::shared_ptr<utils::TemporaryVariableScope> utils::BaseSymbolTable::openTemporaryScope() {
    temporaryVariableScopes.emplace_back(std::make_shared<utils::TemporaryVariableScope>());
    return temporaryVariableScopes.back();
}

std::optional<std::shared_ptr<utils::TemporaryVariableScope>> utils::BaseSymbolTable::closeTemporaryScope() {
    if (temporaryVariableScopes.empty())
        return std::nullopt;

    temporaryVariableScopes.pop_back();
    return getActiveTemporaryScope();
}