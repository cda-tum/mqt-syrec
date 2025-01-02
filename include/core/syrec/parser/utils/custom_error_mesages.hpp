#ifndef CORE_SYREC_PARSER_UTILS_CUSTOM_ERROR_MESSAGES_HPP
#define CORE_SYREC_PARSER_UTILS_CUSTOM_ERROR_MESSAGES_HPP

// TODO: We might improve the readability of the error messages as well as the ease to create semantic errors if we use named parameters in the message formats instead of simply enumerating the user provided arguments
namespace syrecParser {
    enum class SemanticError : unsigned int {
        UnknownIdentifier
    };

    /// Get the identifier associated with a given semantic error
    /// @tparam semanticError The semantic error
    /// @return The identifier for the semantic error or an empty string if none exists.
    template<SemanticError semanticError>
    [[nodiscard]] constexpr std::string_view getIdentifierForSemanticError() {
        // Note: This function can only be implemented as a constexpr if the semantic error is provided as a template argument and not as a function parameter.
        switch (semanticError) {
            case SemanticError::UnknownIdentifier:
                return "TEST";
            default:
                return "";
        }
    }

    /// Get the format of the error message associated with a given semantic error.
    /// @tparam semanticError The semantic error
    /// @return The format of the error message or an empty string if none exists.
    template<SemanticError semanticError>
    [[nodiscard]] constexpr std::string_view getFormatForSemanticErrorMessage() {
        // Note: This function can only be implemented as a constexpr if the semantic error is provided as a template argument and not as a function parameter.
        switch (semanticError) {
            case SemanticError::UnknownIdentifier:
                return "{:d}";
            default:
                return "";
        }
    }
}
#endif