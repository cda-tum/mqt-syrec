#include "core/circuit.hpp"
#include "core/syrec/parser.hpp"
#include "core/syrec/program.hpp"
#include "algorithms/simulation/simple_simulation.hpp"
#include "functions.hpp"
#include <core/utils/costs.hpp>
#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <core/properties.hpp>
#include <string>

#include "gtest/gtest.h"




namespace syrec {
    class syrec_test_parity: public ::testing::Test {
    protected:
        // any objects needed by all tests
        circuit 	circ;
        applications::	program prog;
        std::string	error_string;
        cost_t      qc;
        cost_t      tc;
        properties::ptr settings;
        properties::ptr statistics;
        bool 		okay1;

        void SetUp() override {
            // setup all the individual objects before each test
            error_string = my_read_program(prog, "./circuits/parity_4.src");
            okay1 = syrec::syrec_synthesis(circ, prog);
            qc = syrec::final_quantum_cost(circ, circ.lines());
            tc = syrec::final_transistor_cost(circ, circ.lines());

        }
    };

    TEST_F(syrec_test_parity, GenericTest_parity1) {
        EXPECT_EQ(15, circ.num_gates());
    }

    TEST_F(syrec_test_parity, GenericTest_parity2) {
        EXPECT_EQ(6, circ.lines());
    }

    TEST_F(syrec_test_parity, GenericTest_parity3) {
        EXPECT_EQ(63, qc);
    }

    TEST_F(syrec_test_parity, GenericTest_parity4) {
        EXPECT_EQ(200, tc);
    }


} // namespace syrec
