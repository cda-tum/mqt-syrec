#include "algorithms/synthesis/encoding.hpp"
#include "core/io/pla_parser.hpp"

#include "gtest/gtest.h"

using namespace dd::literals;
using namespace syrec;

class TestHuff: public testing::Test {
protected:
    TruthTable  tt{};
    std::string test_circuits_dir = "./circuits/";

    TruthTable::Cube::Value emptyVal;

    TruthTable::Cube c00{};
    TruthTable::Cube c01{};
    TruthTable::Cube c10{};
    TruthTable::Cube c11{};

    TruthTable::Cube c000{};
    TruthTable::Cube c001{};
    TruthTable::Cube c010{};
    TruthTable::Cube c011{};

    TruthTable::Cube c0_{};
    TruthTable::Cube c0__{};
    TruthTable::Cube c1__{};

    void SetUp() override {
        c00 = TruthTable::Cube::fromInteger(0b00U, 2U);
        c01 = TruthTable::Cube::fromInteger(0b01U, 2U);
        c10 = TruthTable::Cube::fromInteger(0b10U, 2U);
        c11 = TruthTable::Cube::fromInteger(0b11U, 2U);

        c000 = TruthTable::Cube::fromInteger(0b000U, 3U);
        c001 = TruthTable::Cube::fromInteger(0b001U, 3U);
        c010 = TruthTable::Cube::fromInteger(0b010U, 3U);
        c011 = TruthTable::Cube::fromInteger(0b011U, 3U);

        c0_.emplace_back(false);
        c0_.emplace_back(emptyVal);

        c0__.emplace_back(false);
        c0__.emplace_back(emptyVal);
        c0__.emplace_back(emptyVal);

        c1__.emplace_back(true);
        c1__.emplace_back(emptyVal);
        c1__.emplace_back(emptyVal);
    }
};

TEST_F(TestHuff, HUFF1) {
    std::string circHUFF1 = test_circuits_dir + "HUFF1.pla";

    EXPECT_TRUE(read_pla(tt, circHUFF1));

    EXPECT_EQ(tt.ioCube().size(), 3U);

    extend(tt);

    EXPECT_EQ(tt.ioCube().size(), 4U);

    auto search1 = tt.ioCube().find(c00);

    EXPECT_TRUE(search1 != tt.ioCube().end());

    EXPECT_EQ(search1->second, c00);

    EXPECT_EQ(tt.nOutputs(), 2U);

    encodeHuffman(tt);

    EXPECT_EQ(tt.nOutputs(), 2U);

    TruthTable::Cube::Vector encInput{c01, c10};

    for (const auto& in1: encInput) {
        auto search = tt.ioCube().find(in1);

        EXPECT_TRUE(search != tt.ioCube().end());

        EXPECT_EQ(search->second, c0_);
    }

    auto search2 = tt.ioCube().find(c00);

    EXPECT_TRUE(search2 != tt.ioCube().end());

    EXPECT_EQ(search2->second, c10);

    auto search3 = tt.ioCube().find(c11);

    EXPECT_TRUE(search3 != tt.ioCube().end());

    EXPECT_EQ(search3->second, c11);
}

TEST_F(TestHuff, HUFF2) {
    std::string circHUFF2 = test_circuits_dir + "HUFF2.pla";

    EXPECT_TRUE(read_pla(tt, circHUFF2));

    EXPECT_EQ(tt.ioCube().size(), 2U);

    extend(tt);

    EXPECT_EQ(tt.ioCube().size(), 4U);

    auto search1 = tt.ioCube().find(c00);

    EXPECT_TRUE(search1 != tt.ioCube().end());

    EXPECT_EQ(search1->second, c00);

    EXPECT_EQ(tt.nOutputs(), 2U);

    encodeHuffman(tt);

    EXPECT_EQ(tt.nOutputs(), 3U);

    TruthTable::Cube::Vector encInput{c01, c10, c11};

    for (const auto& in1: encInput) {
        auto search2 = tt.ioCube().find(in1);

        EXPECT_TRUE(search2 != tt.ioCube().end());

        EXPECT_EQ(search2->second, c1__);
    }

    auto search3 = tt.ioCube().find(c00);

    EXPECT_TRUE(search3 != tt.ioCube().end());

    EXPECT_EQ(search3->second, c0__);

    EXPECT_EQ(tt.nInputs(), 2U);

    augmentWithConstants(tt);

    EXPECT_EQ(tt.nInputs(), 3U);

    TruthTable::Cube::Vector augInput{c000, c001, c010, c011};

    for (const auto& in2: augInput) {
        auto search4 = tt.ioCube().find(in2);

        EXPECT_TRUE(search4 != tt.ioCube().end());
    }
}
