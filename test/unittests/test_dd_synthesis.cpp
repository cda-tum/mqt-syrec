#include "core/truthTable/dd_synthesis.hpp"
#include "core/truthTable/truth_table.hpp"

#include "gtest/gtest.h"

using namespace dd::literals;
using namespace syrec;

class DDSynth: public testing::TestWithParam<std::string> {
protected:
    TruthTable                     tt{};
    std::unique_ptr<dd::Package<>> dd2Bit;
    std::unique_ptr<dd::Package<>> dd3Bit;

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
        dd2Bit = std::make_unique<dd::Package<>>(2U);
        dd3Bit = std::make_unique<dd::Package<>>(3U);

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

TEST_F(DDSynth, CUSTOMTT) {
    tt.insert(c001, c001);
    tt.insert(c010, c100);
    tt.insert(c011, c101);
    tt.insert(c100, c111);
    tt.insert(c101, c010);
    tt.insert(c110, c011);
    tt.insert(c111, c110);

    EXPECT_EQ(tt.ioCube().size(), 7U);

    tt.extend();

    EXPECT_EQ(tt.ioCube().size(), 8U);

    EXPECT_EQ(tt.nInputs(), 3U);
    EXPECT_EQ(tt.nOutputs(), 3U);

    const auto ttDD = tt.buildDD(dd3Bit);
    EXPECT_TRUE(ttDD.p != nullptr);

    EXPECT_NE(ttDD, dd3Bit->makeIdent(3U));

    auto src = ttDD;

    EXPECT_NE(src, dd3Bit->makeIdent(3U));

    algoQ(src, dd3Bit);

    EXPECT_EQ(src, dd3Bit->makeIdent(3U));
}

TEST_F(DDSynth, CNOT) {
    tt.insert(c01, c01);
    tt.insert(c10, c11);
    tt.insert(c11, c10);

    EXPECT_EQ(tt.ioCube().size(), 3U);

    tt.extend();

    EXPECT_EQ(tt.ioCube().size(), 4U);

    EXPECT_EQ(tt.nInputs(), 2U);
    EXPECT_EQ(tt.nOutputs(), 2U);

    const auto ttDD = tt.buildDD(dd2Bit);
    EXPECT_TRUE(ttDD.p != nullptr);

    EXPECT_NE(ttDD, dd2Bit->makeIdent(2U));

    auto src = ttDD;

    EXPECT_NE(src, dd2Bit->makeIdent(2U));

    algoQ(src, dd2Bit);

    EXPECT_EQ(src, dd2Bit->makeIdent(2U));
}

TEST_F(DDSynth, SWAP) {
    tt.insert(c01, c10);
    tt.insert(c10, c01);
    tt.insert(c11, c11);

    EXPECT_EQ(tt.ioCube().size(), 3U);

    tt.extend();

    EXPECT_EQ(tt.ioCube().size(), 4U);

    EXPECT_EQ(tt.nInputs(), 2U);
    EXPECT_EQ(tt.nOutputs(), 2U);

    const auto ttDD = tt.buildDD(dd2Bit);
    EXPECT_TRUE(ttDD.p != nullptr);

    EXPECT_NE(ttDD, dd2Bit->makeIdent(2U));

    auto src = ttDD;

    EXPECT_NE(src, dd2Bit->makeIdent(2U));

    algoQ(src, dd2Bit);

    EXPECT_EQ(src, dd2Bit->makeIdent(2U));
}

TEST_F(DDSynth, Toffoli) {
    tt.insert(c001, c001);
    tt.insert(c010, c010);
    tt.insert(c011, c011);
    tt.insert(c100, c100);
    tt.insert(c101, c101);
    tt.insert(c110, c111);
    tt.insert(c111, c110);

    EXPECT_EQ(tt.ioCube().size(), 7U);

    tt.extend();

    EXPECT_EQ(tt.ioCube().size(), 8U);

    EXPECT_EQ(tt.nInputs(), 3U);
    EXPECT_EQ(tt.nOutputs(), 3U);

    const auto ttDD = tt.buildDD(dd3Bit);
    EXPECT_TRUE(ttDD.p != nullptr);

    EXPECT_NE(ttDD, dd3Bit->makeIdent(3U));

    auto src = ttDD;

    EXPECT_NE(src, dd3Bit->makeIdent(3U));

    algoQ(src, dd3Bit);

    EXPECT_EQ(src, dd3Bit->makeIdent(3U));
}
