#include "algorithms/simulation/simple_simulation.hpp"
#include "core/circuit.hpp"
#include "core/syrec/program.hpp"
#include "core/test_functions.hpp"

#include "gtest/gtest.h"
#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <core/properties.hpp>
#include <core/utils/costs.hpp>
#include <string>

namespace syrec {
    class syrec_test_swap: public ::testing::Test {
    protected:
        // any objects needed by all tests
        circuit               circ;
        applications::program prog;
        std::string           error_string;
        cost_t                qc = 0;
        cost_t                tc = 0;
        properties::ptr       settings;
        properties::ptr       statistics;

        boost::dynamic_bitset<> input;
        boost::dynamic_bitset<> output;

        void SetUp() override {
            // setup all the individual objects before each test
            error_string = my_read_program(prog, "./circuits/swap_2.src");
            syrec::syrec_synthesis(circ, prog);
            qc = syrec::final_quantum_cost(circ, circ.lines());
            tc = syrec::final_transistor_cost(circ, circ.lines());
            input.resize(circ.lines());
            input.set(2);
            input.set(3);
            output.resize(circ.lines());
            simple_simulation(output, circ, input, settings, statistics);
        }
    };

    TEST_F(syrec_test_swap, GenericTest_swap1) {
        EXPECT_EQ(2, circ.num_gates());
    }

    TEST_F(syrec_test_swap, GenericTest_swap2) {
        EXPECT_EQ(4, circ.lines());
    }

    TEST_F(syrec_test_swap, GenericTest_swap3) {
        EXPECT_EQ(2, qc);
    }

    TEST_F(syrec_test_swap, GenericTest_swap4) {
        EXPECT_EQ(0, tc);
    }

    TEST_F(syrec_test_swap, GenericTest_swap5) {
        EXPECT_EQ("1100", bitset_to_string(output));
    }

} // namespace syrec
