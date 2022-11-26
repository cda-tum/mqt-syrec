#include "algorithms/synthesis/dd_synthesis.hpp"
#include "algorithms/synthesis/encoding.hpp"
#include "core/io/pla_parser.hpp"
#include "dd/Simulation.hpp"

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

    std::vector<dd::vEdge> inEdges;
    inEdges.reserve(tt.size());
    std::vector<std::string> outputStrings;
    outputStrings.reserve(tt.size());

    for (auto const& [input, output]: tt) {
        const std::vector<bool> boolCube = input.toBoolVec();
        inEdges.emplace_back(dd->makeBasisState(static_cast<dd::Qubit>(tt.nInputs()), boolCube));

        TruthTable::Cube reverseCube;
        reverseCube.reserve(tt.nInputs());

        for (auto i = static_cast<int>(tt.nInputs()); i >= 0; i--) {
            reverseCube.emplace_back(input[i]);
        }

        auto it = tt.find(reverseCube.toInteger(), tt.nInputs());

        outputStrings.emplace_back(it->second.toString());
    }

    for (auto in = 0U; in < inEdges.size(); in++) {
        const auto vOut       = dd::simulate(&qc, inEdges[in], dd, 1);
        const auto vOutString = vOut.begin()->first;
        EXPECT_TRUE(TruthTable::Cube::equalDcCube(vOutString, outputStrings[in]));
    }

    std::cout << synthesizer.numGate() << "\n";
    std::cout << synthesizer.getExecutionTime() << "\n";
}
