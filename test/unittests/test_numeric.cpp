#include "core/circuit.hpp"
#include "core/syrec/program.hpp"
#include "core/test_functions.hpp"

#include "gtest/gtest.h"
#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <core/properties.hpp>
#include <core/utils/costs.hpp>
#include <string>

namespace syrec {
    class syrec_test_numeric: public ::testing::Test {
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
            error_string = my_read_program(prog, "./circuits/numeric_2.src");
            syrec::syrec_synthesis(circ, prog);
            qc = syrec::final_quantum_cost(circ, circ.lines());
            tc = syrec::final_transistor_cost(circ, circ.lines());
        }
    };

    TEST_F(syrec_test_numeric, GenericTest_numeric1) {
        EXPECT_EQ(34, circ.num_gates());
    }

    TEST_F(syrec_test_numeric, GenericTest_numeric2) {
        EXPECT_EQ(68, circ.lines());
    }

    TEST_F(syrec_test_numeric, GenericTest_numeric3) {
        EXPECT_EQ(34, qc);
    }

    TEST_F(syrec_test_numeric, GenericTest_numeric4) {
        EXPECT_EQ(272, tc);
    }

} // namespace syrec
