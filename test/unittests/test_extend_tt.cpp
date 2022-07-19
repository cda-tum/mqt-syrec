#include "core/truthTable/truth_table.hpp"

#include "gtest/gtest.h"

using namespace dd::literals;
using namespace syrec;

class TruthTableExtend: public testing::TestWithParam<std::string> {
protected:
    dd::QubitCount                 nqubits = 3U;
    TruthTable                     tt{};
    std::unique_ptr<dd::Package<>> dd;

    TruthTable::Cube::Value emptyVal;

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

    TruthTable::Cube c_11{};
    TruthTable::Cube c1_0{};

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

        c_11.emplace_back(emptyVal);
        c_11.emplace_back(true);
        c_11.emplace_back(true);

        c1_0.emplace_back(true);
        c1_0.emplace_back(emptyVal);
        c1_0.emplace_back(false);
    }
};

TEST_F(TruthTableExtend, Ident2Bit) {
    // create identity truth table

    tt.insert(c01, c01);
    tt.insert(c10, c10);
    tt.insert(c11, c11);

    EXPECT_EQ(tt.ioCube().size(), 3U);

    tt.extend();

    EXPECT_EQ(tt.ioCube().size(), 4U);

    auto search = tt.ioCube().find(c00);

    EXPECT_TRUE(search != tt.ioCube().end());

    EXPECT_EQ(search->second, c00);
}

TEST_F(TruthTableExtend, X2Bit) {
    // create identity X truth table (X gate applied on both the bits)

    tt.insert(c00, c11);
    tt.insert(c01, c10);
    tt.insert(c10, c01);

    EXPECT_EQ(tt.ioCube().size(), 3U);

    tt.extend();

    EXPECT_EQ(tt.ioCube().size(), 4U);

    auto search = tt.ioCube().find(c11);

    EXPECT_TRUE(search != tt.ioCube().end());

    EXPECT_EQ(search->second, c00);
}

TEST_F(TruthTableExtend, CUSTOMTT) {
    tt.insert(c_11, c111);

    tt.insert(c1_0, c101);

    EXPECT_EQ(tt.ioCube().size(), 2U);

    tt.extend();

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
