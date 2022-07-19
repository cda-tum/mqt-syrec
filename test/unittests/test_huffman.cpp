#include "core/truthTable/truth_table.hpp"

#include "gtest/gtest.h"

using namespace dd::literals;
using namespace syrec;

class TruthTableHuff: public testing::TestWithParam<std::string> {
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

    TruthTable::Cube c0_{};
    TruthTable::Cube c1_{};

    TruthTable::Cube c0__{};
    TruthTable::Cube c1__{};

    TruthTable::Cube c10_{};

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

        c0_.emplace_back(false);
        c0_.emplace_back(emptyVal);

        c0__.emplace_back(false);
        c0__.emplace_back(emptyVal);
        c0__.emplace_back(emptyVal);

        c1_.emplace_back(true);
        c1_.emplace_back(emptyVal);

        c1__.emplace_back(true);
        c1__.emplace_back(emptyVal);
        c1__.emplace_back(emptyVal);

        c10_.emplace_back(true);
        c10_.emplace_back(false);
        c10_.emplace_back(emptyVal);
    }
};

TEST_F(TruthTableHuff, CUSTOM1) {
    tt.insert(c01, c01);
    tt.insert(c10, c01);
    tt.insert(c11, c11);

    EXPECT_EQ(tt.ioCube().size(), 3U);

    tt.extend();

    EXPECT_EQ(tt.ioCube().size(), 4U);

    auto search1 = tt.ioCube().find(c00);

    EXPECT_TRUE(search1 != tt.ioCube().end());

    EXPECT_EQ(search1->second, c00);

    EXPECT_EQ(tt.nOutputs(), 2U);

    tt.encodeHuffman();

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

TEST_F(TruthTableHuff, CUSTOM2) {
    tt.insert(c01, c11);
    tt.insert(c10, c11);
    tt.insert(c11, c11);

    EXPECT_EQ(tt.ioCube().size(), 3U);

    tt.extend();

    EXPECT_EQ(tt.ioCube().size(), 4U);

    auto search1 = tt.ioCube().find(c00);

    EXPECT_TRUE(search1 != tt.ioCube().end());

    EXPECT_EQ(search1->second, c00);

    EXPECT_EQ(tt.nOutputs(), 2U);

    tt.encodeHuffman();

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

    tt.augmentWithConstants();

    EXPECT_EQ(tt.nInputs(), 3U);

    TruthTable::Cube::Vector augInput{c000, c001, c010, c011};

    for (const auto& in2: augInput) {
        auto search4 = tt.ioCube().find(in2);

        EXPECT_TRUE(search4 != tt.ioCube().end());
    }
}

TEST_F(TruthTableHuff, CUSTOM3) {
    tt.insert(c000, c010);
    tt.insert(c001, c010);
    tt.insert(c010, c100);
    tt.insert(c011, c100);
    tt.insert(c100, c011);
    tt.insert(c101, c010);
    tt.insert(c110, c010);
    tt.insert(c111, c001);

    EXPECT_EQ(tt.ioCube().size(), 8U);

    tt.extend();

    EXPECT_EQ(tt.ioCube().size(), 8U);

    auto search1 = tt.ioCube().find(c000);

    EXPECT_TRUE(search1 != tt.ioCube().end());

    EXPECT_EQ(search1->second, c010);

    EXPECT_EQ(tt.nOutputs(), 3U);

    tt.encodeHuffman();

    EXPECT_EQ(tt.nOutputs(), 3U);

    TruthTable::Cube::Vector encInput1{c001, c001, c101, c110};

    for (const auto& in1: encInput1) {
        auto search2 = tt.ioCube().find(in1);

        EXPECT_TRUE(search2 != tt.ioCube().end());

        EXPECT_EQ(search2->second, c0__);
    }

    TruthTable::Cube::Vector encInput2{c010, c011};

    for (const auto& in2: encInput2) {
        auto search3 = tt.ioCube().find(in2);

        EXPECT_TRUE(search3 != tt.ioCube().end());

        EXPECT_EQ(search3->second, c10_);
    }

    auto search4 = tt.ioCube().find(c100);

    EXPECT_TRUE(search4 != tt.ioCube().end());

    EXPECT_EQ(search4->second, c111);

    auto search5 = tt.ioCube().find(c111);

    EXPECT_TRUE(search5 != tt.ioCube().end());

    EXPECT_EQ(search5->second, c110);

    EXPECT_EQ(tt.nInputs(), 3U);

    tt.augmentWithConstants();

    EXPECT_EQ(tt.nInputs(), 3U);
}
