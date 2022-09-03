#include "algorithms/synthesis/encoding.hpp"
#include "core/io/pla_parser.hpp"

#include "gtest/gtest.h"

using namespace dd::literals;
using namespace syrec;

class TruthTableExtend: public testing::Test {
protected:
    TruthTable  tt{};
    std::string test_circuits_dir = "./circuits/";
};

TEST_F(TruthTableExtend, Ident2Bit) {
    // create identity truth table
    std::string circIdent2Bit = test_circuits_dir + "Ident2Bit.pla";

    EXPECT_TRUE(read_pla(tt, circIdent2Bit));

    EXPECT_EQ(tt.cubeMap.size(), 3U);

    extend(tt);

    EXPECT_EQ(tt.cubeMap.size(), 4U);

    auto search = tt.findCubeInteger(0b00U, 2U);

    EXPECT_TRUE(search != tt.cubeMap.end());

    EXPECT_TRUE(search->second.equals(0b00U));
}

TEST_F(TruthTableExtend, X2Bit) {
    // create X truth table (X gate applied on both the bits)

    std::string circX2Bit = test_circuits_dir + "X2Bit.pla";

    EXPECT_TRUE(read_pla(tt, circX2Bit));

    EXPECT_EQ(tt.cubeMap.size(), 3U);

    extend(tt);

    EXPECT_EQ(tt.cubeMap.size(), 4U);

    auto search = tt.findCubeInteger(0b11U, 2U);

    EXPECT_TRUE(search != tt.cubeMap.end());

    EXPECT_TRUE(search->second.equals(0b00U));
}

TEST_F(TruthTableExtend, EXTENDTT) {
    std::string circEXTENDTT = test_circuits_dir + "EXTENDTT.pla";

    EXPECT_TRUE(read_pla(tt, circEXTENDTT));

    EXPECT_EQ(tt.cubeMap.size(), 2U);

    extend(tt);

    EXPECT_EQ(tt.cubeMap.size(), 8U);

    std::vector<std::uint64_t> outAssigned1{0b011U, 0b111U};

    for (const auto& in1: outAssigned1) {
        auto search = tt.findCubeInteger(in1, 3U);

        EXPECT_TRUE(search != tt.cubeMap.end());

        EXPECT_TRUE(search->second.equals(0b111U));
    }

    std::vector<std::uint64_t> outAssigned2{0b100U, 0b110U};

    for (const auto& in2: outAssigned2) {
        auto search = tt.findCubeInteger(in2, 3U);

        EXPECT_TRUE(search != tt.cubeMap.end());

        EXPECT_TRUE(search->second.equals(0b101U));
    }

    std::vector<std::uint64_t> notAssigned{0b000U, 0b001U, 0b010U, 0b101U};

    for (const auto& in3: notAssigned) {
        auto search = tt.findCubeInteger(in3, 3U);

        EXPECT_TRUE(search != tt.cubeMap.end());

        EXPECT_TRUE(search->second.equals(0b000U));
    }
}
