#include "algorithms/simulation/simple_simulation.hpp"
#include "core/circuit.hpp"
#include "core/syrec/parser.hpp"
#include "core/syrec/program.hpp"
#include "functions.hpp"

#include "gtest/gtest.h"
#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <boost/dynamic_bitset.hpp>
#include <core/properties.hpp>
#include <core/utils/costs.hpp>
#include <string>

namespace syrec {
    class syrec_test_alu: public ::testing::Test {
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
            error_string = my_read_program(prog, "./circuits/alu_2.src");
            okay1        = syrec::syrec_synthesis(circ, prog);
            qc           = syrec::final_quantum_cost(circ, circ.lines());
            tc           = syrec::final_transistor_cost(circ, circ.lines());
            input.resize(circ.lines());
            input.set(0);
            input.set(6);
            output.resize(circ.lines());
            okay2 = simple_simulation(output, circ, input, settings, statistics);
        }
    };

    TEST_F(syrec_test_alu, GenericTest_alu1) {
        EXPECT_EQ(46, circ.num_gates());
    }

    TEST_F(syrec_test_alu, GenericTest_alu2) {
        EXPECT_EQ(7, circ.lines());
    }

    TEST_F(syrec_test_alu, GenericTest_alu3) {
        EXPECT_EQ(222, qc);
    }

    TEST_F(syrec_test_alu, GenericTest_alu4) {
        EXPECT_EQ(640, tc);
    }

    TEST_F(syrec_test_alu, GenericTest_alu5) {
        EXPECT_EQ("1010001", bitset_to_string(output));
    }
} // namespace syrec
