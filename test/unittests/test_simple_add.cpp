#include "algorithms/simulation/simple_simulation.hpp"
#include "core/circuit.hpp"
#include "core/syrec/parser.hpp"
#include "core/syrec/program.hpp"
#include "core/test_functions.hpp"

#include "gtest/gtest.h"
#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <boost/dynamic_bitset.hpp>
#include <core/properties.hpp>
#include <core/utils/costs.hpp>
#include <string>

namespace syrec {
    class syrec_test_simple_add: public ::testing::Test {
    protected:
        // any objects needed by all tests
        circuit               circ;
        applications::program prog;
        std::string           error_string;
        cost_t                qc;
        cost_t                tc;
        properties::ptr       settings;
        properties::ptr       statistics;
        bool                  okay1;
        bool                  okay2;

        boost::dynamic_bitset<> input;
        boost::dynamic_bitset<> output;

        void SetUp() override {
            // setup all the individual objects before each test
            error_string = my_read_program(prog, "./circuits/simple_add_2.src");
            okay1        = syrec::syrec_synthesis(circ, prog);
            qc           = syrec::final_quantum_cost(circ, circ.lines());
            tc           = syrec::final_transistor_cost(circ, circ.lines());
            input.resize(circ.lines());
            input.set(3);
            output.resize(circ.lines());
            okay2 = simple_simulation(output, circ, input, settings, statistics);
        }
    };

    TEST_F(syrec_test_simple_add, GenericTest_simple_add1) {
        EXPECT_EQ(30, circ.num_gates());
    }

    TEST_F(syrec_test_simple_add, GenericTest_simple_add2) {
        EXPECT_EQ(6, circ.lines());
    }

    TEST_F(syrec_test_simple_add, GenericTest_simple_add3) {
        EXPECT_EQ(54, qc);
    }

    TEST_F(syrec_test_simple_add, GenericTest_simple_add4) {
        EXPECT_EQ(192, tc);
    }

    TEST_F(syrec_test_simple_add, GenericTest_simple_add5) {
        EXPECT_EQ("010100", bitset_to_string(output));
    }
} // namespace syrec
