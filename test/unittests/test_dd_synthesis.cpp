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
    std::unique_ptr<dd::Package<>> dd4Bit;
    std::unique_ptr<dd::Package<>> dd6Bit;

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

    TruthTable::Cube c0000{};
    TruthTable::Cube c0001{};
    TruthTable::Cube c0010{};
    TruthTable::Cube c0011{};
    TruthTable::Cube c0100{};
    TruthTable::Cube c0101{};
    TruthTable::Cube c0110{};
    TruthTable::Cube c0111{};
    TruthTable::Cube c1000{};
    TruthTable::Cube c1001{};
    TruthTable::Cube c1010{};
    TruthTable::Cube c1011{};
    TruthTable::Cube c1100{};
    TruthTable::Cube c1101{};
    TruthTable::Cube c1110{};
    TruthTable::Cube c1111{};

    TruthTable::Cube c000000{};
    TruthTable::Cube c000001{};
    TruthTable::Cube c000010{};
    TruthTable::Cube c000011{};
    TruthTable::Cube c000100{};
    TruthTable::Cube c000101{};
    TruthTable::Cube c000110{};
    TruthTable::Cube c000111{};
    TruthTable::Cube c001000{};
    TruthTable::Cube c001001{};
    TruthTable::Cube c001010{};
    TruthTable::Cube c001011{};
    TruthTable::Cube c001100{};
    TruthTable::Cube c001101{};
    TruthTable::Cube c001110{};
    TruthTable::Cube c001111{};

    TruthTable::Cube c010000{};
    TruthTable::Cube c010001{};
    TruthTable::Cube c010010{};
    TruthTable::Cube c010011{};
    TruthTable::Cube c010100{};
    TruthTable::Cube c010101{};
    TruthTable::Cube c010110{};
    TruthTable::Cube c010111{};
    TruthTable::Cube c011000{};
    TruthTable::Cube c011001{};
    TruthTable::Cube c011010{};
    TruthTable::Cube c011011{};
    TruthTable::Cube c011100{};
    TruthTable::Cube c011101{};
    TruthTable::Cube c011110{};
    TruthTable::Cube c011111{};

    TruthTable::Cube c100000{};
    TruthTable::Cube c100001{};
    TruthTable::Cube c100010{};
    TruthTable::Cube c100011{};
    TruthTable::Cube c100100{};
    TruthTable::Cube c100101{};
    TruthTable::Cube c100110{};
    TruthTable::Cube c100111{};
    TruthTable::Cube c101000{};
    TruthTable::Cube c101001{};
    TruthTable::Cube c101010{};
    TruthTable::Cube c101011{};
    TruthTable::Cube c101100{};
    TruthTable::Cube c101101{};
    TruthTable::Cube c101110{};
    TruthTable::Cube c101111{};

    TruthTable::Cube c110000{};
    TruthTable::Cube c110001{};
    TruthTable::Cube c110010{};
    TruthTable::Cube c110011{};
    TruthTable::Cube c110100{};
    TruthTable::Cube c110101{};
    TruthTable::Cube c110110{};
    TruthTable::Cube c110111{};
    TruthTable::Cube c111000{};
    TruthTable::Cube c111001{};
    TruthTable::Cube c111010{};
    TruthTable::Cube c111011{};
    TruthTable::Cube c111100{};
    TruthTable::Cube c111101{};
    TruthTable::Cube c111110{};
    TruthTable::Cube c111111{};

    void SetUp() override {
        dd2Bit = std::make_unique<dd::Package<>>(2U);
        dd3Bit = std::make_unique<dd::Package<>>(3U);
        dd4Bit = std::make_unique<dd::Package<>>(4U);
        dd6Bit = std::make_unique<dd::Package<>>(6U);

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

        c0000 = TruthTable::Cube::fromInteger(0b0000U, 4U);
        c0001 = TruthTable::Cube::fromInteger(0b0001U, 4U);
        c0010 = TruthTable::Cube::fromInteger(0b0010U, 4U);
        c0011 = TruthTable::Cube::fromInteger(0b0011U, 4U);
        c0100 = TruthTable::Cube::fromInteger(0b0100U, 4U);
        c0101 = TruthTable::Cube::fromInteger(0b0101U, 4U);
        c0110 = TruthTable::Cube::fromInteger(0b0110U, 4U);
        c0111 = TruthTable::Cube::fromInteger(0b0111U, 4U);
        c1000 = TruthTable::Cube::fromInteger(0b1000U, 4U);
        c1001 = TruthTable::Cube::fromInteger(0b1001U, 4U);
        c1010 = TruthTable::Cube::fromInteger(0b1010U, 4U);
        c1011 = TruthTable::Cube::fromInteger(0b1011U, 4U);
        c1100 = TruthTable::Cube::fromInteger(0b1100U, 4U);
        c1101 = TruthTable::Cube::fromInteger(0b1101U, 4U);
        c1110 = TruthTable::Cube::fromInteger(0b1110U, 4U);
        c1111 = TruthTable::Cube::fromInteger(0b1111U, 4U);

        c000000 = TruthTable::Cube::fromInteger(0b000000U, 6U);
        c000001 = TruthTable::Cube::fromInteger(0b000001U, 6U);
        c000010 = TruthTable::Cube::fromInteger(0b000010U, 6U);
        c000011 = TruthTable::Cube::fromInteger(0b000011U, 6U);
        c000100 = TruthTable::Cube::fromInteger(0b000100U, 6U);
        c000101 = TruthTable::Cube::fromInteger(0b000101U, 6U);
        c000110 = TruthTable::Cube::fromInteger(0b000110U, 6U);
        c000111 = TruthTable::Cube::fromInteger(0b000111U, 6U);
        c001000 = TruthTable::Cube::fromInteger(0b001000U, 6U);
        c001001 = TruthTable::Cube::fromInteger(0b001001U, 6U);
        c001010 = TruthTable::Cube::fromInteger(0b001010U, 6U);
        c001011 = TruthTable::Cube::fromInteger(0b001011U, 6U);
        c001100 = TruthTable::Cube::fromInteger(0b001100U, 6U);
        c001101 = TruthTable::Cube::fromInteger(0b001101U, 6U);
        c001110 = TruthTable::Cube::fromInteger(0b001110U, 6U);
        c001111 = TruthTable::Cube::fromInteger(0b001111U, 6U);

        c010000 = TruthTable::Cube::fromInteger(0b010000U, 6U);
        c010001 = TruthTable::Cube::fromInteger(0b010001U, 6U);
        c010010 = TruthTable::Cube::fromInteger(0b010010U, 6U);
        c010011 = TruthTable::Cube::fromInteger(0b010011U, 6U);
        c010100 = TruthTable::Cube::fromInteger(0b010100U, 6U);
        c010101 = TruthTable::Cube::fromInteger(0b010101U, 6U);
        c010110 = TruthTable::Cube::fromInteger(0b010110U, 6U);
        c010111 = TruthTable::Cube::fromInteger(0b010111U, 6U);
        c011000 = TruthTable::Cube::fromInteger(0b011000U, 6U);
        c011001 = TruthTable::Cube::fromInteger(0b011001U, 6U);
        c011010 = TruthTable::Cube::fromInteger(0b011010U, 6U);
        c011011 = TruthTable::Cube::fromInteger(0b011011U, 6U);
        c011100 = TruthTable::Cube::fromInteger(0b011100U, 6U);
        c011101 = TruthTable::Cube::fromInteger(0b011101U, 6U);
        c011110 = TruthTable::Cube::fromInteger(0b011110U, 6U);
        c011111 = TruthTable::Cube::fromInteger(0b011111U, 6U);

        c100000 = TruthTable::Cube::fromInteger(0b100000U, 6U);
        c100001 = TruthTable::Cube::fromInteger(0b100001U, 6U);
        c100010 = TruthTable::Cube::fromInteger(0b100010U, 6U);
        c100011 = TruthTable::Cube::fromInteger(0b100011U, 6U);
        c100100 = TruthTable::Cube::fromInteger(0b100100U, 6U);
        c100101 = TruthTable::Cube::fromInteger(0b100101U, 6U);
        c100110 = TruthTable::Cube::fromInteger(0b100110U, 6U);
        c100111 = TruthTable::Cube::fromInteger(0b100111U, 6U);
        c101000 = TruthTable::Cube::fromInteger(0b101000U, 6U);
        c101001 = TruthTable::Cube::fromInteger(0b101001U, 6U);
        c101010 = TruthTable::Cube::fromInteger(0b101010U, 6U);
        c101011 = TruthTable::Cube::fromInteger(0b101011U, 6U);
        c101100 = TruthTable::Cube::fromInteger(0b101100U, 6U);
        c101101 = TruthTable::Cube::fromInteger(0b101101U, 6U);
        c101110 = TruthTable::Cube::fromInteger(0b101110U, 6U);
        c101111 = TruthTable::Cube::fromInteger(0b101111U, 6U);

        c110000 = TruthTable::Cube::fromInteger(0b110000U, 6U);
        c110001 = TruthTable::Cube::fromInteger(0b110001U, 6U);
        c110010 = TruthTable::Cube::fromInteger(0b110010U, 6U);
        c110011 = TruthTable::Cube::fromInteger(0b110011U, 6U);
        c110100 = TruthTable::Cube::fromInteger(0b110100U, 6U);
        c110101 = TruthTable::Cube::fromInteger(0b110101U, 6U);
        c110110 = TruthTable::Cube::fromInteger(0b110110U, 6U);
        c110111 = TruthTable::Cube::fromInteger(0b110111U, 6U);
        c111000 = TruthTable::Cube::fromInteger(0b111000U, 6U);
        c111001 = TruthTable::Cube::fromInteger(0b111001U, 6U);
        c111010 = TruthTable::Cube::fromInteger(0b111010U, 6U);
        c111011 = TruthTable::Cube::fromInteger(0b111011U, 6U);
        c111100 = TruthTable::Cube::fromInteger(0b111100U, 6U);
        c111101 = TruthTable::Cube::fromInteger(0b111101U, 6U);
        c111110 = TruthTable::Cube::fromInteger(0b111110U, 6U);
        c111111 = TruthTable::Cube::fromInteger(0b111111U, 6U);
    }
};

