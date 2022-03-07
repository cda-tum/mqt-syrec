#include "core/circuit.hpp"
#include "core/syrec/program.hpp"
#include "core/test_functions.hpp"

#include "gtest/gtest.h"
#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <core/properties.hpp>
#include <core/utils/costs.hpp>
#include <string>

namespace syrec {
    class syrec_test_input_repeated: public ::testing::Test {
    protected:
        // any objects needed by all tests
        circuit               circ;
        applications::program prog;
        std::string           error_string;
        cost_t                qc = 0;
        cost_t                tc = 0;
        properties::ptr       settings;
        properties::ptr       statistics;

        void SetUp() override {
            // setup all the individual objects before each test
            error_string = my_read_program(prog, "./circuits/input_repeated_4.src");
            syrec::syrec_synthesis(circ, prog);
            qc = syrec::final_quantum_cost(circ, circ.lines());
            tc = syrec::final_transistor_cost(circ, circ.lines());
        }
    };

    TEST_F(syrec_test_input_repeated, GenericTest_input_repeated1) {
        EXPECT_EQ(224, circ.num_gates());
    }

    TEST_F(syrec_test_input_repeated, GenericTest_input_repeated2) {
        EXPECT_EQ(24, circ.lines());
    }

    TEST_F(syrec_test_input_repeated, GenericTest_input_repeated3) {
        EXPECT_EQ(416, qc);
    }

    TEST_F(syrec_test_input_repeated, GenericTest_input_repeated4) {
        EXPECT_EQ(1792, tc);
    }

} // namespace syrec
