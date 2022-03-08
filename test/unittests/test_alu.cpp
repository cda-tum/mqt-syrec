#include "algorithms/simulation/simple_simulation.hpp"
#include "algorithms/synthesis/syrec_synthesis.hpp"
#include "core/circuit.hpp"
#include "core/properties.hpp"
#include "core/syrec/program.hpp"
#include "core/test_functions.hpp"
#include "core/utils/costs.hpp"

#include "gtest/gtest.h"
#include <boost/dynamic_bitset.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace syrec {
    class syrec_test_alu: public ::testing::Test {
    protected:
        // any objects needed by all tests
        std::vector<std::vector<std::string>> content;
        std::string                           word;

        void SetUp() override {
            // setup all the individual objects before each test

            std::vector<std::string>              row;

            std::string                           line;
            std::fstream                          file;

            file.open("./circuits/circuits.csv", std::ios::in);

            while (getline(file, line)) {
                row.clear();

                std::stringstream str(line);

                while (getline(str, word, ','))
                    row.push_back(word);
                content.push_back(row);
            }


            file.close();
        }
    };

    TEST_F(syrec_test_alu, GenericTest_alu1) {
        circuit               circ;
        for (int i = 1; i < (int)content.size(); i++) {


            applications::program prog;
            std::string           error_string;
            cost_t                qc;
            cost_t                tc;
            properties::ptr       settings;
            properties::ptr       statistics;

            boost::dynamic_bitset<> input;
            boost::dynamic_bitset<> output;

            std::vector<std::vector<unsigned>> cl;
            std::vector<std::vector<unsigned>> tl;
            std::vector<gate>                  gates_vec;

            unsigned    expected_num_gates;
            unsigned    expected_lines;
            cost_t      expected_qc;
            cost_t      expected_tc;

            std::string expected_sim_out;

            std::vector<int>                      set_lines;

            std::string                           file_name;
            file_name.clear();

            file_name.append("./circuits/");
            file_name.append(content[i][0]);
            file_name.append(".src");

            std::cout<<"src file: " <<file_name<<std::endl;

            expected_num_gates = std::stoi(content[i][1]);
            expected_lines     = std::stoi(content[i][2]);
            expected_qc        = (cost_t)std::stoi(content[i][3]);
            expected_tc        = (cost_t)std::stoi(content[i][4]);

            std::stringstream str(content[i][5]);
            while (getline(str, word, '-'))
                set_lines.push_back(std::stoi(word));

            expected_sim_out = content[i][6];

            error_string = my_read_program(prog, file_name);
            EXPECT_TRUE(error_string.empty());
            EXPECT_TRUE(syrec::syrec_synthesis(circ, prog));
            qc = syrec::final_quantum_cost(circ, circ.lines());
            tc = syrec::final_transistor_cost(circ, circ.lines());
            gates_vec = ct_gates(circ);
            for (const gate& g: gates_vec) {
                cl.push_back(control_lines_check(g));
                tl.push_back(target_lines_check(g));
            }

            input.resize(circ.lines());

            for (int line: set_lines)
                input.set(line);

            output.resize(circ.lines());
            EXPECT_TRUE(simple_simulation(output, circ, input, settings, statistics));

            EXPECT_EQ(expected_num_gates, circ.num_gates());
            EXPECT_EQ(expected_lines, circ.lines());
            EXPECT_EQ(expected_qc, qc);
            EXPECT_EQ(expected_tc, tc);
            EXPECT_EQ(expected_sim_out, bitset_to_string(output));
        }
    }

} // namespace syrec
