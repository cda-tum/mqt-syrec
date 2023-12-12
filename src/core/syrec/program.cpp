#include "core/syrec/program.hpp"

namespace syrec {

    bool Program::readFile(const std::string& filename, const ReadProgramSettings settings, std::string* error) {
        std::string content;
        std::string line;

        std::ifstream is;
        is.open(filename.c_str(), std::ios::in);

        while (getline(is, line)) {
            content += line + '\n';
        }

        return readProgramFromString(content, settings, error);
    }

    std::string Program::read(const std::string& filename, const ReadProgramSettings settings) {
        std::string errorMessage;
        if (!(readFile(filename, settings, &errorMessage))) {
            return errorMessage;
        }
        return {};
    }

} // namespace syrec
