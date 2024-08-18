#include "core/syrec/program.hpp"

#include "ANTLRInputStream.h"
#include "TSyrecParser.h"
#include "TSyrecLexer.h"
#include "core/syrec/parser/components/custom_error_listener.hpp"

#include "core/syrec/parser/utils/parser_messages_container.hpp"
#include "core/syrec/parser/components/custom_module_visitor.hpp"


namespace syrec {
    bool Program::readFile(const std::string& filename, const ReadProgramSettings settings, std::string* error) {
        if (const auto& concatenatedErrors = read(filename, settings); !concatenatedErrors.empty()) {
            *error = concatenatedErrors;
            return false;
        }
        return true;
    }

    bool Program::readProgramFromString(const std::string& content, const ReadProgramSettings& settings, std::string* error) {
        antlr4::ANTLRInputStream  input(content);
        syrecParser::TSyrecLexer  lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);
        syrecParser::TSyrecParser antlrParser(&tokens);

        auto       parserMessageGenerator = std::make_shared<syrecParser::ParserMessagesContainer>(error == nullptr);
        const auto customVisitor          = std::make_unique<syrecParser::CustomModuleVisitor>(parserMessageGenerator);
        const auto customErrorListener    = std::make_unique<syrecParser::CustomErrorListener>(parserMessageGenerator);
        lexer.addErrorListener(customErrorListener.get());
        antlrParser.addErrorListener(customErrorListener.get());

        customVisitor->visitProgram(antlrParser.program());

        lexer.removeErrorListener(customErrorListener.get());
        antlrParser.removeErrorListener(customErrorListener.get());

        if (const auto& generatedErrorMessages = parserMessageGenerator->getMessagesOfType(syrecParser::Message::MessageType::Error); !generatedErrorMessages.empty()) {
            std::stringstream concatenatedErrorMessageContainer;
            for (std::size_t i = 0; i < generatedErrorMessages.size(); ++i)
                concatenatedErrorMessageContainer << generatedErrorMessages.at(i).content << "\n";

            concatenatedErrorMessageContainer << generatedErrorMessages.back().content;
            if (error)
                *error = concatenatedErrorMessageContainer.str();
            return false;
        }
        return true;
    }


    std::string Program::read(const std::string& filename, const ReadProgramSettings settings) {
        std::string foundErrorWhileReadingFileContent;
        const std::optional<std::string> readFileContent = tryReadFileContent(filename, &foundErrorWhileReadingFileContent);
        if (foundErrorWhileReadingFileContent.empty())
            readProgramFromString(*readFileContent, settings, &foundErrorWhileReadingFileContent);

        return foundErrorWhileReadingFileContent;
    }

    std::optional<std::string> Program::tryReadFileContent(std::string_view filename, std::string* foundFileHandlingErrors) {
        if (std::ifstream inputFileStream(filename.data(), std::ifstream::in | std::ifstream::binary); inputFileStream.is_open()) {
            inputFileStream.ignore(std::numeric_limits<std::streamsize>::max());
            const auto fileContentLength = inputFileStream.gcount();
            inputFileStream.clear(); //  Since ignore will have set eof.
            inputFileStream.seekg(0, std::ios_base::beg);

            std::string fileContentBuffer(fileContentLength, ' ');
            inputFileStream.read(fileContentBuffer.data(), fileContentLength);
            if (!inputFileStream.bad())
                return std::make_optional(fileContentBuffer);

            if (foundFileHandlingErrors)
                *foundFileHandlingErrors = "Error while reading content from file @ " + std::string(filename);
        }

        if (foundFileHandlingErrors && foundFileHandlingErrors->empty()) 
            *foundFileHandlingErrors = "Cannot open given circuit file @ " + std::string(filename);
        
        return std::nullopt;
    }
} // namespace syrec
