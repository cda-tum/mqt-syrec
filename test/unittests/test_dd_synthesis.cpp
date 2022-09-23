#include "algorithms/synthesis/dd_synthesis.hpp"
#include "algorithms/synthesis/encoding.hpp"
#include "core/io/pla_parser.hpp"

#include "gtest/gtest.h"

using namespace dd::literals;
using namespace syrec;
using std::system;

class TestDDSynth: public testing::TestWithParam<std::string> {
protected:
    TruthTable                     tt{};
    std::string                    testCircuitsDir = "./circuits/";
    std::unique_ptr<dd::Package<>> ddSynth         = std::make_unique<dd::Package<>>(9U);
    std::string                    fileName;

    void SetUp() override {
        fileName = testCircuitsDir + GetParam() + ".pla";
    }
};

INSTANTIATE_TEST_SUITE_P(TestDDSynth, TestDDSynth,
                         testing::Values(
                                 "cnot",
                                 "swap",
                                 "toffoli",
                                 "x2Bit",
                                 "test_dd_synthesis_1",
                                 "test_dd_synthesis_2",
                                 "3_17_6",
                                 "bitwiseXor2Bit",
                                 "adder2Bit",
                                 "adder3Bit",
                                 "4_49_7",
                                 "hwb4_12",
                                 "hwb5_13",
                                 "hwb6_14",
                                 "hwb7_15",
                                 "hwb8_64",
                                 "hwb9_65"),
                         [](const testing::TestParamInfo<TestDDSynth::ParamType>& info) {
                             auto s = info.param;
                             std::replace( s.begin(), s.end(), '-', '_');
                             return s; });

TEST_P(TestDDSynth, GenericDDSynthesisTest) {
    EXPECT_TRUE(readPla(tt, fileName));

    extend(tt);

    auto ttDD = buildDD(tt, ddSynth);

    EXPECT_TRUE(ttDD.p != nullptr);

    EXPECT_NE(ttDD, ddSynth->makeIdent(tt.nInputs()));

    DDSynthesizer synthesizer(tt.nInputs());

    const auto srcCpy = ttDD;

    ddSynth->incRef(srcCpy);

    synthesizer.synthesize(ttDD, ddSynth);

    EXPECT_EQ(srcCpy, synthesizer.buildFunctionality(ddSynth));

    std::cout << synthesizer.numGate() << std::endl;

    std::cout << synthesizer.getExecutionTime() << std::endl;

    EXPECT_EQ(ttDD, ddSynth->makeIdent(tt.nInputs()));
}
