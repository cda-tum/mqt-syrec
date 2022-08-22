#include "core/truthTable/pla_parser.hpp"

namespace syrec {

    [[maybe_unused]] void pla_parser(std::istream& in, syrec::TruthTable& reader) {
        std::string line;
        std::regex  whitespace("\\s+");

        while (in.good() && getline(in, line)) {
            trim(line);
            line = std::regex_replace(line, whitespace, std::string(" "));
            if ((line.empty()) || (line.rfind('#', 0) == 0) || (line.rfind(".i", 0) == 0) || (line.rfind(".o", 0) == 0) || (line.rfind(".ilb", 0) == 0) || (line.rfind(".ob", 0) == 0) || (line.rfind(".p", 0) == 0) || (line.rfind(".type ", 0) == 0)) { continue; }

            //std::string comment( std::find_if( line.begin(), line.end(), []( char c ) { return c != '#'; } ) );
            //reader.on_comment( comment );

            //reader.on_num_inputs( static_cast<unsigned>( line.substr( 3 ) ) );

            //reader.on_num_outputs( boost::lexical_cast<unsigned>( line.substr( 3 ) ) );

            //reader.on_num_products( boost::lexical_cast<unsigned>( line.substr( 3 ) ) );

            //reader.on_type( line.substr( 6 ) );
            else if (line == ".e") {
                break;
                //reader.on_end();
            }

            else {
                assert(!line.empty() && (line[0] == '0' || line[0] == '1' || line[0] == '-'));

                std::vector<std::string> inout;
                //boost::split( inout, line, boost::is_any_of(" \t|" ), boost::token_compress_on );
                const int MAX = 200;
                char      input[MAX];
                strcpy(input, line.c_str());
                char* token = std::strtok(input, " \t|");
                while (token != nullptr) {
                    inout.emplace_back(token);
                    token = std::strtok(nullptr, " ");
                }

                assert(inout.size() == 2);

                syrec::TruthTable::Cube cubeIn;
                cubeIn.reserve(inout[0].size());

                for (auto s: inout[0]) {
                    cubeIn.emplace_back(transform_pla_to_constants(s));
                }

                syrec::TruthTable::Cube cubeOut;
                cubeOut.reserve(inout[1].size());

                for (auto s: inout[1]) {
                    cubeOut.emplace_back(transform_pla_to_constants(s));
                }

                reader.insert(cubeIn, cubeOut);
            }
        }
    }

    [[maybe_unused]] void read_pla(syrec::TruthTable& spec, std::istream& in) {
        spec.clear();

        pla_parser(in, spec);

        spec.extend();

        spec.encodeHuffman();

        spec.augmentWithConstants();
    }

    bool read_pla(syrec::TruthTable& spec, const std::string& filename) {
        std::ifstream is;
        is.open(filename.c_str(), std::ifstream::in);

        if (!is.good()) {
            std::cerr << "Cannot open " + filename << std::endl;

            return false;
        }

        read_pla(spec, is);

        return true;
    }

} // namespace syrec
