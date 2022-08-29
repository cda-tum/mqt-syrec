#include "core/io/pla_parser.hpp"

namespace syrec {

    [[maybe_unused]] void parse_pla(TruthTable& reader, std::istream& in) {
        std::string line;
        std::regex  whitespace("\\s+");

        while (in.good() && getline(in, line)) {
            trim(line);
            line = std::regex_replace(line, whitespace, " ");
            if ((line.empty()) || (line.rfind('#', 0) == 0) || (line.rfind(".i", 0) == 0) || (line.rfind(".o", 0) == 0) || (line.rfind(".ilb", 0) == 0) || (line.rfind(".ob", 0) == 0) || (line.rfind(".p", 0) == 0) || (line.rfind(".type ", 0) == 0)) {
                continue;
            }

            else if (line == ".e") {
                break;
            }

            else {
                assert(!line.empty() && (line[0] == '0' || line[0] == '1' || line[0] == '-' || line[0] == '~'));

                std::vector<std::string> inout;

                auto        lineString = std::stringstream(line);
                std::string tokenString;
                while (std::getline(lineString, tokenString, ' ')) {
                    inout.emplace_back(tokenString);
                }

                if (inout.size() != 2)
                    std::cerr << "I/O not available" << std::endl;

                TruthTable::Cube cubeIn;
                cubeIn.reserve(inout[0].size());

                for (auto s: inout[0]) {
                    cubeIn.emplace_back(transformPlaChar(s));
                }

                TruthTable::Cube cubeOut;
                cubeOut.reserve(inout[1].size());

                for (auto s: inout[1]) {
                    cubeOut.emplace_back(transformPlaChar(s));
                }

                reader.insert(cubeIn, cubeOut);
            }
        }
    }

    bool read_pla(TruthTable& reader, const std::string& filename) {
        std::ifstream is;
        is.open(filename.c_str(), std::ifstream::in);

        if (!is.good()) {
            std::cerr << "Cannot open " + filename << std::endl;

            return false;
        }

        parse_pla(reader, is);

        return true;
    }

} // namespace syrec
