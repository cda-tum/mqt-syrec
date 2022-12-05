#include "algorithms/simulation/circuit_to_truthtable.hpp"
#include "algorithms/synthesis/dd_synthesis.hpp"
#include "algorithms/synthesis/encoding.hpp"
#include "core/io/pla_parser.hpp"

#include "gtest/gtest.h"

using namespace dd::literals;
using namespace syrec;

class TestDDSynthDc: public testing::TestWithParam<std::string> {
protected:
    TruthTable  tt{};
    TruthTable  ttExpected{};
    TruthTable  ttqc{};
    std::string testCircuitsDir = "./circuits/";
    std::string fileName;

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
                                 "sym6_32",
                                 "huff_2",
                                 "huff_21",
                                 "huff_31",
                                 "huff_32",
                                 "huff_33",
                                 "huff_34",
                                 "rd32_19",
                                 "c17",
                                 "con1",
                                 "sqr",
                                 "aludc",
                                 "minialu",
                                 "majority",
                                 "4gt10",
                                 "counter",
                                 "4gt5",
                                 "4mod5",
                                 "decode24",
                                 "decode24e",
                                 "rd53",
                                 "wim",
                                 "z4",
                                 "z4ml"),
                         [](const testing::TestParamInfo<TestDDSynthDc::ParamType>& info) {
                             auto s = info.param;
                             std::replace( s.begin(), s.end(), '-', '_');
                             return s; });

TEST_P(TestDDSynthDc, GenericDDSynthesisDcTest) {
    EXPECT_TRUE(readPla(tt, fileName));

    DDSynthesizer synthesizer(tt);
    const auto&   qc          = synthesizer.synthesize(tt);
    const auto    totalNoBits = synthesizer.getNbits();
    auto const*   qcPtr       = qc.get();

    // generate the complete truth table.
    TruthTable ttExpected{tt};
    completeTruthTable(ttExpected, totalNoBits);

    buildTruthTable(qcPtr, ttqc);

    EXPECT_TRUE(TruthTable::equal(ttExpected, ttqc));

    std::cout << synthesizer.numGate() << "\n";
    std::cout << synthesizer.getExecutionTime() << "\n";
}
