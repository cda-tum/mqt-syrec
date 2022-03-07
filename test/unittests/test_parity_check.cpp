#include "core/circuit.hpp"
#include "core/syrec/program.hpp"
#include "core/test_functions.hpp"

#include "gtest/gtest.h"
#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <core/properties.hpp>
#include <core/utils/costs.hpp>
#include <string>

namespace syrec {
    class syrec_test_parity_check: public ::testing::Test {
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
            error_string = my_read_program(prog, "./circuits/parity_check_16.src");
            syrec::syrec_synthesis(circ, prog);
            qc = syrec::final_quantum_cost(circ, circ.lines());
            tc = syrec::final_transistor_cost(circ, circ.lines());
        }
    };

    TEST_F(syrec_test_parity_check, GenericTest_parity_check1) {
        EXPECT_EQ(67, circ.num_gates());
    }

    TEST_F(syrec_test_parity_check, GenericTest_parity_check2) {
        EXPECT_EQ(19, circ.lines());
    }

    TEST_F(syrec_test_parity_check, GenericTest_parity_check3) {
        EXPECT_EQ(323, qc);
    }

    TEST_F(syrec_test_parity_check, GenericTest_parity_check4) {
        EXPECT_EQ(1032, tc);
    }

} // namespace syrec
