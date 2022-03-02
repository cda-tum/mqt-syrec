//#include "Dummy.hpp"
#include "core/circuit.hpp"
#include "core/syrec/parser.hpp"
#include "core/syrec/program.hpp"

#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <core/properties.hpp>
//#include "gtest/gtest.h"

#include <string>

//using namespace syrec;
//using namespace syrec::applications;
std::string py_read_program(syrec::applications::program& prog, const std::string& filename, const syrec::read_program_settings& settings) {
    std::string error_message;

    if (!(syrec::read_program(prog, filename, settings, &error_message))) {
        return error_message;
    } else {
        return {};
    }
}

std::string my_read_program(syrec::applications::program& prog, const std::string& filename, unsigned default_bitwidth = 32) {
    syrec::read_program_settings settings;
    settings.default_bitwidth = default_bitwidth;
    std::string error         = py_read_program(prog, filename, settings);
    return error;
}

//template<typename T>
//void properties_set(syrec::properties& prop, const std::string& key, const T& value) {
//    prop.set(key, value);
//}

int main() {
    /// build the circuit
    syrec::applications::program prog;
    std::string            error_string = my_read_program(prog, "./circuits/simple_add.src");
    syrec::circuit         circ;
    syrec::syrec_synthesis(circ, prog);

    return 0;
} // namespace dum
