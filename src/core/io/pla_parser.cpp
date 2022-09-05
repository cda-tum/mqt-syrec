#include "core/io/pla_parser.hpp"

namespace syrec {

    void parse_pla(TruthTable& tt, std::istream& in) {
        std::string line;
        std::size_t nInputs  = 0;
        std::size_t nOutputs = 0;
        std::regex  whitespace("\\s+");

        while (in.good() && getline(in, line)) {
            trim(line);
            line = std::regex_replace(line, whitespace, " ");
            if ((line.empty()) || (line.rfind('#', 0) == 0) || (line.rfind(".ilb", 0) == 0) || (line.rfind(".ob", 0) == 0) || (line.rfind(".p", 0) == 0) || (line.rfind(".type ", 0) == 0)) {
                continue;
            }

            else if (line.rfind(".i", 0) == 0) {
                nInputs = std::stoi(line.substr(3));
            }

            else if (line.rfind(".o", 0) == 0) {
                nOutputs = std::stoi(line.substr(3));
            }

            else if (line == ".e") {
                break;
            }

            else {
                assert((line[0] == '0' || line[0] == '1' || line[0] == '-' || line[0] == '~'));

                std::vector<std::string> inputOutputMapping;

                auto        lineString = std::stringstream(line);
                std::string tokenString;
                while (std::getline(lineString, tokenString, ' ')) {
                    inputOutputMapping.emplace_back(tokenString);
                }

                if (inputOutputMapping.size() != 2)
                    throw std::invalid_argument("Expected exactly 2 columns (input and output), received " + std::to_string(inputOutputMapping.size()) + std::string(" columns"));

                if (inputOutputMapping[0].size() != nInputs)
                    throw std::invalid_argument(".i " + std::string("(") + std::to_string(nInputs) + std::string(")") + std::string(" not equal to received number of inputs ") + std::string("(") + std::to_string(inputOutputMapping[0].size()) + std::string(")"));

                if (inputOutputMapping[1].size() != nOutputs)
                    throw std::invalid_argument(".o " + std::string("(") + std::to_string(nOutputs) + std::string(")") + std::string(" not equal to received number of outputs ") + std::string("(") + std::to_string(inputOutputMapping[1].size()) + std::string(")"));

                TruthTable::Cube cubeIn;
                cubeIn.reserve(nInputs);

                for (auto s: inputOutputMapping[0]) {
                    cubeIn.emplace_back(TruthTable::Cube::getValue(s));
                }

                TruthTable::Cube cubeOut;
                cubeOut.reserve(nOutputs);

                for (auto s: inputOutputMapping[1]) {
                    cubeOut.emplace_back(TruthTable::Cube::getValue(s));
                }

                tt.try_emplace(cubeIn, cubeOut);
            }
        }
    }

    bool read_pla(TruthTable& tt, const std::string& filename) {
        std::ifstream is;
        is.open(filename.c_str(), std::ifstream::in);

        if (!is.good()) {
            std::cerr << "Cannot open " + filename << std::endl;

            return false;
        }

        parse_pla(tt, is);

        return true;
    }

} // namespace syrec
