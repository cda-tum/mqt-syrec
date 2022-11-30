
#include "algorithms/synthesis/encoding.hpp"
#include "core/io/pla_parser.hpp"

#include "gtest/gtest.h"

using namespace dd::literals;
using namespace syrec;

class TestHuff: public testing::Test {
protected:
    TruthTable  tt{};
    std::string testCircuitsDir = "./circuits/";
};

TEST_F(TestHuff, Ident2Bit) {
    const std::string circIdent2Bit = testCircuitsDir + "ident2Bit.pla";

    EXPECT_TRUE(readPla(tt, circIdent2Bit));

    EXPECT_EQ(tt.size(), 3U);

    extend(tt);

    EXPECT_EQ(tt.size(), 4U);

    auto search = tt.find(0b00U, 2U);

    EXPECT_TRUE(search != tt.end());

    EXPECT_TRUE(search->second.equals(0b00U, 2U));

    const TruthTable ttExtend(tt);

    encodeHuffman(tt);

    ASSERT_TRUE(tt == ttExtend);
}

TEST_F(TestHuff, HUFF1) {
    const std::string circHUFF1 = testCircuitsDir + "huff_1.pla";

    EXPECT_TRUE(readPla(tt, circHUFF1));

    EXPECT_EQ(tt.size(), 3U);

    extend(tt);

    EXPECT_EQ(tt.size(), 4U);

    auto search1 = tt.find(0b00U, 2U);

    EXPECT_TRUE(search1 != tt.end());

    EXPECT_TRUE(search1->second.equals(0b00U, 2U));

    EXPECT_EQ(tt.nOutputs(), 2U);

    encodeHuffman(tt);

    EXPECT_EQ(tt.nOutputs(), 2U);

    const std::vector<std::uint64_t> encInput{0b01U, 0b10U};

    for (const auto& in1: encInput) {
        auto search = tt.find(in1, 2U);
        EXPECT_TRUE(search != tt.end());
        EXPECT_TRUE(search->second.equals("1-"));
    }

    auto search2 = tt.find(0b00U, 2U);

    EXPECT_TRUE(search2 != tt.end());

    EXPECT_TRUE(search2->second.equals(0b00U, 2U));

    auto search3 = tt.find(0b11U, 2U);

    EXPECT_TRUE(search3 != tt.end());

    EXPECT_TRUE(search3->second.equals(0b01U, 2U));
}

TEST_F(TestHuff, HUFF2) {
    const std::string circHUFF2 = testCircuitsDir + "huff_2.pla";

    EXPECT_TRUE(readPla(tt, circHUFF2));

    EXPECT_EQ(tt.size(), 2U);

    extend(tt);

    EXPECT_EQ(tt.size(), 4U);

    auto search1 = tt.find(0b00U, 2U);

    EXPECT_TRUE(search1 != tt.end());

    EXPECT_TRUE(search1->second.equals(0b00U, 2U));

    EXPECT_EQ(tt.nOutputs(), 2U);

    encodeHuffman(tt);

    EXPECT_EQ(tt.nOutputs(), 3U);

    const std::vector<std::uint64_t> encInput{0b01U, 0b10U, 0b11U};

    for (const auto& in1: encInput) {
        auto search2 = tt.find(in1, 2U);
        EXPECT_TRUE(search2 != tt.end());

        EXPECT_TRUE(search2->second.equals("1--"));
    }

    auto search3 = tt.find(0b00U, 2U);

    EXPECT_TRUE(search3 != tt.end());

    EXPECT_TRUE(search3->second.equals("0--"));

    EXPECT_EQ(tt.nInputs(), 2U);

    augmentWithConstants(tt);

    EXPECT_EQ(tt.nInputs(), 3U);

    const std::vector<std::uint64_t> augInput{0b000U, 0b001U, 0b010U, 0b011U};

    for (const auto& in2: augInput) {
        auto search4 = tt.find(in2, 3U);

        EXPECT_TRUE(search4 != tt.end());
    }
}
