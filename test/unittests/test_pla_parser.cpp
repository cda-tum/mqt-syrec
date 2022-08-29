#include "core/io/pla_parser.hpp"

#include "gtest/gtest.h"

using namespace syrec;

class PlaParserTest: public testing::Test {
protected:
    std::string test_circuits_dir = "./circuits/";
    TruthTable  testPla;

    TruthTable::Cube::Value emptyVal;

    TruthTable::Cube c11{};

    TruthTable::Cube c1{};

    TruthTable::Cube c1_{};
    TruthTable::Cube c_1{};

    void SetUp() override {
        c11 = TruthTable::Cube::fromInteger(0b11U, 2U);

        c1 = TruthTable::Cube::fromInteger(0b1U, 1U);

        c1_.emplace_back(true);
        c1_.emplace_back(emptyVal);

        c_1.emplace_back(emptyVal);
        c_1.emplace_back(true);
    }
};

TEST_F(PlaParserTest, andTest) {
    std::string circAnd = test_circuits_dir + "and.pla";

    EXPECT_TRUE(read_pla(testPla, circAnd));

    EXPECT_EQ(testPla.nInputs(), 2U);
    EXPECT_EQ(testPla.nOutputs(), 1U);
    EXPECT_EQ(testPla.ioCube().size(), 1U);

    auto it11 = testPla.ioCube().find(c11);

    EXPECT_TRUE(it11 != testPla.ioCube().end());
    EXPECT_EQ(it11->second, c1);
}

TEST_F(PlaParserTest, orTest) {
    std::string circOr = test_circuits_dir + "or.pla";

    EXPECT_TRUE(read_pla(testPla, circOr));

    EXPECT_EQ(testPla.nInputs(), 2U);
    EXPECT_EQ(testPla.nOutputs(), 1U);
    EXPECT_EQ(testPla.ioCube().size(), 2U);

    auto it_1 = testPla.ioCube().find(c_1);

    EXPECT_TRUE(it_1 != testPla.ioCube().end());
    EXPECT_EQ(it_1->second, c1);

    auto it1_ = testPla.ioCube().find(c1_);

    EXPECT_TRUE(it1_ != testPla.ioCube().end());
    EXPECT_EQ(it1_->second, c1);
}
