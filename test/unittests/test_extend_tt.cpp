#include "algorithms/synthesis/encoding.hpp"
#include "core/io/pla_parser.hpp"

#include "gtest/gtest.h"

using namespace qc::literals;
using namespace syrec;

class TruthTableExtend: public testing::Test {
protected:
    TruthTable  tt{};
    std::string testCircuitsDir = "./circuits/";
};

TEST_F(TruthTableExtend, Max) {
    std::string circMax = testCircuitsDir + "max.pla";

    // the max.pla consist of 64 inputs. Currently, functions with less than 63 inputs are supported.
    EXPECT_ANY_THROW(readPla(tt, circMax));
}

TEST_F(TruthTableExtend, Ident2Bit) {
    // create identity truth table
    std::string circIdent2Bit = testCircuitsDir + "ident2Bit.pla";

    EXPECT_TRUE(readPla(tt, circIdent2Bit));

    EXPECT_EQ(tt.size(), 4U);

    auto search = tt.find(0b00U, 2U);

    EXPECT_TRUE(search != tt.end());

    EXPECT_TRUE(search->second.equals(0b00U, 2U));
}

TEST_F(TruthTableExtend, X2Bit) {
    // create X truth table (X gate applied on both the bits)

    std::string circX2Bit = testCircuitsDir + "x2Bit.pla";

    EXPECT_TRUE(readPla(tt, circX2Bit));

    EXPECT_EQ(tt.size(), 4U);

    auto search = tt.find(0b11U, 2U);

    EXPECT_TRUE(search != tt.end());

    EXPECT_TRUE(search->second.equals(0b00U, 2U));
}

TEST_F(TruthTableExtend, EXTENDTT) {
    std::string circEXTENDTT = testCircuitsDir + "extend.pla";

    EXPECT_TRUE(readPla(tt, circEXTENDTT));

    EXPECT_EQ(tt.size(), 8U);

    std::vector<std::uint64_t> outAssigned1{0b011U, 0b111U};

    for (const auto& in1: outAssigned1) {
        auto search = tt.find(in1, 3U);

        EXPECT_TRUE(search != tt.end());

        EXPECT_TRUE(search->second.equals(0b111U, 3U));
    }

    std::vector<std::uint64_t> outAssigned2{0b100U, 0b110U};

    for (const auto& in2: outAssigned2) {
        auto search = tt.find(in2, 3U);

        EXPECT_TRUE(search != tt.end());

        EXPECT_TRUE(search->second.equals(0b101U, 3U));
    }

    std::vector<std::uint64_t> notAssigned{0b000U, 0b001U, 0b010U, 0b101U};

    for (const auto& in3: notAssigned) {
        auto search = tt.find(in3, 3U);

        EXPECT_TRUE(search != tt.end());

        EXPECT_TRUE(search->second.equals(0b000U, 3U));
    }
}
