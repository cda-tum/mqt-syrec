#include "algorithms/simulation/simple_simulation.hpp"
#include "core/circuit.hpp"
#include "core/syrec/program.hpp"
#include "core/test_functions.hpp"
#include "gtest/gtest.h"
#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <boost/dynamic_bitset.hpp>
#include <core/properties.hpp>
#include <core/utils/costs.hpp>
#include <string>

namespace syrec {
    class syrec_test_alu: public ::testing::Test {
    protected:
        // any objects needed by all tests
        circuit               circ;
        applications::program prog;
        std::string           error_string;
        cost_t                qc = 0;
        cost_t                tc = 0;
        properties::ptr       settings;
        properties::ptr       statistics;

        boost::dynamic_bitset<> input;
        boost::dynamic_bitset<> output;

        void SetUp() override {
            // setup all the individual objects before each test

            file.open("test.csv", std::ios::in);

            while (getline(file, line))
            {
                row.clear();

                std::stringstream str(line);

                while(getline(str, word, ','))
                    row.push_back(word);
                content.push_back(row);
            }

            file.close();
        }
    };

    TEST_F(syrec_test_alu, GenericTest_alu1) {
        EXPECT_EQ(46, circ.num_gates());
    }

    TEST_F(syrec_test_alu, GenericTest_alu2) {
        EXPECT_EQ(7, circ.lines());
    }

    TEST_F(syrec_test_alu, GenericTest_alu3) {
        EXPECT_EQ(222, qc);
    }

    TEST_F(syrec_test_alu, GenericTest_alu4) {
        EXPECT_EQ(640, tc);
    }

    TEST_F(syrec_test_alu, GenericTest_alu5) {
        EXPECT_EQ("1010001", bitset_to_string(output));
    }
} // namespace syrec
