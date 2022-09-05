#include "algorithms/synthesis/dd_synthesis.hpp"
#include "algorithms/synthesis/encoding.hpp"
#include "core/io/pla_parser.hpp"
#include "dd/FunctionalityConstruction.hpp"

#include "gtest/gtest.h"

using namespace dd::literals;
using namespace syrec;

class TestDDSynth: public testing::Test {
protected:
    TruthTable                     tt{};
    std::string                    test_circuits_dir = "./circuits/";
    std::unique_ptr<dd::Package<>> ddSynth           = std::make_unique<dd::Package<>>(6U);
};

TEST_F(TestDDSynth, CUSTOMTT1) {
    std::string circCUSTOMTT1 = test_circuits_dir + "CUSTOMTT1.pla";

    EXPECT_TRUE(read_pla(tt, circCUSTOMTT1));

    EXPECT_EQ(tt.size(), 7U);

    extend(tt);

    EXPECT_EQ(tt.size(), 8U);

    EXPECT_EQ(tt.nInputs(), 3U);
    EXPECT_EQ(tt.nOutputs(), 3U);

    auto ttDD = buildDD(tt, ddSynth);

    EXPECT_TRUE(ttDD.p != nullptr);

    EXPECT_NE(ttDD, ddSynth->makeIdent(3U));

    DDSynthesis quantumCircuit(3U);

    auto trueMatrix  = ddSynth->getMatrix(ttDD);
    auto synthMatrix = ddSynth->getMatrix(dd::buildFunctionality(&quantumCircuit.synthesize(ttDD, ddSynth), ddSynth));

    EXPECT_EQ(trueMatrix, synthMatrix);

    EXPECT_EQ(ttDD, ddSynth->makeIdent(3U));
}

TEST_F(TestDDSynth, CUSTOMTT2) {
    std::string circCUSTOMTT2 = test_circuits_dir + "CUSTOMTT2.pla";

    EXPECT_TRUE(read_pla(tt, circCUSTOMTT2));

    EXPECT_EQ(tt.size(), 7U);

    extend(tt);

    EXPECT_EQ(tt.size(), 8U);

    EXPECT_EQ(tt.nInputs(), 3U);
    EXPECT_EQ(tt.nOutputs(), 3U);

    auto ttDD = buildDD(tt, ddSynth);
    EXPECT_TRUE(ttDD.p != nullptr);

    EXPECT_NE(ttDD, ddSynth->makeIdent(3U));

    DDSynthesis quantumCircuit(3U);

    auto trueMatrix  = ddSynth->getMatrix(ttDD);
    auto synthMatrix = ddSynth->getMatrix(dd::buildFunctionality(&quantumCircuit.synthesize(ttDD, ddSynth), ddSynth));

    EXPECT_EQ(trueMatrix, synthMatrix);

    EXPECT_EQ(ttDD, ddSynth->makeIdent(3U));
}

TEST_F(TestDDSynth, CNOT) {
    std::string circCNOT = test_circuits_dir + "CNOT.pla";

    EXPECT_TRUE(read_pla(tt, circCNOT));

    EXPECT_EQ(tt.size(), 3U);

    extend(tt);

    EXPECT_EQ(tt.size(), 4U);

    EXPECT_EQ(tt.nInputs(), 2U);
    EXPECT_EQ(tt.nOutputs(), 2U);

    auto ttDD = buildDD(tt, ddSynth);

    EXPECT_TRUE(ttDD.p != nullptr);

    EXPECT_NE(ttDD, ddSynth->makeIdent(2U));

    DDSynthesis quantumCircuit(2U);

    auto trueMatrix  = ddSynth->getMatrix(ttDD);
    auto synthMatrix = ddSynth->getMatrix(dd::buildFunctionality(&quantumCircuit.synthesize(ttDD, ddSynth), ddSynth));

    EXPECT_EQ(trueMatrix, synthMatrix);

    EXPECT_EQ(ttDD, ddSynth->makeIdent(2U));
}

TEST_F(TestDDSynth, SWAP) {
    std::string circSWAP = test_circuits_dir + "SWAP.pla";

    EXPECT_TRUE(read_pla(tt, circSWAP));

    EXPECT_EQ(tt.size(), 3U);

    extend(tt);

    EXPECT_EQ(tt.size(), 4U);

    EXPECT_EQ(tt.nInputs(), 2U);
    EXPECT_EQ(tt.nOutputs(), 2U);

    auto ttDD = buildDD(tt, ddSynth);
    EXPECT_TRUE(ttDD.p != nullptr);

    EXPECT_NE(ttDD, ddSynth->makeIdent(2U));

    DDSynthesis quantumCircuit(2U);

    auto trueMatrix  = ddSynth->getMatrix(ttDD);
    auto synthMatrix = ddSynth->getMatrix(dd::buildFunctionality(&quantumCircuit.synthesize(ttDD, ddSynth), ddSynth));

    EXPECT_EQ(trueMatrix, synthMatrix);
    EXPECT_EQ(ttDD, ddSynth->makeIdent(2U));
}

