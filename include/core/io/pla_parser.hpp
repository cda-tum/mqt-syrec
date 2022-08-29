#pragma once

#include "core/truthTable/truth_table.hpp"

#include <stdexcept>

namespace syrec {

    inline TruthTable::Cube::Value transformPlaChar(const char& c) {
        switch (c) {
            case '-':
            case '~':
                return {};
            case '0':
                return false;
            case '1':
                return true;
            default:
                throw std::invalid_argument("Unknown Character");
                return {};
        }
    }

    inline void trim(std::string& str) {
        str.erase(str.find_last_not_of(' ') + 1); //suffixing spaces
        str.erase(0, str.find_first_not_of(' ')); //prefixing spaces
    }

    void parse_pla(TruthTable& tt, std::istream& in);

    bool read_pla(TruthTable& tt, const std::string& filename);

} // namespace syrec
