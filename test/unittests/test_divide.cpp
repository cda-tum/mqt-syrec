#include "core/circuit.hpp"
#include "core/syrec/program.hpp"
#include "core/test_functions.hpp"

#include "gtest/gtest.h"
#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <core/properties.hpp>
#include <core/utils/costs.hpp>
#include <string>

namespace syrec {
    class syrec_test_divide: public ::testing::Test {
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
            error_string = my_read_program(prog, "./circuits/divide_2.src");
            syrec::syrec_synthesis(circ, prog);
            qc = syrec::final_quantum_cost(circ, circ.lines());
            tc = syrec::final_transistor_cost(circ, circ.lines());
        }
    };

    TEST_F(syrec_test_divide, GenericTest_divide1) {
        EXPECT_EQ(252, circ.num_gates());
    }

    TEST_F(syrec_test_divide, GenericTest_divide2) {
        EXPECT_EQ(22, circ.lines());
    }

    TEST_F(syrec_test_divide, GenericTest_divide3) {
        EXPECT_EQ(1076, qc);
    }

    TEST_F(syrec_test_divide, GenericTest_divide4) {
        EXPECT_EQ(2832, tc);
    }

} // namespace syrec
