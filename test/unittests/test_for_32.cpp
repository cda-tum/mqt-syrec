#include "core/circuit.hpp"
#include "core/syrec/program.hpp"
#include "core/test_functions.hpp"

#include "gtest/gtest.h"
#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <core/properties.hpp>
#include <core/utils/costs.hpp>
#include <string>

namespace syrec {
    class syrec_test_for_32: public ::testing::Test {
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
            error_string = my_read_program(prog, "./circuits/for_32.src");
            syrec::syrec_synthesis(circ, prog);
            qc = syrec::final_quantum_cost(circ, circ.lines());
            tc = syrec::final_transistor_cost(circ, circ.lines());
        }
    };

    TEST_F(syrec_test_for_32, GenericTest_for321) {
        EXPECT_EQ(4738, circ.num_gates());
    }

    TEST_F(syrec_test_for_32, GenericTest_for322) {
        EXPECT_EQ(385, circ.lines());
    }

    TEST_F(syrec_test_for_32, GenericTest_for323) {
        EXPECT_EQ(27522, qc);
    }

    TEST_F(syrec_test_for_32, GenericTest_for324) {
        EXPECT_EQ(75520, tc);
    }

} // namespace syrec
