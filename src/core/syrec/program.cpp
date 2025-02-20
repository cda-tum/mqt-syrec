#include "ANTLRInputStream.h"
#include "CommonTokenStream.h"
#include "TSyrecParser.h"
#include "TSyrecLexer.h"
#include "core/syrec/parser/components/custom_error_listener.hpp"

#include "core/syrec/program.hpp"
#include "core/syrec/parser/utils/parser_messages_container.hpp"
#include "core/syrec/parser/components/custom_module_visitor.hpp"


namespace syrec {
    std::string Program::read(const std::string& filename, const ReadProgramSettings settings) {
        std::string                      foundErrorWhileReadingFileContent;
        if (const std::optional<std::string> readFileContent = tryReadFileContent(filename, &foundErrorWhileReadingFileContent); readFileContent.has_value() && foundErrorWhileReadingFileContent.empty())
            readProgramFromString(*readFileContent, settings, &foundErrorWhileReadingFileContent);   
        return foundErrorWhileReadingFileContent;
    }
    
    std::string Program::readFromString(const std::string_view& stringifiedProgram, const ReadProgramSettings settings) {
        std::string foundErrorWhileReadingFileContent;
        readProgramFromString(stringifiedProgram, settings, &foundErrorWhileReadingFileContent);
        return foundErrorWhileReadingFileContent;
    }

    bool Program::readFile(const std::string& filename, const ReadProgramSettings settings, std::string* error) {
        if (const auto& concatenatedErrors = read(filename, settings); !concatenatedErrors.empty()) {
            if (error)
                *error = concatenatedErrors;
            return false;
        }
        return true;
    }

    bool Program::readProgramFromString(const std::string_view& content, const ReadProgramSettings& settings, std::string* error) {
        antlr4::ANTLRInputStream  input(content);
        syrec_parser::TSyrecLexer  lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);
        syrec_parser::TSyrecParser antlrParser(&tokens);

        auto       parserMessageGenerator = std::make_shared<syrec_parser::ParserMessagesContainer>();
        const auto customVisitor          = std::make_unique<syrec_parser::CustomModuleVisitor>(parserMessageGenerator, settings);
        const auto customErrorListener    = std::make_unique<syrec_parser::CustomErrorListener>(parserMessageGenerator);
        lexer.addErrorListener(customErrorListener.get());
        antlrParser.addErrorListener(customErrorListener.get());

        const std::optional<std::shared_ptr<Program>> parsedSyrecProgram = customVisitor->parseProgram(antlrParser.program());

        lexer.removeErrorListener(customErrorListener.get());
        antlrParser.removeErrorListener(customErrorListener.get());

        if (const auto& generatedErrorMessages = parserMessageGenerator->getMessagesOfType(syrec_parser::Message::Type::Error); !generatedErrorMessages.empty()) {
            std::stringstream concatenatedErrorMessageContainer;
            for (std::size_t i = 0; i < generatedErrorMessages.size() - 1; ++i) {
                concatenatedErrorMessageContainer << generatedErrorMessages.at(i)->stringify();
                #if _WIN32
                    concatenatedErrorMessageContainer << "\r\n";
                #else 
                    concatenatedErrorMessageContainer << "\n";
                #endif
            }
            concatenatedErrorMessageContainer << generatedErrorMessages.back()->stringify();
            if (error)
                *error = concatenatedErrorMessageContainer.str();
            return false;
        }
        if (parsedSyrecProgram.has_value() && *parsedSyrecProgram)
            modulesVec = parsedSyrecProgram->get()->modulesVec;
        return true;
    }

    std::optional<std::string> Program::tryReadFileContent(std::string_view filename, std::string* foundFileHandlingErrors) {
        if (std::ifstream inputFileStream(filename.data(), std::ifstream::in | std::ifstream::binary); inputFileStream.is_open()) {
            inputFileStream.ignore(std::numeric_limits<std::streamsize>::max());
            const std::streampos fileContentLength = inputFileStream.gcount();
            inputFileStream.clear(); //  Since ignore will have set eof.
            inputFileStream.seekg(0, std::ios_base::beg);

            // Since std::streampos is a signed data type, std::string::size_type will always be larger thus or static_case will work in all cases
            // ReSharper disable once CppRedundantCastExpression
            std::string fileContentBuffer(static_cast<std::string::size_type>(fileContentLength), ' ');
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
