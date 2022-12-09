#include "algorithms/synthesis/dd_synthesis.hpp"
#include "algorithms/synthesis/encoding.hpp"
#include "core/io/pla_parser.hpp"
#include "dd/FunctionalityConstruction.hpp"

#include "gtest/gtest.h"

using namespace dd::literals;
using namespace syrec;

class TruthTableDD: public testing::Test {
protected:
    std::string                    testCircuitsDir = "./circuits/";
    TruthTable                     tt{};
    std::unique_ptr<dd::Package<>> dd = std::make_unique<dd::Package<>>(3U);
};

TEST_F(TruthTableDD, Ident2Bit) {
    const std::string circIdent2Bit = testCircuitsDir + "ident2Bit.pla";
    EXPECT_TRUE(readPla(tt, circIdent2Bit));
    EXPECT_EQ(tt.nInputs(), 2U);
    EXPECT_EQ(tt.nOutputs(), 2U);

    const auto ttDD = buildDD(tt, dd);
    EXPECT_TRUE(ttDD.p != nullptr);

    EXPECT_EQ(ttDD, dd->makeIdent(2U));
}

TEST_F(TruthTableDD, CNOT) {
    const std::string circCNOT = testCircuitsDir + "cnot.pla";
    EXPECT_TRUE(readPla(tt, circCNOT));
    EXPECT_EQ(tt.nInputs(), 2U);
    EXPECT_EQ(tt.nOutputs(), 2U);

    const auto ttDD = buildDD(tt, dd);
    EXPECT_TRUE(ttDD.p != nullptr);

    // CNOT with target q0 and control q1
    const auto cnot = qc::StandardOperation(2U, 1_pc, 0, qc::X);
    EXPECT_EQ(ttDD, dd::getDD(&cnot, dd));
}

TEST_F(TruthTableDD, SWAP) {
    const std::string circSWAP = testCircuitsDir + "swap.pla";
    EXPECT_TRUE(readPla(tt, circSWAP));
    EXPECT_EQ(tt.nInputs(), 2U);
    EXPECT_EQ(tt.nOutputs(), 2U);

    const auto ttDD = buildDD(tt, dd);
    EXPECT_TRUE(ttDD.p != nullptr);

    // SWAP with q0 and q1
    const auto swap = qc::StandardOperation(2U, {0, 1}, qc::SWAP);
    EXPECT_EQ(ttDD, dd::getDD(&swap, dd));
}

TEST_F(TruthTableDD, Toffoli) {
    const std::string circToffoli = testCircuitsDir + "toffoli.pla";
    EXPECT_TRUE(readPla(tt, circToffoli));
    EXPECT_EQ(tt.nInputs(), 3U);
    EXPECT_EQ(tt.nOutputs(), 3U);

    const auto ttDD = buildDD(tt, dd);
    EXPECT_TRUE(ttDD.p != nullptr);

    // Toffoli with target q0, control q1 and control q2
    const auto toffoli = qc::StandardOperation(3U, {1_pc, 2_pc}, 0, qc::X);
    EXPECT_EQ(ttDD, dd::getDD(&toffoli, dd));
}
