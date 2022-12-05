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
    std::unique_ptr<dd::Package<>> dd              = std::make_unique<dd::Package<>>(15U);
    std::string                    fileName;

    void SetUp() override {
        fileName = testCircuitsDir + GetParam() + ".pla";
    }
};

INSTANTIATE_TEST_SUITE_P(TestDDSynth, TestDDSynth,
                         testing::Values(
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
                                 "hwb9_65",
                                 "graycode",
                                 "hamming_7",
                                 "mod4096",
                                 "mod8192",
                                 "mod638192",
                                 "14_bit",
                                 "urf1",
                                 "urf2",
                                 "urf3",
                                 "urf4",
                                 "urf5"),
                         [](const testing::TestParamInfo<TestDDSynth::ParamType>& info) {
                             auto s = info.param;
                             std::replace( s.begin(), s.end(), '-', '_');
                             return s; });

TEST_P(TestDDSynth, GenericDDSynthesisTest) {
    EXPECT_TRUE(readPla(tt, fileName));
    extend(tt);

    const auto ttDD = buildDD(tt, dd);
    EXPECT_TRUE(ttDD.p != nullptr);
    DDSynthesizer synthesizer(tt);
    const auto    totalNoBits = synthesizer.getNbits();

    auto qc = std::make_shared<qc::QuantumComputation>(totalNoBits);

    synthesizer.synthesize(ttDD, dd, qc);
    auto const* qcPtr = qc.get();
    const auto& qcDD  = dd::buildFunctionality(qcPtr, dd);
    EXPECT_EQ(ttDD, qcDD);

    std::cout << synthesizer.numGate() << "\n";
    std::cout << synthesizer.getExecutionTime() << "\n";
}
