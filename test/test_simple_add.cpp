#include "core/circuit.hpp"
#include "core/syrec/parser.hpp"
#include "core/syrec/program.hpp"
#include <core/utils/costs.hpp>
#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <core/properties.hpp>
#include "gtest/gtest.h"


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

namespace syrec {
    class syrec_test: public ::testing::Test {
    protected:
        // any objects needed by all tests
        circuit 	circ;
        applications::	program prog;
        std::string	error_string;
        bool 		okay;
        void SetUp() override {
            // setup all the individual objects before each test
            error_string = my_read_program(prog, "./circuits/simple_add.src");
            okay = syrec::syrec_synthesis(circ, prog);
        }
    };

    TEST_F(syrec_test, GenericTest) {
        EXPECT_EQ(5, circ.lines());
    }

} // namespace dum
