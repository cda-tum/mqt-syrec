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
    TruthTable                     ttExpected{};
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
    extend(tt);

    TruthTable const ttOri(tt); //unchanged Truth Table.

    //n -> No. of primary inputs.
    //m -> No. of primary outputs.
    //k1 -> Minimum no. of additional lines required.
    //codewords -> Output patterns with the respective codewords.
    //totalNoBits -> Total no. of bits required to create the circuit
    // r -> additional variables/bits required to decode r MSB bits of the circuit.

    const auto n = tt.nInputs();
    const auto m = tt.nOutputs();

    const auto [codewords, k1] = encodeHuffman(tt);

    const auto totalNoBits = std::max(n, m + k1);
    const auto r           = totalNoBits - tt.nOutputs();

    augmentWithConstants(tt, totalNoBits);

    const auto ttDD = buildDD(tt, dd);
    EXPECT_TRUE(ttDD.p != nullptr);

    DDSynthesizer synthesizer(totalNoBits);

    // codewords, r, m required for decoding purposes.
    const auto& qc = synthesizer.synthesize(ttDD, codewords, r, m, dd);

    //Expected Truth table
    for (const auto& [input, output]: ttOri) {
        auto inCube(input);
        for (auto i = 0U; i < (totalNoBits - input.size()); i++) {
            inCube.insertZero();
        }
        TruthTable::Cube outCube(output);
        outCube.resize(totalNoBits);
        ttExpected.try_emplace(inCube, outCube);
    }

    buildTruthTable(qc, ttqc);

    EXPECT_TRUE(TruthTable::equal(ttExpected, ttqc));

    std::cout << totalNoBits << "\n";
    std::cout << synthesizer.numGate() << "\n";
    std::cout << synthesizer.getExecutionTime() << "\n";
}
