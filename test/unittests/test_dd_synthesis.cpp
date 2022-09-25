#include "algorithms/synthesis/dd_synthesis.hpp"
#include "algorithms/synthesis/encoding.hpp"
#include "core/io/pla_parser.hpp"
#include "dd/FunctionalityConstruction.hpp"

#include "gtest/gtest.h"

using namespace dd::literals;
using namespace syrec;

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
                                 "hwb9_65",
                                 "hwb8_64"),
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

    EXPECT_EQ(ttDD, dd::buildFunctionality(&synthesizer.synthesize(ttDD, ddSynth), ddSynth));

    std::cout << synthesizer.numGate() << std::endl;

    std::cout << synthesizer.getExecutionTime() << std::endl;
}
