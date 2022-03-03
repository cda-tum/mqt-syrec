#include "algorithms/simulation/simple_simulation.hpp"
#include "core/circuit.hpp"
#include "core/syrec/parser.hpp"
#include "core/syrec/program.hpp"
#include "core/test_functions.hpp"

#include "gtest/gtest.h"
#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <core/properties.hpp>
#include <core/utils/costs.hpp>
#include <string>

namespace syrec {
    class syrec_test_multiple_statement: public ::testing::Test {
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

        void SetUp() override {
            // setup all the individual objects before each test
            error_string = my_read_program(prog, "./circuits/multiple_statement_4.src");
            okay1        = syrec::syrec_synthesis(circ, prog);
            qc           = syrec::final_quantum_cost(circ, circ.lines());
            tc           = syrec::final_transistor_cost(circ, circ.lines());
        }
    };

    TEST_F(syrec_test_multiple_statement, GenericTest_multiple_statement1) {
        EXPECT_EQ(172, circ.num_gates());
    }

    TEST_F(syrec_test_multiple_statement, GenericTest_multiple_statement2) {
        EXPECT_EQ(16, circ.lines());
    }

    TEST_F(syrec_test_multiple_statement, GenericTest_multiple_statement3) {
        EXPECT_EQ(316, qc);
    }

    TEST_F(syrec_test_multiple_statement, GenericTest_multiple_statement4) {
        EXPECT_EQ(1344, tc);
    }

} // namespace syrec
