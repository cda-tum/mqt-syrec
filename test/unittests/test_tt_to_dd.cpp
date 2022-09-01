#include "algorithms/synthesis/dd_synthesis.hpp"
#include "algorithms/synthesis/encoding.hpp"
#include "core/io/pla_parser.hpp"
#include "dd/FunctionalityConstruction.hpp"

#include "gtest/gtest.h"

using namespace dd::literals;
using namespace syrec;

class TruthTableDD: public testing::Test {
protected:
    std::string                    test_circuits_dir = "./circuits/";
    TruthTable                     tt{};
    std::unique_ptr<dd::Package<>> dd = std::make_unique<dd::Package<>>(3U);
};

TEST_F(TruthTableDD, Ident2Bit) {
    // create identity truth table
    std::string circIdent2Bit = test_circuits_dir + "Ident2Bit.pla";

    EXPECT_TRUE(read_pla(tt, circIdent2Bit));

    EXPECT_EQ(tt.nInputs(), 2U);
    EXPECT_EQ(tt.nOutputs(), 2U);

    extend(tt);

    auto ttDD = buildDD(tt, dd);
    EXPECT_TRUE(ttDD.p != nullptr);

    EXPECT_EQ(ttDD, dd->makeIdent(2U));
}

TEST_F(TruthTableDD, CNOT) {
    // create identity truth table
    std::string circCNOT = test_circuits_dir + "CNOT.pla";

    EXPECT_TRUE(read_pla(tt, circCNOT));

    EXPECT_EQ(tt.nInputs(), 2U);
    EXPECT_EQ(tt.nOutputs(), 2U);

    extend(tt);

    auto ttDD = buildDD(tt, dd);
    EXPECT_TRUE(ttDD.p != nullptr);

    auto qc = qc::QuantumComputation(2U);
    qc.x(0, 1_pc); // CNOT with target q0 and control q1
    EXPECT_EQ(ttDD, dd::buildFunctionality(&qc, dd));
}

TEST_F(TruthTableDD, SWAP) {
    // create identity truth table

    std::string circSWAP = test_circuits_dir + "SWAP.pla";

    EXPECT_TRUE(read_pla(tt, circSWAP));

    EXPECT_EQ(tt.nInputs(), 2U);
    EXPECT_EQ(tt.nOutputs(), 2U);

    extend(tt);

    auto ttDD = buildDD(tt, dd);
    EXPECT_TRUE(ttDD.p != nullptr);

    auto qc = qc::QuantumComputation(2U);
    qc.swap(0, 1);
    EXPECT_EQ(ttDD, dd::buildFunctionality(&qc, dd));
}

TEST_F(TruthTableDD, Toffoli) {
    // create identity truth table

    std::string circToffoli = test_circuits_dir + "Toffoli.pla";

    EXPECT_TRUE(read_pla(tt, circToffoli));

    EXPECT_EQ(tt.nInputs(), 3U);
    EXPECT_EQ(tt.nOutputs(), 3U);

    extend(tt);

    auto ttDD = buildDD(tt, dd);
    EXPECT_TRUE(ttDD.p != nullptr);

    auto qc = qc::QuantumComputation(3U);
    qc.x(0, {1_pc, 2_pc}); // Toffoli with target q0 and controls q1 and q2
    EXPECT_EQ(ttDD, dd::buildFunctionality(&qc, dd));
}
