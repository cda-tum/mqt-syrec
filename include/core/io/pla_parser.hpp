#pragma once

#include "core/truthTable/truth_table.hpp"

#include <stdexcept>

namespace syrec {

    inline void trim(std::string& str) {
        str.erase(str.find_last_not_of(' ') + 1); //suffixing spaces
        str.erase(0, str.find_first_not_of(' ')); //prefixing spaces
    }

    void parsePla(TruthTable& tt, std::istream& in);

    bool readPla(TruthTable& tt, const std::string& filename);

} // namespace syrec
