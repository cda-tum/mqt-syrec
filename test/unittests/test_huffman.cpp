#include "algorithms/synthesis/encoding.hpp"
#include "core/io/pla_parser.hpp"

#include "gtest/gtest.h"

using namespace dd::literals;
using namespace syrec;

class TestHuff: public testing::Test {
protected:
    TruthTable  tt{};
    std::string test_circuits_dir = "./circuits/";
};

TEST_F(TestHuff, HUFF1) {
    std::string circHUFF1 = test_circuits_dir + "HUFF1.pla";

    EXPECT_TRUE(read_pla(tt, circHUFF1));

    EXPECT_EQ(tt.cubeMap.size(), 3U);

    extend(tt);

    EXPECT_EQ(tt.cubeMap.size(), 4U);

    auto search1 = tt.findCubeInteger(0b00U, 2U);

    EXPECT_TRUE(search1 != tt.cubeMap.end());

    EXPECT_TRUE(search1->second.equals(0b00U));

    EXPECT_EQ(tt.nOutputs(), 2U);

    encodeHuffman(tt);

    EXPECT_EQ(tt.nOutputs(), 2U);

    std::vector<std::uint64_t> encInput{0b01U, 0b10U};

    for (const auto& in1: encInput) {
        auto search = tt.findCubeInteger(in1, 2U);
        EXPECT_TRUE(search != tt.cubeMap.end());
        EXPECT_TRUE(search->second.equals(std::string("0-")));
    }

    auto search2 = tt.findCubeInteger(0b00U, 2U);

    EXPECT_TRUE(search2 != tt.cubeMap.end());

    EXPECT_TRUE(search2->second.equals(0b10U));

    auto search3 = tt.findCubeInteger(0b11U, 2U);

    EXPECT_TRUE(search3 != tt.cubeMap.end());

    EXPECT_TRUE(search3->second.equals(0b11U));
}

TEST_F(TestHuff, HUFF2) {
    std::string circHUFF2 = test_circuits_dir + "HUFF2.pla";

    EXPECT_TRUE(read_pla(tt, circHUFF2));

    EXPECT_EQ(tt.cubeMap.size(), 2U);

    extend(tt);

    EXPECT_EQ(tt.cubeMap.size(), 4U);

    auto search1 = tt.findCubeInteger(0b00U, 2U);

    EXPECT_TRUE(search1 != tt.cubeMap.end());

    EXPECT_TRUE(search1->second.equals(0b00U));

    EXPECT_EQ(tt.nOutputs(), 2U);

    encodeHuffman(tt);

    EXPECT_EQ(tt.nOutputs(), 3U);

    std::vector<std::uint64_t> encInput{0b01U, 0b10U, 0b11U};

    for (const auto& in1: encInput) {
        auto search2 = tt.findCubeInteger(in1, 2U);
        EXPECT_TRUE(search2 != tt.cubeMap.end());

        EXPECT_TRUE(search2->second.equals(std::string("1--")));
    }

    auto search3 = tt.findCubeInteger(0b00U, 2U);

    EXPECT_TRUE(search3 != tt.cubeMap.end());

    EXPECT_TRUE(search3->second.equals(std::string("0--")));

    EXPECT_EQ(tt.nInputs(), 2U);

    augmentWithConstants(tt);

    EXPECT_EQ(tt.nInputs(), 3U);

    std::vector<std::uint64_t> augInput{0b000U, 0b001U, 0b010U, 0b011U};

    for (const auto& in2: augInput) {
        auto search4 = tt.findCubeInteger(in2, 3U);

        EXPECT_TRUE(search4 != tt.cubeMap.end());
    }
}
