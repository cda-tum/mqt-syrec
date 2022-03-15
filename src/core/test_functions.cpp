#include "core/test_functions.hpp"

class const_iterator;

std::vector<unsigned> control_lines_check(const syrec::gate& g) {
    //syrec::gate::line_container c;
    std::vector<unsigned> l;
    //control_lines(g, std::insert_iterator<syrec::gate::line_container>(c, c.begin()));
    //for (const auto& control: c) {
    //    l.push_back(control);
    //}
    for (auto c = g.begin_controls(); c != g.end_controls(); ++c) {
        l.push_back(*c);
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