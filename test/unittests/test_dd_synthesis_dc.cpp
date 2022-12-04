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
    TruthTable                     ttqc{};
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
                                 "root",
                                 "sqr",
                                 "sqrt8",
                                 "life",
                                 "aludc",
                                 "minialu",
                                 "dc2",
                                 "dk27",
                                 "pm1",
                                 "majority",
                                 "max10",
                                 "4gt10",
                                 "counter",
                                 "4gt5",
                                 "4mod5",
                                 "decode24",
                                 "mod10",
                                 "asym",
                                 "decode24e",
                                 "radd",
                                 "radd8",
                                 "rd53",
                                 "rd84",
                                 "dist",
                                 "mlp4",
                                 "wim",
                                 "z4",
                                 "z4ml",
                                 "misex"),
                         [](const testing::TestParamInfo<TestDDSynthDc::ParamType>& info) {
                             auto s = info.param;
                             std::replace( s.begin(), s.end(), '-', '_');
                             return s; });

TEST_P(TestDDSynthDc, GenericDDSynthesisDcTest) {
    EXPECT_TRUE(readPla(tt, fileName));
    extend(tt);

    encodeHuffman(tt);

    augmentWithConstants(tt);

    const auto ttDD = buildDD(tt, dd);
    EXPECT_TRUE(ttDD.p != nullptr);

    DDSynthesizer synthesizer(tt.nInputs());
    const auto&   qc = synthesizer.synthesize(ttDD, dd);

    buildTruthTable(qc, ttqc);

    EXPECT_TRUE(TruthTable::equal(tt, ttqc));

    std::cout << synthesizer.numGate() << "\n";
    std::cout << synthesizer.getExecutionTime() << "\n";
}
