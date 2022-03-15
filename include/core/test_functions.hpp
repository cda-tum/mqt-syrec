#include "core/circuit.hpp"
#include "core/gate.hpp"
#include "core/syrec/parser.hpp"
#include "core/syrec/program.hpp"

#include <boost/dynamic_bitset.hpp>
#include <string>

std::vector<unsigned>    control_lines_check(const syrec::gate& g);
std::vector<unsigned>    target_lines_check(const syrec::gate& g);
std::vector<syrec::gate> ct_gates(const syrec::circuit& circ);