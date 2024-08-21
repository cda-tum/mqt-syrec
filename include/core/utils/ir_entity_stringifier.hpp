#ifndef IR_ENTITY_DUMP_EXTENSIONS_HPP
#define IR_ENTITY_DUMP_EXTENSIONS_HPP

#include <optional>
#include <string>

#include "core/syrec/program.hpp"

namespace syrec {
    class IrEntityStringifier {
    public:
        struct CustomStringificationConfig {
            std::string            moduleKeywordIdent          = "module";
            std::string            inVariableTypeIdent         = "in";
            std::string            outVariableTypeIdent        = "out";
            std::string            inoutVariableTypeIdent      = "inout";
            std::string            wireVariableTypeIdent       = "wire";
            std::string            stateVariableTypeIdent      = "state";
            std::string            bitRangeStartIdent          = ".";
            std::string            bitRangeEndIdent            = ":";
            std::string            loopVariablePrefixIdent     = "$";
            std::string            callKeywordIdent            = "call";
            std::string            uncallKeywordIdent          = "uncall";
            std::string            ifKeywordIdent              = "if";
            std::string            thenKeywordIdent            = "then";
            std::string            elseKeywordIdent            = "else";
            std::string            fiKeywordIdent              = "fi";
            std::string            skipKeywordIdent            = "skip";
            std::string            forKeywordIdent             = "for";
            std::string            toKeywordIdent              = "to";
            std::string            stepKeywordIdent            = "step";
            std::string            doKeywordIdent              = "do";
            std::string            rofKeywordIdent             = "rof";
            std::string            swapOperationIdent          = "<=>";
            std::string            newLineIdent                = "\n";
            std::string            identationCharacterSequence = "\t";
            std::string::size_type initialIdentationLevel      = 0;

            [[nodiscard]] static CustomStringificationConfig getDefaultConfig() {
                return {};
            }
        };

        // TODO: All functions could be made static to not require a this reference
        virtual ~IrEntityStringifier() = default;
        IrEntityStringifier(CustomStringificationConfig stringLiteralsLookupConfig):
            currIdentationLevel(stringLiteralsLookupConfig.initialIdentationLevel), stringifiedIdentsLookup(std::move(stringLiteralsLookupConfig)) {}

        [[nodiscard]] virtual std::optional<std::string> stringify(const Program& program);
        [[nodiscard]] virtual std::optional<std::string> stringify(const Module& programModule);
        [[nodiscard]] virtual std::optional<std::string> stringify(const Variable& variable) const;
        [[nodiscard]] virtual std::optional<std::string> stringify(const Statement& statement) const;
        [[nodiscard]] virtual std::optional<std::string> stringify(const AssignStatement& assignStatement) const;
        [[nodiscard]] virtual std::optional<std::string> stringify(const CallStatement& callStatement);
        [[nodiscard]] virtual std::optional<std::string> stringify(const ForStatement& forStatement);
        [[nodiscard]] virtual std::optional<std::string> stringify(const IfStatement& ifStatement);
        [[nodiscard]] virtual std::optional<std::string> stringify(const SwapStatement& swapStatement) const;
        [[nodiscard]] virtual std::optional<std::string> stringify(const UnaryStatement& unaryAssignStatement) const;
        [[nodiscard]] virtual std::optional<std::string> stringify(const UncallStatement& uncallStatement);
        [[nodiscard]] virtual std::optional<std::string> stringifySkipStatement() const noexcept;

        [[nodiscard]] virtual std::optional<std::string> stringify(const Expression& expression) const;
        [[nodiscard]] virtual std::optional<std::string> stringify(const BinaryExpression& binaryExpression) const;
        [[nodiscard]] virtual std::optional<std::string> stringify(const VariableExpression& variableExpression) const;
        [[nodiscard]] virtual std::optional<std::string> stringify(const NumericExpression& numericExpression) const;

        [[nodiscard]] virtual std::optional<std::string> stringify(const VariableAccess& variableAccess) const;
        [[nodiscard]] virtual std::optional<std::string> stringify(const Number& number) const;

    protected:
        std::string::size_type      currIdentationLevel;
        CustomStringificationConfig stringifiedIdentsLookup;

        [[maybe_unused]] bool                            incrementIdentationLevel() noexcept;
        [[maybe_unused]] bool                            decrementIdentationLevel() noexcept;
        [[nodiscard]] virtual std::optional<std::string> stringifyCollection(const Variable::vec& variables) const;
        [[nodiscard]] virtual std::optional<std::string> stringifyCollection(const Statement::vec& statements) const;
        [[nodiscard]] virtual std::optional<std::string> stringifyVariableType(Variable::Type variableType) const noexcept;
        [[nodiscard]] virtual std::optional<std::string> stringifyCallVariant(const std::string& operationKeyword, const std::string& callTargetName, const std::vector<std::string>& callTargetParameters) noexcept;
        [[nodiscard]] static std::optional<std::string>  stringifyOperation(BinaryExpression::BinaryOperation operation) noexcept;
        [[nodiscard]] static std::optional<std::string>  stringifyOperation(UnaryStatement::UnaryOperation operation) noexcept;
        [[nodiscard]] static std::optional<std::string>  stringifyOperation(AssignStatement::AssignOperation operation) noexcept;
        [[nodiscard]] static bool                        appendIdentationPaddingSequence(std::ostringstream& streamToAppendTo, const std::string& identiationSequence, std::string::size_type currentIdentationLevel) noexcept;
    };
} // namespace syrec

#endif