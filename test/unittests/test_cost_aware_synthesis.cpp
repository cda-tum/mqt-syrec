#include "algorithms/synthesis/syrec_cost_aware_synthesis.hpp"
#include "core/circuit.hpp"
#include "core/gate.hpp"
#include "core/syrec/program.hpp"

#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <string>

// The .clang-tidy warning about the missing header file seems to be a false positive since the include of the required <nlohmann/json.hpp> is defined in this file.
// Maybe this warning is reported because the nlohmann library is implicitly added by one of the external dependencies?
using json = nlohmann::json; // NOLINT(misc-include-cleaner)

using namespace syrec;

class SyrecCostAwareSynthesisTest: public testing::TestWithParam<std::string> {
protected:
    std::string  testConfigsDir  = "./configs/";
    std::string  testCircuitsDir = "./circuits/";
    std::string  fileName;
    Gate::cost_t qc               = 0;
    Gate::cost_t tc               = 0;
    unsigned     expectedNumGates = 0;
    unsigned     expectedLines    = 0;
    Gate::cost_t expectedQc       = 0;
    Gate::cost_t expectedTc       = 0;

    void SetUp() override {
        const std::string synthesisParam = GetParam();
        fileName                         = testCircuitsDir + GetParam() + ".src";
        std::ifstream i(testConfigsDir + "circuits_cost_aware_synthesis.json");
        json          j  = json::parse(i);
        expectedNumGates = j[synthesisParam]["num_gates"];
        expectedLines    = j[synthesisParam]["lines"];
        expectedQc       = j[synthesisParam]["quantum_costs"];
        expectedTc       = j[synthesisParam]["transistor_costs"];
    }
};

INSTANTIATE_TEST_SUITE_P(SyrecSynthesisTest, SyrecCostAwareSynthesisTest,
                         testing::Values(
                                 "alu_2",
                                 "binary_numeric",
                                 "bitwise_and_2",
                                 "bitwise_or_2",
                                 "bn_2",
                                 "call_8",
                                 "constExpr_8",
                                 "divide_2",
                                 "for_4",
                                 "for_32",
                                 "gray_binary_conversion_16",
                                 "ifCondVariants_4",
                                 "input_repeated_2",
                                 "input_repeated_4",
                                 "logical_and_1",
                                 "logical_or_1",
                                 "modulo_2",
                                 "multiply_2",
                                 "negate_8",
                                 "numeric_2",
                                 "operators_repeated_4",
                                 "parity_4",
                                 "parity_check_16",
                                 "relationalOp_4",
                                 "shift_4",
                                 "simple_add_2",
                                 "single_longstatement_4",
                                 "skip",
                                 "swap_2"),
                         [](const testing::TestParamInfo<SyrecCostAwareSynthesisTest::ParamType>& info) {
                             auto s = info.param;
                             std::replace( s.begin(), s.end(), '-', '_');
                             return s; });

TEST_P(SyrecCostAwareSynthesisTest, GenericSynthesisTest) {
    Circuit                   circ;
    Program                   prog;
    const ReadProgramSettings settings;
    std::string               errorString;

    ASSERT_NO_FATAL_FAILURE(errorString = prog.read(fileName, settings)) << "Unexpected crash during processing of SyReC program";
    ASSERT_TRUE(errorString.empty()) << "Found errors during processing of SyReC program: " << errorString;
    ASSERT_TRUE(CostAwareSynthesis::synthesize(circ, prog));

    qc = circ.quantumCost();
    tc = circ.transistorCost();

    EXPECT_EQ(expectedNumGates, circ.numGates());
    EXPECT_EQ(expectedLines, circ.getLines());
    EXPECT_EQ(expectedQc, qc);
    EXPECT_EQ(expectedTc, tc);
}
