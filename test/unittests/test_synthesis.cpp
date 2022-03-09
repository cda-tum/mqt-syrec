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
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

using namespace syrec;

class SyrecSynthesisTest: public testing::TestWithParam<std::string> {
protected:
    std::string test_circuits_dir = "./circuits/";
    std::string file_name;
    cost_t      qc;
    cost_t      tc;
    unsigned    expected_num_gates;
    unsigned    expected_lines;
    cost_t      expected_qc;
    cost_t      expected_tc;

    void SetUp() override {
        std::string synthesis_param = GetParam();
        file_name                   = test_circuits_dir + GetParam() + ".src";
        std::ifstream i(test_circuits_dir + "circuits_synthesis.json");
        json          j    = json::parse(i);
        expected_num_gates = j[synthesis_param]["num_gates"];
        expected_lines     = j[synthesis_param]["lines"];
        expected_qc        = j[synthesis_param]["quantum_costs"];
        expected_tc        = j[synthesis_param]["transistor_costs"];
    }
};

INSTANTIATE_TEST_SUITE_P(SyrecSynthesisTest, SyrecSynthesisTest,
                         testing::Values(
                                 "alu_2",
                                 "swap_2",
                                 "shift_4"),
                         [](const testing::TestParamInfo<SyrecSynthesisTest::ParamType>& info) {
                             auto s = info.param;
                             std::replace( s.begin(), s.end(), '-', '_');
                             return s; });

TEST_P(SyrecSynthesisTest, GenericSynthesisTest) {
    circuit                            circ;
    applications::program              prog;
    std::string                        error_string;
    std::vector<std::vector<unsigned>> cl;
    std::vector<std::vector<unsigned>> tl;
    std::vector<gate>                  gates_vec;

    error_string = my_read_program(prog, file_name);
    EXPECT_TRUE(error_string.empty());

    EXPECT_TRUE(syrec_synthesis(circ, prog));

    qc = syrec::final_quantum_cost(circ, circ.lines());
    tc = syrec::final_transistor_cost(circ, circ.lines());

    gates_vec = ct_gates(circ);
    for (const gate& g: gates_vec) {
        cl.push_back(control_lines_check(g));
        tl.push_back(target_lines_check(g));
    }

    EXPECT_EQ(expected_num_gates, circ.num_gates());
    EXPECT_EQ(expected_lines, circ.lines());
    EXPECT_EQ(expected_qc, qc);
    EXPECT_EQ(expected_tc, tc);
}