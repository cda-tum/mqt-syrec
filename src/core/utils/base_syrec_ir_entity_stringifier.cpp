#include "core/utils/base_syrec_ir_entity_stringifier.hpp"

using namespace utils;

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, const syrec::Program& program) {
    resetInternals();

    bool       stringificationSuccessful               = true;
    const auto iteratorToFirstModuleWithNewlinePostfix = program.modules().cbegin();
    const auto iteratorToLastModuleWithNewlinePostfix  = program.modules().empty() ? program.modules().cbegin() : std::prev(program.modules().cend());
    for (auto moduleIterator = iteratorToFirstModuleWithNewlinePostfix; stringificationSuccessful && moduleIterator != iteratorToLastModuleWithNewlinePostfix; ++moduleIterator)
        stringificationSuccessful = *moduleIterator && stringify(outputStream, **moduleIterator) && appendNewlineToStream(outputStream);

    return stringificationSuccessful && (program.modules().size() > 1 ? program.modules().back() && stringify(outputStream, *program.modules().back()) : true);
}

// START OF NON-PUBLIC INTERFACE
void BaseSyrecIrEntityStringifier::resetInternals() {
    currIndentationLevel = 0;
}

bool BaseSyrecIrEntityStringifier::incrementIdentationLevel() noexcept {
    if (indentIdent.empty() || !currIndentationLevel)
        return true;

    if (currIndentationLevel == SIZE_MAX)
        return false;

    ++currIndentationLevel;
    return true;
}

bool BaseSyrecIrEntityStringifier::decrementIdentationLevel() noexcept {
    if (!currIndentationLevel)
        return false;

    --currIndentationLevel;
    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, const syrec::Module& programModule) {
    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, const syrec::Variable& variable) const {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, const syrec::Statement& statement) const {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, const syrec::AssignStatement& assignStatement) const {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, const syrec::CallStatement& callStatement) {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, const syrec::ForStatement& forStatement) {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, const syrec::IfStatement& ifStatement) {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, const syrec::SwapStatement& swapStatement) const {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, const syrec::UnaryStatement& unaryAssignStatement) const {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, const syrec::UncallStatement& uncallStatement) {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, const syrec::Expression& expression) const {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, const syrec::BinaryExpression& binaryExpression) const {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, const syrec::VariableExpression& variableExpression) const {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, const syrec::NumericExpression& numericExpression) const {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, const syrec::VariableAccess& variableAccess) const {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, const syrec::Number& number) const {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    return true;
}


bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, const syrec::Variable::vec& variables) {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, const syrec::Statement::vec& statements, std::ostringstream& containerForStringifiedResult) {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, syrec::Variable::Type variableType) const noexcept {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    switch (variableType) {
        case syrec::Variable::Type::In:
            outputStream << inVariableTypeIdent;
            break;
        case syrec::Variable::Type::Out:
            outputStream << outVariableTypeIdent;
            break;
        case syrec::Variable::Type::Inout:
            outputStream << inoutVariableTypeIdent;
            break;
        case syrec::Variable::Type::Wire:
            outputStream << wireVariableTypeIdent;
            break;
        case syrec::Variable::Type::State:
            outputStream << stateVariableTypeIdent;
            break;
        default:
            return setStreamInFailedState(outputStream);
    }
    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, syrec::BinaryExpression::BinaryOperation operation) const noexcept {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    switch (operation) {
        case syrec::BinaryExpression::BinaryOperation::Add:
            outputStream << "+";
            break;
        case syrec::BinaryExpression::BinaryOperation::BitwiseAnd:
            outputStream << "&";
            break;
        case syrec::BinaryExpression::BinaryOperation::BitwiseOr:
            outputStream << "|";
            break;
        case syrec::BinaryExpression::BinaryOperation::Divide:
            outputStream << "/";
            break;
        case syrec::BinaryExpression::BinaryOperation::Equals:
            outputStream << "==";
            break;
        case syrec::BinaryExpression::BinaryOperation::Exor:
            outputStream << "^";
            break;
        case syrec::BinaryExpression::BinaryOperation::FracDivide:
            outputStream << "*>";
            break;
        case syrec::BinaryExpression::BinaryOperation::GreaterEquals:
            outputStream << ">=";
            break;
        case syrec::BinaryExpression::BinaryOperation::GreaterThan:
            outputStream << ">";
            break;
        case syrec::BinaryExpression::BinaryOperation::LessEquals:
            outputStream << "<=";
            break;
        case syrec::BinaryExpression::BinaryOperation::LessThan:
            outputStream << "<";
            break;
        case syrec::BinaryExpression::BinaryOperation::LogicalAnd:
            outputStream << "&&";
            break;
        case syrec::BinaryExpression::BinaryOperation::LogicalOr:
            outputStream << "||";
            break;
        case syrec::BinaryExpression::BinaryOperation::Modulo:
            outputStream << "%";
            break;
        case syrec::BinaryExpression::BinaryOperation::Multiply:
            outputStream << "*";
            break;
        case syrec::BinaryExpression::BinaryOperation::NotEquals:
            outputStream << "!=";
            break;
        case syrec::BinaryExpression::BinaryOperation::Subtract:
            outputStream << "-";
            break;
        default:
            return setStreamInFailedState(outputStream);
    }
    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, syrec::UnaryStatement::UnaryOperation operation) const noexcept {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    switch (operation) {
        case syrec::UnaryStatement::UnaryOperation::Increment:
            outputStream << "++=";
            break;
        case syrec::UnaryStatement::UnaryOperation::Decrement:
            outputStream << "--=";
            break;
        case syrec::UnaryStatement::UnaryOperation::Invert:
            outputStream << "~=";
            break;
        default:
            return setStreamInFailedState(outputStream);
    }
    return true;
}

bool BaseSyrecIrEntityStringifier::stringify(std::ostream& outputStream, syrec::AssignStatement::AssignOperation operation) const noexcept {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    switch (operation) {
        case syrec::AssignStatement::AssignOperation::Add:
            outputStream << "+=";
            break;
        case syrec::AssignStatement::AssignOperation::Subtract:
            outputStream << "-=";
            break;
        case syrec::AssignStatement::AssignOperation::Exor:
            outputStream << "^=";
            break;
        default:
            return setStreamInFailedState(outputStream);
    }
    return true;
}

bool BaseSyrecIrEntityStringifier::stringifySkipStatement(std::ostream& outputStream) const noexcept {
    if (!outputStream.good() || skipKeywordIdent.empty())
        return setStreamInFailedState(outputStream);

    outputStream << skipKeywordIdent;
    return true;
}

bool BaseSyrecIrEntityStringifier::setStreamInFailedState(std::ostream& stream) {
    if (!stream.bad())
        stream.setstate(std::ios_base::failbit);
    return false;
}


bool BaseSyrecIrEntityStringifier::appendIdentationPaddingSequence(std::ostream& outputStream, const std::string& identiationSequence, std::string::size_type currentIdentationLevel) {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);
    return true;
}

bool BaseSyrecIrEntityStringifier::appendNewlineToStream(std::ostream& outputStream) {
    if (!outputStream.good())
        return setStreamInFailedState(outputStream);

    #if _WIN32 or _WIN64
        outputStream << "\r\n";
    #else
        outputStream << '\n';
    #endif
        return true;
}