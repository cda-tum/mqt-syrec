#include "core/circuit.hpp"
#include "core/syrec/program.hpp"
#include "core/test_functions.hpp"

#include "gtest/gtest.h"
#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <core/properties.hpp>
#include <core/utils/costs.hpp>
#include <string>

namespace syrec {
    class syrec_test_bn: public ::testing::Test {
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
            error_string = my_read_program(prog, "./circuits/bn_2.src");
            syrec::syrec_synthesis(circ, prog);
            qc = syrec::final_quantum_cost(circ, circ.lines());
            tc = syrec::final_transistor_cost(circ, circ.lines());
        }
    };

    TEST_F(syrec_test_bn, GenericTest_parity_bn1) {
        EXPECT_EQ(296, circ.num_gates());
    }

    TEST_F(syrec_test_bn, GenericTest_parity_bn2) {
        EXPECT_EQ(48, circ.lines());
    }

    TEST_F(syrec_test_bn, GenericTest_parity_bn3) {
        EXPECT_EQ(732, qc);
    }

    TEST_F(syrec_test_bn, GenericTest_parity_bn4) {
        EXPECT_EQ(2544, tc);
    }

} // namespace syrec