TEST_F(DDSynth, CUSTOMTT1) {
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

TEST_F(DDSynth, CUSTOMTT2) {
    tt.insert(c001, c011);
    tt.insert(c010, c001);
    tt.insert(c011, c010);
    tt.insert(c100, c100);
    tt.insert(c101, c111);
    tt.insert(c110, c101);
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

TEST_F(DDSynth, BitwiseXor2Bit) {
    tt.insert(c0001, c0001);
    tt.insert(c0010, c0010);
    tt.insert(c0011, c0011);

    tt.insert(c0100, c0101);
    tt.insert(c0101, c0100);
    tt.insert(c0110, c0111);
    tt.insert(c0111, c0110);

    tt.insert(c1000, c1010);
    tt.insert(c1001, c1011);
    tt.insert(c1010, c1000);
    tt.insert(c1011, c1001);

    tt.insert(c1100, c1111);
    tt.insert(c1101, c1110);
    tt.insert(c1110, c1101);
    tt.insert(c1111, c1100);

    EXPECT_EQ(tt.ioCube().size(), 15U);

    tt.extend();

    EXPECT_EQ(tt.ioCube().size(), 16U);

    EXPECT_EQ(tt.nInputs(), 4U);
    EXPECT_EQ(tt.nOutputs(), 4U);

    const auto ttDD = tt.buildDD(dd4Bit);
    EXPECT_TRUE(ttDD.p != nullptr);

    EXPECT_NE(ttDD, dd4Bit->makeIdent(4U));

    auto src = ttDD;

    EXPECT_NE(src, dd4Bit->makeIdent(4U));

    algoQ(src, dd4Bit);

    EXPECT_EQ(src, dd4Bit->makeIdent(4U));
}

TEST_F(DDSynth, Adder2Bit) {
    tt.insert(c0001, c0001);
    tt.insert(c0010, c0010);
    tt.insert(c0011, c0011);

    tt.insert(c0100, c0101);
    tt.insert(c0101, c0110);
    tt.insert(c0110, c0111);
    tt.insert(c0111, c0100);

    tt.insert(c1000, c1010);
    tt.insert(c1001, c1011);
    tt.insert(c1010, c1000);
    tt.insert(c1011, c1001);

    tt.insert(c1100, c1111);
    tt.insert(c1101, c1100);
    tt.insert(c1110, c1101);
    tt.insert(c1111, c1110);

    EXPECT_EQ(tt.ioCube().size(), 15U);

    tt.extend();

    EXPECT_EQ(tt.ioCube().size(), 16U);

    EXPECT_EQ(tt.nInputs(), 4U);
    EXPECT_EQ(tt.nOutputs(), 4U);

    const auto ttDD = tt.buildDD(dd4Bit);
    EXPECT_TRUE(ttDD.p != nullptr);

    EXPECT_NE(ttDD, dd4Bit->makeIdent(4U));

    auto src = ttDD;

    EXPECT_NE(src, dd4Bit->makeIdent(4U));

    algoQ(src, dd4Bit);

    EXPECT_EQ(src, dd4Bit->makeIdent(4U));
}

TEST_F(DDSynth, Adder3Bit) {
    tt.insert(c000000, c000000);
    tt.insert(c000001, c000111);
    tt.insert(c000010, c000110);
    tt.insert(c000011, c000101);
    tt.insert(c000100, c000100);
    tt.insert(c000101, c000011);
    tt.insert(c000110, c000010);
    tt.insert(c000111, c000001);
    tt.insert(c001000, c001001);
    tt.insert(c001001, c001000);
    tt.insert(c001010, c001111);
    tt.insert(c001011, c001110);
    tt.insert(c001100, c001101);
    tt.insert(c001101, c001100);
    tt.insert(c001110, c001011);
    tt.insert(c001111, c001010);

    tt.insert(c010000, c010010);
    tt.insert(c010001, c010001);
    tt.insert(c010010, c010000);
    tt.insert(c010011, c010111);
    tt.insert(c010100, c010110);
    tt.insert(c010101, c010101);
    tt.insert(c010110, c010100);
    tt.insert(c010111, c010011);
    tt.insert(c011000, c011011);
    tt.insert(c011001, c011010);
    tt.insert(c011010, c011001);
    tt.insert(c011011, c011000);
    tt.insert(c011100, c011111);
    tt.insert(c011101, c011110);
    tt.insert(c011110, c011101);
    tt.insert(c011111, c011100);

    tt.insert(c100000, c100100);
    tt.insert(c100001, c100011);
    tt.insert(c100010, c100010);
    tt.insert(c100011, c100001);
    tt.insert(c100100, c100000);
    tt.insert(c100101, c100111);
    tt.insert(c100110, c100110);
    tt.insert(c100111, c100101);
    tt.insert(c101000, c101101);
    tt.insert(c101001, c101100);
    tt.insert(c101010, c101011);
    tt.insert(c101011, c101010);
    tt.insert(c101100, c101001);
    tt.insert(c101101, c101000);
    tt.insert(c101110, c101111);
    tt.insert(c101111, c101110);

    tt.insert(c110000, c110110);
    tt.insert(c110001, c110101);
    tt.insert(c110010, c110100);
    tt.insert(c110011, c110011);
    tt.insert(c110100, c110010);
    tt.insert(c110101, c110001);
    tt.insert(c110110, c110000);
    tt.insert(c110111, c110111);
    tt.insert(c111000, c111111);
    tt.insert(c111001, c111110);
    tt.insert(c111010, c111101);
    tt.insert(c111011, c111100);
    tt.insert(c111100, c111011);
    tt.insert(c111101, c111010);
    tt.insert(c111110, c111001);
    tt.insert(c111111, c111000);

    EXPECT_EQ(tt.ioCube().size(), 64U);

    tt.extend();

    EXPECT_EQ(tt.ioCube().size(), 64U);

    EXPECT_EQ(tt.nInputs(), 6U);
    EXPECT_EQ(tt.nOutputs(), 6U);

    const auto ttDD = tt.buildDD(dd6Bit);
    EXPECT_TRUE(ttDD.p != nullptr);

    EXPECT_NE(ttDD, dd6Bit->makeIdent(6U));

    auto src = ttDD;

    EXPECT_NE(src, dd6Bit->makeIdent(6U));

    algoQ(src, dd6Bit);

    EXPECT_EQ(src, dd6Bit->makeIdent(6U));
}
