#include "core/circuit.hpp"
#include "core/syrec/program.hpp"
#include "core/test_functions.hpp"

#include "gtest/gtest.h"
#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <core/properties.hpp>
#include <core/utils/costs.hpp>
#include <string>

namespace syrec {
    class syrec_test_multiply: public ::testing::Test {
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
            error_string = my_read_program(prog, "./circuits/multiply_2.src");
            syrec::syrec_synthesis(circ, prog);
            qc = syrec::final_quantum_cost(circ, circ.lines());
            tc = syrec::final_transistor_cost(circ, circ.lines());
        }
    };

    TEST_F(syrec_test_multiply, GenericTest_multiply1) {
        EXPECT_EQ(5, circ.num_gates());
    }

    TEST_F(syrec_test_multiply, GenericTest_multiply2) {
        EXPECT_EQ(8, circ.lines());
    }

    TEST_F(syrec_test_multiply, GenericTest_multiply3) {
        EXPECT_EQ(17, qc);
    }

    TEST_F(syrec_test_multiply, GenericTest_multiply4) {
        EXPECT_EQ(64, tc);
    }

} // namespace syrec
