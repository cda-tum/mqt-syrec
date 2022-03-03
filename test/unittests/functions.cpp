#include "functions.hpp"

std::string py_read_program(syrec::applications::program& prog, const std::string& filename, const syrec::read_program_settings& settings) {
    std::string error_message;

    if (!(syrec::read_program(prog, filename, settings, &error_message))) {
        return error_message;
    } else {
        return {};
    }
}

std::string my_read_program(syrec::applications::program& prog, const std::string& filename, unsigned default_bitwidth) {
    syrec::read_program_settings settings;
    settings.default_bitwidth = default_bitwidth;
    std::string error         = py_read_program(prog, filename, settings);
    return error;
}

std::string bitset_to_string(boost::dynamic_bitset<> const& bitset) {
    std::string res;
    for (unsigned i = 0; i < bitset.size(); i++) {
        res += bitset[i] ? "1" : "0";
    }
    return res;
}
