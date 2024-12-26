#include "core/syrec/parser/utils/symbolTable/temporary_variable_scope.hpp"

bool utils::TemporaryVariableScope::ScopeEntry::isReferenceToLoopVariable() const {
    return getReadOnlyLoopVariableData().value_or(nullptr) != nullptr;
}

std::string utils::TemporaryVariableScope::ScopeEntry::getVariableIdentifier() const {
    if (const std::optional<std::shared_ptr<const syrec::Variable>>& entryAsVariable = getReadonlyVariableData(); entryAsVariable.value_or(nullptr))
        return entryAsVariable->get()->name;
    if (const std::optional<std::shared_ptr<const syrec::Number>>& entryAsNumber = getReadOnlyLoopVariableData(); entryAsNumber.value_or(nullptr))
        return entryAsNumber->get()->variableName();

    return "";
}

std::vector<unsigned> utils::TemporaryVariableScope::ScopeEntry::getDeclaredVariableDimensions() const {
    if (const std::optional<std::shared_ptr<const syrec::Variable>>& entryAsVariable = getReadonlyVariableData(); entryAsVariable.value_or(nullptr))
        return entryAsVariable->get()->dimensions;

    return {1};
}

std::optional<unsigned> utils::TemporaryVariableScope::ScopeEntry::getDeclaredVariableBitwidth() const {
    if (const std::optional<std::shared_ptr<const syrec::Variable>>& entryAsVariable = getReadonlyVariableData(); entryAsVariable.value_or(nullptr))
        return entryAsVariable->get()->bitwidth;

    return std::nullopt;
}

std::optional<std::shared_ptr<const syrec::Variable>> utils::TemporaryVariableScope::ScopeEntry::getReadonlyVariableData() const {
    if (std::holds_alternative<syrec::Variable::ptr>(data))
        return std::get<syrec::Variable::ptr>(data);

    return std::nullopt;
}

std::optional<std::shared_ptr<const syrec::Number>> utils::TemporaryVariableScope::ScopeEntry::getReadOnlyLoopVariableData() const {
    if (std::holds_alternative<syrec::Number::ptr>(data))
        return std::get<syrec::Number::ptr>(data);

    return std::nullopt;
}

bool utils::TemporaryVariableScope::existsVariableForName(const std::string_view& signalIdentifier) const {
    return getVariableByName(signalIdentifier).has_value();
}

std::optional<utils::TemporaryVariableScope::ScopeEntry::readOnylPtr> utils::TemporaryVariableScope::getVariableByName(const std::string_view& signalIdentifier) const {
    const auto scopeEntryMatchingSignalIdentifier = !signalIdentifier.empty() ? signalIdentifierLookup.find(signalIdentifier) : signalIdentifierLookup.end();
    if (scopeEntryMatchingSignalIdentifier == signalIdentifierLookup.end())
        return std::nullopt;

    return scopeEntryMatchingSignalIdentifier->second;
}

std::vector<utils::TemporaryVariableScope::ScopeEntry::readOnylPtr> utils::TemporaryVariableScope::getVariablesMatchingType(const std::unordered_set<syrec::Variable::Type>& lookedForVariableTypes) const {
    if (lookedForVariableTypes.empty())
        return {};

    std::vector<ScopeEntry::readOnylPtr> variablesMatchingType;

    auto signalsIterator = signalIdentifierLookup.begin();
    auto endSignalIterator = signalIdentifierLookup.end();
    while (signalsIterator != endSignalIterator) {
        if (const std::shared_ptr<const syrec::Variable> variableData = signalsIterator->second->getReadonlyVariableData().value_or(nullptr);
            variableData && lookedForVariableTypes.count(variableData->type))
            variablesMatchingType.emplace_back(signalsIterator->second);

        ++endSignalIterator;
    }
    return variablesMatchingType;
}

bool utils::TemporaryVariableScope::recordVariable(const syrec::Variable::ptr& signal) {
    if (!signal || signal->name.empty() || existsVariableForName(signal->name))
        return false;

    auto scopeEntryForSignal = std::make_shared<ScopeEntry>(signal);
    signalIdentifierLookup.insert({signal->name, scopeEntryForSignal});
    return true;
}

bool utils::TemporaryVariableScope::recordLoopVariable(const syrec::Number::ptr& loopVariable) {
    if (!loopVariable || !loopVariable->isLoopVariable() || loopVariable->variableName().empty() || existsVariableForName(loopVariable->variableName()))
        return false;

    auto scopeEntryForSignal = std::make_shared<ScopeEntry>(loopVariable);
    signalIdentifierLookup.insert({loopVariable->variableName(), scopeEntryForSignal});
    return true;
}

bool utils::TemporaryVariableScope::removeVariable(const std::string_view& signalIdentifier) {
    const auto scopeEntryMatchingSignalIdentifier = !signalIdentifier.empty() ? signalIdentifierLookup.find(signalIdentifier) : signalIdentifierLookup.end();
    if (scopeEntryMatchingSignalIdentifier == signalIdentifierLookup.end())
        return false;

    signalIdentifierLookup.erase(scopeEntryMatchingSignalIdentifier);
    return false;
}