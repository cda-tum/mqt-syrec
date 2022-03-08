#include "core/circuit.hpp"
#include "core/gate.hpp"
#include "core/syrec/parser.hpp"
#include "core/syrec/program.hpp"

#include <boost/dynamic_bitset.hpp>
#include <string>

std::string py_read_program(syrec::applications::program& prog, const std::string& filename, const syrec::read_program_settings& settings);

std::string              my_read_program(syrec::applications::program& prog, const std::string& filename, unsigned default_bitwidth = 32);
std::string              bitset_to_string(boost::dynamic_bitset<> const& bitset);
std::vector<unsigned>    control_lines_check(const syrec::gate& g);
std::vector<unsigned>    target_lines_check(const syrec::gate& g);
std::vector<syrec::gate> ct_gates(const syrec::circuit& circ);
