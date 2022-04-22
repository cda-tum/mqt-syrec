#include "core/syrec/program.hpp"

namespace syrec {

    bool program::read_file(const std::string& filename, const read_program_settings settings, std::string* error) {
        std::string content, line;

        std::ifstream is;
        is.open(filename.c_str(), std::ios::in);

        while (getline(is, line)) {
            content += line + '\n';
        }

        return read_program_from_string(content, settings, error);
    }

    std::string program::read(const std::string& filename, const read_program_settings settings) {
        std::string error_message;
        if (!(read_file(filename, settings, &error_message))) {
            return error_message;
        } else {
            return {};
        }
    }

} // namespace syrec
