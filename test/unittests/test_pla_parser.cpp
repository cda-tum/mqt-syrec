#include "core/io/pla_parser.hpp"

#include "gtest/gtest.h"

using namespace syrec;

class PlaParserTest: public testing::Test {
protected:
    std::string testCircuitsDir = "./circuits/";
    TruthTable  testPla;

    TruthTable::Cube::Value emptyVal;

    TruthTable::Cube c11{};

    TruthTable::Cube c1{};

    TruthTable::Cube cOneDc{};
    TruthTable::Cube cDcOne{};

    void SetUp() override {
        c11 = TruthTable::Cube::fromInteger(0b11U, 2U);

        c1 = TruthTable::Cube::fromInteger(0b1U, 1U);

        cOneDc.emplace_back(true);
        cOneDc.emplace_back(emptyVal);

        cDcOne.emplace_back(emptyVal);
        cDcOne.emplace_back(true);
    }
};

TEST_F(PlaParserTest, andTest) {
    std::string circAnd = testCircuitsDir + "and.pla";

    EXPECT_TRUE(readPla(testPla, circAnd));

    EXPECT_EQ(testPla.nInputs(), 2U);
    EXPECT_EQ(testPla.nOutputs(), 1U);
    EXPECT_EQ(testPla.size(), 1U);

    auto it11 = testPla.find(0b11U, 2U);

    EXPECT_TRUE(it11 != testPla.end());
    EXPECT_TRUE(it11->second.equals(0b1U, 1U));
}

TEST_F(PlaParserTest, orTest) {
    std::string circOr = testCircuitsDir + "or.pla";

    EXPECT_TRUE(readPla(testPla, circOr));

    EXPECT_EQ(testPla.nInputs(), 2U);
    EXPECT_EQ(testPla.nOutputs(), 1U);
    EXPECT_EQ(testPla.size(), 2U);

    auto itDcOne = testPla.find("-1");

    EXPECT_TRUE(itDcOne != testPla.end());
    EXPECT_TRUE(itDcOne->second.equals(0b1U, 1U));

    auto it1 = testPla.find("1-");

    EXPECT_TRUE(it1 != testPla.end());
    EXPECT_TRUE(it1->second.equals(0b1U, 1U));
}
