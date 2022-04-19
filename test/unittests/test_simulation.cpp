#include "algorithms/simulation/simple_simulation.hpp"
#include "algorithms/synthesis/syrec_synthesis.hpp"
#include "core/circuit.hpp"
#include "core/properties.hpp"
#include "core/syrec/parser.hpp"
#include "core/syrec/program.hpp"

#include "gtest/gtest.h"
#include <algorithm>
#include <boost/dynamic_bitset.hpp>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

using namespace syrec;

class SyrecSimulationTest: public testing::TestWithParam<std::string> {
protected:
    std::string             test_circuits_dir = "./circuits/";
    std::string             file_name;
    boost::dynamic_bitset<> input;
    boost::dynamic_bitset<> output;
    std::vector<int>        set_lines;
    std::string             expected_sim_out;
    std::string             output_string;

    void SetUp() override {
        std::string synthesis_param = GetParam();
        file_name                   = test_circuits_dir + GetParam() + ".src";
        std::ifstream i(test_circuits_dir + "circuits_simulation.json");
        json          j  = json::parse(i);
        expected_sim_out = j[synthesis_param]["sim_out"];
        set_lines        = j[synthesis_param]["set_lines"].get<std::vector<int>>();
    }
};

INSTANTIATE_TEST_SUITE_P(SyrecSimulationTest, SyrecSimulationTest,
                         testing::Values(
                                 "alu_2",
                                 "swap_2",
                                 "simple_add_2",
                                 "multiply_2",
                                 "modulo_2",
                                 "negate_8"),
                         [](const testing::TestParamInfo<SyrecSimulationTest::ParamType>& info) {
                             auto s = info.param;
                             std::replace( s.begin(), s.end(), '-', '_');
                             return s; });

TEST_P(SyrecSimulationTest, GenericSimulationTest) {
    for (int line: set_lines)
        std::cout << line;

    circuit               circ;
    program               prog;
    read_program_settings settings;
    properties::ptr       statistics;
    std::string           error_string;

    error_string = prog.read(file_name, settings);
    EXPECT_TRUE(error_string.empty());

    EXPECT_TRUE(syrec_synthesis(circ, prog));

    input.resize(circ.get_lines());

    for (int line: set_lines)
        input.set(line);

    output.resize(circ.get_lines());

    simple_simulation(output, circ, input, statistics);

    boost::to_string(output, output_string);
    std::reverse(output_string.begin(), output_string.end());

    EXPECT_EQ(expected_sim_out, output_string);
}
