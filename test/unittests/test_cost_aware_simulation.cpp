#include "algorithms/simulation/simple_simulation.hpp"
#include "algorithms/synthesis/syrec_cost_aware_synthesis.hpp"
#include "core/circuit.hpp"
#include "core/properties.hpp"
#include "core/syrec/program.hpp"

#include "gtest/gtest.h"
#include <algorithm>
#include <boost/dynamic_bitset.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

using namespace syrec;

class SyrecAddLinesSimulationTest: public testing::TestWithParam<std::string> {
protected:
    std::string             testConfigsDir  = "./configs/";
    std::string             testCircuitsDir = "./circuits/";
    std::string             fileName;
    boost::dynamic_bitset<> input;
    boost::dynamic_bitset<> output;
    std::vector<int>        setLines;
    std::string             expectedSimOut;
    std::string             outputString;

    void SetUp() override {
        std::string synthesisParam = GetParam();
        fileName                   = testCircuitsDir + GetParam() + ".src";
        std::ifstream i(testConfigsDir + "circuits_cost_aware_simulation.json");
        json          j = json::parse(i);
        expectedSimOut  = j[synthesisParam]["sim_out"];
        setLines        = j[synthesisParam]["set_lines"].get<std::vector<int>>();
    }
};

INSTANTIATE_TEST_SUITE_P(SyrecSimulationTest, SyrecAddLinesSimulationTest,
                         testing::Values(
                                 "alu_2",
                                 "swap_2",
                                 "simple_add_2",
                                 "multiply_2",
                                 "modulo_2",
                                 "negate_8"),
                         [](const testing::TestParamInfo<SyrecAddLinesSimulationTest::ParamType>& info) {
                             auto s = info.param;
                             std::replace( s.begin(), s.end(), '-', '_');
                             return s; });

TEST_P(SyrecAddLinesSimulationTest, GenericSimulationTest) {
    Circuit             circ;
    program             prog;
    ReadProgramSettings settings;
    Properties::ptr     statistics;
    std::string         errorString;

    errorString = prog.read(fileName, settings);
    EXPECT_TRUE(errorString.empty());

    EXPECT_TRUE(CostAware::synthesize(circ, prog));

    input.resize(circ.getLines());

    for (int line: setLines) {
        input.set(line);
    }

    output.resize(circ.getLines());

    simpleSimulation(output, circ, input, statistics);

    boost::to_string(output, outputString);
    std::reverse(outputString.begin(), outputString.end());

    EXPECT_EQ(expectedSimOut, outputString);
}
