#include "algorithms/synthesis/encoding.hpp"
#include "core/io/pla_parser.hpp"

#include "gtest/gtest.h"

using namespace dd::literals;
using namespace syrec;

class TruthTableExtend: public testing::Test {
protected:
    TruthTable  tt{};
    std::string test_circuits_dir = "./circuits/";

    TruthTable::Cube c00{};
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
        c00 = TruthTable::Cube::fromInteger(0b00U, 2U);
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

TEST_F(TruthTableExtend, Ident2Bit) {
    // create identity truth table

    std::string circIdent2Bit = test_circuits_dir + "Ident2Bit.pla";

    EXPECT_TRUE(read_pla(tt, circIdent2Bit));

    EXPECT_EQ(tt.ioCube().size(), 3U);

    extend(tt);

    EXPECT_EQ(tt.ioCube().size(), 4U);

    auto search = tt.ioCube().find(c00);

    EXPECT_TRUE(search != tt.ioCube().end());

    EXPECT_EQ(search->second, c00);
}

TEST_F(TruthTableExtend, X2Bit) {
    // create X truth table (X gate applied on both the bits)

    std::string circX2Bit = test_circuits_dir + "X2Bit.pla";

    EXPECT_TRUE(read_pla(tt, circX2Bit));

    EXPECT_EQ(tt.ioCube().size(), 3U);

    extend(tt);

    EXPECT_EQ(tt.ioCube().size(), 4U);

    auto search = tt.ioCube().find(c11);

    EXPECT_TRUE(search != tt.ioCube().end());

    EXPECT_EQ(search->second, c00);
}

TEST_F(TruthTableExtend, EXTENDTT) {
    std::string circEXTENDTT = test_circuits_dir + "EXTENDTT.pla";

    EXPECT_TRUE(read_pla(tt, circEXTENDTT));

    EXPECT_EQ(tt.ioCube().size(), 2U);

    extend(tt);

    EXPECT_EQ(tt.ioCube().size(), 8U);

    TruthTable::Cube::Vector outAssigned1{c011, c111};

    for (const auto& in1: outAssigned1) {
        auto search = tt.ioCube().find(in1);

        EXPECT_TRUE(search != tt.ioCube().end());

        EXPECT_EQ(search->second, c111);
    }

    TruthTable::Cube::Vector outAssigned2{c100, c110};

    for (const auto& in2: outAssigned2) {
        auto search = tt.ioCube().find(in2);

        EXPECT_TRUE(search != tt.ioCube().end());

        EXPECT_EQ(search->second, c101);
    }

    TruthTable::Cube::Vector notAssigned{c000, c001, c010, c101};

    for (const auto& in3: notAssigned) {
        auto search = tt.ioCube().find(in3);

        EXPECT_TRUE(search != tt.ioCube().end());

        EXPECT_EQ(search->second, c000);
    }
}
