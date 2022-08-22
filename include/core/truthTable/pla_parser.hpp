#pragma once

#include "core/truthTable/truth_table.hpp"

namespace syrec {

    [[maybe_unused]] inline syrec::TruthTable::Cube::Value transform_pla_to_constants(const char& c) {
        syrec::TruthTable::Cube::Value constant;
        syrec::TruthTable::Cube::Value one  = true;
        syrec::TruthTable::Cube::Value zero = false;
        switch (c) {
            case '-':
            case '~':
                return constant;

            case '0':
                return zero;
            case '1':
                return one;

            default:
                assert(false);
                return constant;
        }
    }

    [[maybe_unused]] inline std::string trim(std::string& str) {
        str.erase(str.find_last_not_of(' ') + 1); //suffixing spaces
        str.erase(0, str.find_first_not_of(' ')); //prefixing spaces
        return str;
    }

    [[maybe_unused]] void pla_parser(std::istream& in, syrec::TruthTable& reader);

    [[maybe_unused]] void read_pla(TruthTable& spec, std::istream& in);

    bool read_pla(TruthTable& spec, const std::string& filename);

} // namespace syrec
