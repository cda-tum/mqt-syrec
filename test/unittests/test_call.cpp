#include "core/circuit.hpp"
#include "core/syrec/program.hpp"
#include "core/test_functions.hpp"

#include "gtest/gtest.h"
#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <core/properties.hpp>
#include <core/utils/costs.hpp>
#include <string>

namespace syrec {
    class syrec_test_call: public ::testing::Test {
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
            error_string = my_read_program(prog, "./circuits/call_8.src");
            syrec::syrec_synthesis(circ, prog);
            qc = syrec::final_quantum_cost(circ, circ.lines());
            tc = syrec::final_transistor_cost(circ, circ.lines());
        }
    };

    TEST_F(syrec_test_call, GenericTest_call1) {
        EXPECT_EQ(196, circ.num_gates());
    }

    TEST_F(syrec_test_call, GenericTest_call2) {
        EXPECT_EQ(25, circ.lines());
    }

    TEST_F(syrec_test_call, GenericTest_call3) {
        EXPECT_EQ(1412, qc);
    }

    TEST_F(syrec_test_call, GenericTest_call4) {
        EXPECT_EQ(3520, tc);
    }

} // namespace syrec
