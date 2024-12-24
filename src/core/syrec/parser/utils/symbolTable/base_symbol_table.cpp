#include <core/syrec/parser/utils/symbolTable/base_symbol_table.hpp>

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
    return {};
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