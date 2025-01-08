#ifndef CORE_SYREC_PARSER_UTILS_SYMBOLTABLE_BASE_SYMBOLTABLE_HPP
#define CORE_SYREC_PARSER_UTILS_SYMBOLTABLE_BASE_SYMBOLTABLE_HPP

#include "core/syrec/module.hpp"
#include "core/syrec/variable.hpp"
#include "core/syrec/parser/utils/symbolTable/temporary_variable_scope.hpp"

// TODO: Validation of entities during insert?
namespace utils {
    class BaseSymbolTable {
    public:
        struct ModuleOverloadResolutionResult {
            enum Result {
                CallerArgumentsInvalid,
                SingleMatchFound,
                MultipleMatchesFound,
                NoMatchFound
            };
            Result                                              resolutionResult;
            std::optional<std::shared_ptr<const syrec::Module>> moduleMatchingSignature;
        };

        [[maybe_unused]] bool            insertModule(const syrec::Module::ptr& module);
        [[nodiscard]] syrec::Module::vec getModulesByName(const std::string_view& accessedModuleIdentifier) const;
        [[nodiscard]] bool               existsModuleForName(const std::string_view& accessedModuleIdentifier) const;
        // TODO: Should we allow variable type assignments that are not valid based on their semantics if we can prove that no assignment is performed (i.e. assignment of variable of type 'in' to type 'out')
        // TODO: Should module overload resolution be an optional feature and the lookup be performed using exact variable type matching?
        // TODO: Change signature to use std::vector of const variable pointers
        [[nodiscard]] ModuleOverloadResolutionResult getModulesMatchingSignature(const std::string_view& accessedModuleIdentifier, const syrec::Variable::vec& callerArguments) const;

        [[nodiscard]] std::optional<TemporaryVariableScope::ptr>    getActiveTemporaryScope() const;
        [[maybe_unused]] TemporaryVariableScope::ptr                openTemporaryScope();
        [[maybe_unused]] std::optional<TemporaryVariableScope::ptr> closeTemporaryScope();

    protected:
        // To be able to perform heterogeneous lookup using std::string_view in a STL set/dictionary container in C++17 only std::map and std::set can be used. C++20 supports this functionality also for the unordered STL container variants.
        std::map<std::string, syrec::Module::vec, std::less<>> declaredModules;
        std::vector<TemporaryVariableScope::ptr>               temporaryVariableScopes;

        [[nodiscard]] ModuleOverloadResolutionResult getModulesMatchingSignature(const std::string_view& accessedModuleIdentifier, const syrec::Variable::vec& callerArguments, bool validateCallerArguments) const;
        [[nodiscard]] constexpr static bool          doesVariableTypePairCreateOverloadResolutionAmbiguity(syrec::Variable::Type lType, syrec::Variable::Type rType) {
            switch (lType) {
                case syrec::Variable::Type::In:
                    return rType != syrec::Variable::Type::Out;
                case syrec::Variable::Type::Out:
                case syrec::Variable::Type::Inout:
                    return rType != syrec::Variable::Type::In;
                default:
                    return true;
            }
        }
        [[nodiscard]] static bool doVariableStructuresMatch(const syrec::Variable& lVariable, const syrec::Variable& rVariable) noexcept;
    };
} // namespace utils

#endif