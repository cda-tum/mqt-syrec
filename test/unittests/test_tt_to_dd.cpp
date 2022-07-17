#include "core/truthTable/truth_table.hpp"
#include "dd/FunctionalityConstruction.hpp"

#include "gtest/gtest.h"

using namespace dd::literals;
using namespace syrec;

class TruthTableDD: public testing::TestWithParam<std::string> {
protected:
    dd::QubitCount                 nqubits = 3U;
    TruthTable                     tt{};
    std::unique_ptr<dd::Package<>> dd;

    TruthTable::Cube c00{};
    TruthTable::Cube c01{};
    TruthTable::Cube c10{};
    TruthTable::Cube c11{};

    TruthTable::Cube c000{};
    TruthTable::Cube c001{};
    TruthTable::Cube c010{};
    TruthTable::Cube c011{};
    TruthTable::Cube c100{};
    TruthTable::Cube c101{};
    TruthTable::Cube c110{};
    TruthTable::Cube c111{};

    void SetUp() override {
        dd = std::make_unique<dd::Package<>>(nqubits);

        c00 = TruthTable::Cube::fromInteger(0b00U, 2U);
        c01 = TruthTable::Cube::fromInteger(0b01U, 2U);
        c10 = TruthTable::Cube::fromInteger(0b10U, 2U);
        c11 = TruthTable::Cube::fromInteger(0b11U, 2U);

        c000 = TruthTable::Cube::fromInteger(0b000U, 3U);
        c001 = TruthTable::Cube::fromInteger(0b001U, 3U);
        c010 = TruthTable::Cube::fromInteger(0b010U, 3U);
        c011 = TruthTable::Cube::fromInteger(0b011U, 3U);
        c100 = TruthTable::Cube::fromInteger(0b100U, 3U);
        c101 = TruthTable::Cube::fromInteger(0b101U, 3U);
        c110 = TruthTable::Cube::fromInteger(0b110U, 3U);
        c111 = TruthTable::Cube::fromInteger(0b111U, 3U);
    }
};

TEST_F(TruthTableDD, Ident2Bit) {
    // create identity truth table
    tt.insert(c00, c00);
    tt.insert(c01, c01);
    tt.insert(c10, c10);
    tt.insert(c11, c11);

    EXPECT_EQ(tt.nInputs(), 2U);
    EXPECT_EQ(tt.nOutputs(), 2U);

    const auto ttDD = tt.buildDD(dd);
    EXPECT_TRUE(ttDD.p != nullptr);

    EXPECT_EQ(ttDD, dd->makeIdent(2U));
}

TEST_F(TruthTableDD, CNOT) {
    // create identity truth table
    tt.insert(c00, c00);
    tt.insert(c01, c01);
    tt.insert(c10, c11);
    tt.insert(c11, c10);

    EXPECT_EQ(tt.nInputs(), 2U);
    EXPECT_EQ(tt.nOutputs(), 2U);

    const auto ttDD = tt.buildDD(dd);
    EXPECT_TRUE(ttDD.p != nullptr);

    auto qc = qc::QuantumComputation(2U);
    qc.x(0, 1_pc); // CNOT with target q0 and control q1
    EXPECT_EQ(ttDD, dd::buildFunctionality(&qc, dd));
}

TEST_F(TruthTableDD, SWAP) {
    // create identity truth table
    tt.insert(c00, c00);
    tt.insert(c01, c10);
    tt.insert(c10, c01);
    tt.insert(c11, c11);

    EXPECT_EQ(tt.nInputs(), 2U);
    EXPECT_EQ(tt.nOutputs(), 2U);

    const auto ttDD = tt.buildDD(dd);
    EXPECT_TRUE(ttDD.p != nullptr);

    auto qc = qc::QuantumComputation(2U);
    qc.swap(0, 1);
    EXPECT_EQ(ttDD, dd::buildFunctionality(&qc, dd));
}

TEST_F(TruthTableDD, Toffoli) {
    // create identity truth table
    tt.insert(c000, c000);
    tt.insert(c001, c001);
    tt.insert(c010, c010);
    tt.insert(c011, c011);
    tt.insert(c100, c100);
    tt.insert(c101, c101);
    tt.insert(c110, c111);
    tt.insert(c111, c110);

    EXPECT_EQ(tt.nInputs(), 3U);
    EXPECT_EQ(tt.nOutputs(), 3U);

    const auto ttDD = tt.buildDD(dd);
    EXPECT_TRUE(ttDD.p != nullptr);

    auto qc = qc::QuantumComputation(3U);
    qc.x(0, {1_pc, 2_pc}); // Toffoli with target q0 and controls q1 and q2
    EXPECT_EQ(ttDD, dd::buildFunctionality(&qc, dd));
}
