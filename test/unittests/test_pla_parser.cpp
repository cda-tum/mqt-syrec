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

    TruthTable::Cube c10{};
    TruthTable::Cube c01{};

    TruthTable::Cube c1_0{};
    TruthTable::Cube c101{};

    void SetUp() override {
        c11  = TruthTable::Cube::fromInteger(0b11U, 2U);
        c1   = TruthTable::Cube::fromInteger(0b1U, 1U);
        c01  = TruthTable::Cube::fromInteger(0b01U, 2U);
        c10  = TruthTable::Cube::fromInteger(0b10U, 2U);
        c101 = TruthTable::Cube::fromInteger(0b101U, 3U);

        c1_.emplace_back(true);
        c1_.emplace_back(emptyVal);

        c_1.emplace_back(emptyVal);
        c_1.emplace_back(true);

        c1_0.emplace_back(true);
        c1_0.emplace_back(emptyVal);
        c1_0.emplace_back(false);
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

TEST_F(PlaParserTest, example2Test) {
    std::string circExample2 = test_circuits_dir + "example2.pla";

    EXPECT_TRUE(read_pla(testPla, circExample2));

    EXPECT_EQ(testPla.nInputs(), 3U);
    EXPECT_EQ(testPla.nOutputs(), 1U);
    EXPECT_EQ(testPla.ioCube().size(), 2U);

    auto it1_0 = testPla.ioCube().find(c1_0);

    EXPECT_TRUE(it1_0 != testPla.ioCube().end());
    EXPECT_EQ(it1_0->second, c1);

    auto it101 = testPla.ioCube().find(c101);

    EXPECT_TRUE(it101 != testPla.ioCube().end());
    EXPECT_EQ(it101->second, c1);
}

TEST_F(PlaParserTest, example3Test) {
    std::string circExample3 = test_circuits_dir + "example3.pla";

    EXPECT_TRUE(read_pla(testPla, circExample3));

    EXPECT_EQ(testPla.nInputs(), 2U);
    EXPECT_EQ(testPla.nOutputs(), 1U);
    EXPECT_EQ(testPla.ioCube().size(), 2U);

    auto it01 = testPla.ioCube().find(c01);

    EXPECT_TRUE(it01 != testPla.ioCube().end());
    EXPECT_EQ(it01->second, c1);

    auto it10 = testPla.ioCube().find(c10);

    EXPECT_TRUE(it10 != testPla.ioCube().end());
    EXPECT_EQ(it10->second, c1);
}
