#ifndef CORE_UTILS_IR_ENTITY_DUMP_EXTENSIONS_HPP
#define CORE_UTILS_IR_ENTITY_DUMP_EXTENSIONS_HPP

#include "core/syrec/program.hpp"

#include <string>

namespace utils {
    class BaseSyrecIrEntityStringifier {
    public:
        struct AdditionalFormattingOptions {
            bool        useWhitespaceBetweenOperandsOfBinaryOperation;
            bool        useWhitespaceAfterAfterModuleParameterDeclaration;
            bool        useWhitespaceAfterCallStatementArgumentDefinition;
            bool        useWhitespaceAfterAnyKeyword;
            std::size_t defaultSignalBitwidth;

            AdditionalFormattingOptions():
                useWhitespaceBetweenOperandsOfBinaryOperation(true), useWhitespaceAfterAfterModuleParameterDeclaration(true), useWhitespaceAfterCallStatementArgumentDefinition(true), useWhitespaceAfterAnyKeyword(true), defaultSignalBitwidth(16) {}
        };

        virtual ~BaseSyrecIrEntityStringifier() = default;
        BaseSyrecIrEntityStringifier(const std::optional<AdditionalFormattingOptions>& additionalFormattingOptions)
            : additionalFormattingOptions(additionalFormattingOptions.value_or(AdditionalFormattingOptions())) {}

        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, const syrec::Program& program);
    protected:
        std::string            moduleKeywordIdent      = "module";
        std::string            inVariableTypeIdent     = "in";
        std::string            outVariableTypeIdent    = "out";
        std::string            inoutVariableTypeIdent  = "inout";
        std::string            wireVariableTypeIdent   = "wire";
        std::string            stateVariableTypeIdent  = "state";
        std::string            bitRangeStartIdent      = ".";
        std::string            bitRangeEndIdent        = ":";
        std::string            loopVariablePrefixIdent = "$";
        std::string            callKeywordIdent        = "call";
        std::string            uncallKeywordIdent      = "uncall";
        std::string            ifKeywordIdent          = "if";
        std::string            thenKeywordIdent        = "then";
        std::string            elseKeywordIdent        = "else";
        std::string            fiKeywordIdent          = "fi";
        std::string            skipKeywordIdent        = "skip";
        std::string            forKeywordIdent         = "for";
        std::string            toKeywordIdent          = "to";
        std::string            stepKeywordIdent        = "step";
        std::string            doKeywordIdent          = "do";
        std::string            rofKeywordIdent         = "rof";
        std::string            swapOperationIdent      = "<=>";
        std::string            indentIdent             = "\t";
        std::string::size_type currIndentationLevel     = 0;

        AdditionalFormattingOptions additionalFormattingOptions;

        virtual void resetInternals();
        [[maybe_unused]] bool incrementIdentationLevel() noexcept;
        [[maybe_unused]] bool decrementIdentationLevel() noexcept;

        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, const syrec::Module& programModule);
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, const syrec::Variable& variable) const;
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, const syrec::Statement& statement) const;
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, const syrec::AssignStatement& assignStatement) const;
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, const syrec::CallStatement& callStatement);
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, const syrec::ForStatement& forStatement);
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, const syrec::IfStatement& ifStatement);
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, const syrec::SwapStatement& swapStatement) const;
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, const syrec::UnaryStatement& unaryAssignStatement) const;
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, const syrec::UncallStatement& uncallStatement);
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, const syrec::Expression& expression) const;
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, const syrec::BinaryExpression& binaryExpression) const;
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, const syrec::VariableExpression& variableExpression) const;
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, const syrec::NumericExpression& numericExpression) const;
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, const syrec::VariableAccess& variableAccess) const;
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, const syrec::Number& number) const;

        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, const syrec::Variable::vec& variables);
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, const syrec::Statement::vec& statements, std::ostringstream& containerForStringifiedResult);
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, syrec::Variable::Type variableType) const noexcept;
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, syrec::BinaryExpression::BinaryOperation operation) const noexcept;
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, syrec::UnaryStatement::UnaryOperation operation) const noexcept;
        [[maybe_unused]] virtual bool stringify(std::ostream& outputStream, syrec::AssignStatement::AssignOperation operation) const noexcept;
        [[maybe_unused]] virtual bool stringifySkipStatement(std::ostream& outputStream) const noexcept;

        [[maybe_unused]] static bool setStreamInFailedState(std::ostream& stream);
        [[nodiscard]] static bool    appendIdentationPaddingSequence(std::ostream& outputStream, const std::string& identiationSequence, std::string::size_type currentIdentationLevel);
        [[nodiscard]] static bool    appendNewlineToStream(std::ostream& outputStream);
    };
} // namespace syrec

#endif