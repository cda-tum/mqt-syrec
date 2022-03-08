#include "core/test_functions.hpp"

class const_iterator;
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

std::vector<unsigned> control_lines_check(const syrec::gate& g) {
    syrec::gate::line_container c;
    std::vector<unsigned>       l;
    control_lines(g, std::insert_iterator<syrec::gate::line_container>(c, c.begin()));
    for (const auto& control: c) {
        l.push_back(control);
    }
    return l;
}

std::vector<unsigned> target_lines_check(const syrec::gate& g) {
    syrec::gate::line_container c;
    std::vector<unsigned>       l;
    target_lines(g, std::insert_iterator<syrec::gate::line_container>(c, c.begin()));
    for (const auto& target: c) {
        l.push_back(target);
    }
    return l;
}

std::vector<syrec::gate> ct_gates(const syrec::circuit& circ) {
    std::vector<syrec::gate>       my_gates;
    syrec::circuit::const_iterator first = circ.begin();
    syrec::circuit::const_iterator last  = circ.end();
    while (first != last) {
        my_gates.push_back(*first);
        ++first;
    }
    return my_gates;
}
