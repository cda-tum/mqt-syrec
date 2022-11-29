#include "algorithms/simulation/circuit_to_truthtable.hpp"
#include "algorithms/synthesis/dd_synthesis.hpp"
#include "algorithms/synthesis/encoding.hpp"
#include "core/io/pla_parser.hpp"

#include "gtest/gtest.h"

using namespace dd::literals;
using namespace syrec;

class TestDDSynthDc: public testing::TestWithParam<std::string> {
protected:
    TruthTable                     tt{};
    std::string                    testCircuitsDir = "./circuits/";
    std::unique_ptr<dd::Package<>> dd              = std::make_unique<dd::Package<>>(15U);
    std::string                    fileName;

    void SetUp() override {
        fileName = testCircuitsDir + GetParam() + ".pla";
    }
};

INSTANTIATE_TEST_SUITE_P(TestDDSynth, TestDDSynthDc,
                         testing::Values(
                                 "huff_1",
                                 "dcX2bit",
                                 "dc3bit",
                                 "dc3bitNew",
                                 "dcX4bit",
                                 "sym6_32"),
                         [](const testing::TestParamInfo<TestDDSynthDc::ParamType>& info) {
                             auto s = info.param;
                             std::replace( s.begin(), s.end(), '-', '_');
                             return s; });

TEST_P(TestDDSynthDc, GenericDDSynthesisDcTest) {
    EXPECT_TRUE(readPla(tt, fileName));
    extend(tt);

    encodeHuffman(tt);

    const auto ttDD = buildDD(tt, dd);
    EXPECT_TRUE(ttDD.p != nullptr);

    DDSynthesizer synthesizer(tt.nInputs());
    const auto&   qc = synthesizer.synthesize(ttDD, dd);

    EXPECT_TRUE(!qc.empty());

    auto ttSimOut = buildTruthTable(qc);

    EXPECT_TRUE(TruthTable::equal(tt, ttSimOut));

    std::cout << synthesizer.numGate() << "\n";
    std::cout << synthesizer.getExecutionTime() << "\n";
}
