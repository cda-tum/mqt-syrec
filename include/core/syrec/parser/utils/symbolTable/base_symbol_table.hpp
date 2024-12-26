#ifndef CORE_SYREC_PARSER_UTILS_SYMBOLTABLE_BASE_SYMBOLTABLE_HPP
#define CORE_SYREC_PARSER_UTILS_SYMBOLTABLE_BASE_SYMBOLTABLE_HPP

#include "core/syrec/module.hpp"
#include "core/syrec/variable.hpp"
#include "core/syrec/parser/utils/symbolTable/temporary_variable_scope.hpp"

// TODO: Validation of entities during insert?
namespace utils {
    class BaseSymbolTable {
    public:
        [[maybe_unused]] bool                         insertModule(const syrec::Module::ptr& module);
        [[nodiscard]] std::vector<syrec::Module::ptr> getModulesByName(const std::string_view& accessedModuleIdentifier) const;
        [[nodiscard]] syrec::Module::vec              getModulesMatchingSignature(const std::string_view& accessedModuleIdentifier, const syrec::Variable::vec& callerArguments) const;

        [[nodiscard]] std::optional<TemporaryVariableScope::ptr>    getActiveTemporaryScope() const;
        [[maybe_unused]] TemporaryVariableScope::ptr                openTemporaryScope();
        [[maybe_unused]] std::optional<TemporaryVariableScope::ptr> closeTemporaryScope();

    protected:
        // To be able to perform heterogeneous lookup using std::string_view in a STL set/dictionary container in C++17 only std::map and std::set can be used. C++20 supports this functionality also for the unordered STL container variants.
        std::map<std::string, syrec::Module::vec, std::less<>> declaredModules;
        std::vector<TemporaryVariableScope::ptr>               temporaryVariableScopes;
    };
} // namespace utils

#endif