TEST_F(TestDDSynth, Toffoli) {
    std::string circToffoli = test_circuits_dir + "Toffoli.pla";

    EXPECT_TRUE(read_pla(tt, circToffoli));

    EXPECT_EQ(tt.size(), 7U);

    extend(tt);

    EXPECT_EQ(tt.size(), 8U);

    EXPECT_EQ(tt.nInputs(), 3U);
    EXPECT_EQ(tt.nOutputs(), 3U);

    auto ttDD = buildDD(tt, ddSynth);
    EXPECT_TRUE(ttDD.p != nullptr);

    EXPECT_NE(ttDD, ddSynth->makeIdent(3U));

    DDSynthesis quantumCircuit(3U);

    auto trueMatrix  = ddSynth->getMatrix(ttDD);
    auto synthMatrix = ddSynth->getMatrix(dd::buildFunctionality(&quantumCircuit.synthesize(ttDD, ddSynth), ddSynth));

    EXPECT_EQ(trueMatrix, synthMatrix);

    EXPECT_EQ(ttDD, ddSynth->makeIdent(3U));
}

TEST_F(TestDDSynth, BitwiseXor2Bit) {
    std::string circBitwiseXor2Bit = test_circuits_dir + "BitwiseXor2Bit.pla";

    EXPECT_TRUE(read_pla(tt, circBitwiseXor2Bit));

    EXPECT_EQ(tt.size(), 15U);

    extend(tt);

    EXPECT_EQ(tt.size(), 16U);

    EXPECT_EQ(tt.nInputs(), 4U);
    EXPECT_EQ(tt.nOutputs(), 4U);

    auto ttDD = buildDD(tt, ddSynth);
    EXPECT_TRUE(ttDD.p != nullptr);

    EXPECT_NE(ttDD, ddSynth->makeIdent(4U));

    DDSynthesis quantumCircuit(4U);

    auto trueMatrix  = ddSynth->getMatrix(ttDD);
    auto synthMatrix = ddSynth->getMatrix(dd::buildFunctionality(&quantumCircuit.synthesize(ttDD, ddSynth), ddSynth));

    EXPECT_EQ(trueMatrix, synthMatrix);

    EXPECT_EQ(ttDD, ddSynth->makeIdent(4U));
}

TEST_F(TestDDSynth, Adder2Bit) {
    std::string circAdder2Bit = test_circuits_dir + "Adder2Bit.pla";

    EXPECT_TRUE(read_pla(tt, circAdder2Bit));

    EXPECT_EQ(tt.size(), 15U);

    extend(tt);

    EXPECT_EQ(tt.size(), 16U);

    EXPECT_EQ(tt.nInputs(), 4U);
    EXPECT_EQ(tt.nOutputs(), 4U);

    auto ttDD = buildDD(tt, ddSynth);
    EXPECT_TRUE(ttDD.p != nullptr);

    EXPECT_NE(ttDD, ddSynth->makeIdent(4U));

    DDSynthesis quantumCircuit(4U);

    auto trueMatrix  = ddSynth->getMatrix(ttDD);
    auto synthMatrix = ddSynth->getMatrix(dd::buildFunctionality(&quantumCircuit.synthesize(ttDD, ddSynth), ddSynth));

    EXPECT_EQ(trueMatrix, synthMatrix);

    EXPECT_EQ(ttDD, ddSynth->makeIdent(4U));
}

TEST_F(TestDDSynth, Adder3Bit) {
    std::string circAdder3Bit = test_circuits_dir + "Adder3Bit.pla";

    EXPECT_TRUE(read_pla(tt, circAdder3Bit));

    EXPECT_EQ(tt.size(), 64U);

    extend(tt);

    EXPECT_EQ(tt.size(), 64U);

    EXPECT_EQ(tt.nInputs(), 6U);
    EXPECT_EQ(tt.nOutputs(), 6U);

    auto ttDD = buildDD(tt, ddSynth);
    EXPECT_TRUE(ttDD.p != nullptr);

    EXPECT_NE(ttDD, ddSynth->makeIdent(6U));

    DDSynthesis quantumCircuit(6U);

    auto trueMatrix  = ddSynth->getMatrix(ttDD);
    auto synthMatrix = ddSynth->getMatrix(dd::buildFunctionality(&quantumCircuit.synthesize(ttDD, ddSynth), ddSynth));

    EXPECT_EQ(trueMatrix, synthMatrix);
    EXPECT_EQ(ttDD, ddSynth->makeIdent(6U));
}
