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

template<typename T>
void properties_set(syrec::properties& prop, const std::string& key, const T& value) {
    prop.set(key, value);
}

int main() {
    /// build the circuit
    syrec::applications::program prog;
    //[[maybe_unused]] syrec::read_program_settings prog_settings;
    std::string            error_string = my_read_program(prog, "./circuits/simple_add.src");
    syrec::circuit         circ;
    syrec::properties::ptr property1;
    //std::string variable_name_format = "%1$s%3$s.%2$d";
    //std::string main_module = "";
    //unsigned if_realization = 0u;
    //bool efficient_controls = true;
    //bool modules_hierarchy = false;
    //properties_set<std::string>(property1, "variable_name_format", variable_name_format);
    //properties_set<std::string>(property1, "main_module", main_module);
    //properties_set<unsigned>(property1, "if_realization", if_realization);
    //properties_set<bool>(property1, "efficient_controls", efficient_controls);
    //properties_set<bool>(property1, "modules_hierarchy", modules_hierarchy );
    syrec::properties::ptr property2;
    if (syrec::syrec_synthesis(circ, prog, property1, property2)) {
        std::cout << "Inside syrec_synthesis" << std::endl;
        std::cout << circ.num_gates() << std::endl;
        std::cout << circ.lines() << std::endl;
    } else {
        std::cout << "not inside syrec_synthesis" << std::endl;
    }
    //std::cout<<"working"<<std::endl;
    return 0;
} // namespace dum
