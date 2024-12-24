#ifndef CORE_SYREC_PARSER_UTILS_SYMBOLTABLE_TEMPORARY_VARIABLE_SCOPE_HPP
#define CORE_SYREC_PARSER_UTILS_SYMBOLTABLE_TEMPORARY_VARIABLE_SCOPE_HPP

#include "core/syrec/number.hpp"
#include "core/syrec/variable.hpp"

#include <unordered_set>

namespace utils {
    class TemporaryVariableScope {
    public:
        class ScopeEntry {
        public:
            explicit ScopeEntry(syrec::Variable::ptr signalData):
                data(signalData) {}

            explicit ScopeEntry(syrec::Number::ptr loopVariable):
                data(loopVariable) {}

            [[nodiscard]] bool                                                  isReferenceToLoopVariable() const;
            [[nodiscard]] std::string                                           getVariableIdentifier() const;
            [[nodiscard]] std::vector<unsigned int>                             getDeclaredVariableDimensions() const;
            [[nodiscard]] std::optional<unsigned int>                           getDeclaredVariableBitwidth() const;
            [[nodiscard]] std::optional<std::shared_ptr<const syrec::Variable>> getReadonlyVariableData() const;
            [[nodiscard]] std::optional<std::shared_ptr<const syrec::Number>>   getReadOnlyLoopVariableData() const;

        protected:
            std::variant<syrec::Variable::ptr, syrec::Number::ptr> data;
        };

        [[nodiscard]] bool                                             existsVariableForName(const std::string_view& signalIdentifier) const;
        [[nodiscard]] std::optional<std::shared_ptr<const ScopeEntry>> getVariableByName(const std::string_view& signalIdentifier) const;
        [[nodiscard]] std::vector<std::shared_ptr<const ScopeEntry>>   getVariablesMatchingType(const std::unordered_set<syrec::Variable::Type>& lookedForVariableTypes) const;
        [[maybe_unused]] bool                                          recordVariable(const syrec::Variable::ptr& signal);
        [[maybe_unused]] bool                                          recordLoopVariable(const syrec::Number::ptr& loopVariable);
        [[maybe_unused]] bool                                          removeVariable(const std::string_view& signalIdentifier);

    protected:
        // To be able to perform heterogeneous lookup using std::string_view in a STL set/dictionary container in C++17 only std::map and std::set can be used. C++20 supports this functionality also for the unordered STL container variants.
        std::map<std::string, std::shared_ptr<ScopeEntry>, std::less<>> signalIdentifierLookup;
    };
}
#endif