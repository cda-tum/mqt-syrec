#include "core/circuit.hpp"
#include "core/syrec/parser.hpp"
#include "core/syrec/program.hpp"
#include "algorithms/simulation/simple_simulation.hpp"

#include <core/utils/costs.hpp>
#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <core/properties.hpp>
#include <boost/dynamic_bitset.hpp>
#include <string>

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

std::string bitset_to_string(boost::dynamic_bitset<> const& bitset) {
    std::string res;
    for (unsigned i = 0; i < bitset.size(); i++) {
        res += bitset[i] ? "1" : "0";
    }
    return res;
}

namespace syrec {
    class syrec_test: public ::testing::Test {
    protected:
        // any objects needed by all tests
        circuit 	circ;
        applications::	program prog;
        std::string	error_string;
        cost_t      qc;
        cost_t      tc;
        properties::ptr settings;
        properties::ptr statistics;
        bool 		okay1;
        bool 		okay2;

        boost::dynamic_bitset<> input;
        boost::dynamic_bitset<> output;

        void SetUp() override {
            // setup all the individual objects before each test
            error_string = my_read_program(prog, "./circuits/simple_add.src");
            okay1 = syrec::syrec_synthesis(circ, prog);
            qc = syrec::final_quantum_cost(circ, circ.lines());
            tc = syrec::final_transistor_cost(circ, circ.lines());
            input.resize(circ.lines());
            input.set(3);
            output.resize(circ.lines());
            okay2 = simple_simulation(output, circ, input, settings, statistics);

        }
    };

    TEST_F(syrec_test, GenericTest_syrec1) {
        EXPECT_EQ(30, circ.num_gates());
    }

    TEST_F(syrec_test, GenericTest_syrec2) {
        EXPECT_EQ(6, circ.lines());
    }

    TEST_F(syrec_test, GenericTest_syrec3) {
        EXPECT_EQ(54, qc);
    }

    TEST_F(syrec_test, GenericTest_syrec4) {
        EXPECT_EQ(192, tc);
    }


    TEST_F(syrec_test, GenericTest_syrec5) {
        EXPECT_EQ("010100", bitset_to_string(output));
    }
} // namespace dum
