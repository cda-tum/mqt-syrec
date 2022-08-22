#include "core/truthTable/pla_parser.hpp"

#include "gtest/gtest.h"

using namespace syrec;

class PlaParserTest: public testing::TestWithParam<std::string> {
protected:
    std::string       test_circuits_dir = "./circuits/";
    syrec::TruthTable testPla;
};

TEST_F(PlaParserTest, andTest) {
    std::string circAnd = test_circuits_dir + "and.pla";

    EXPECT_EQ(testPla.nInputs(), 0U);
    EXPECT_EQ(testPla.nOutputs(), 0U);
    EXPECT_EQ(testPla.ioCube().size(), 0U);

    EXPECT_TRUE(read_pla(testPla, circAnd));

    EXPECT_EQ(testPla.nInputs(), 3U);
    EXPECT_EQ(testPla.nOutputs(), 3U);
    EXPECT_EQ(testPla.ioCube().size(), 4U);
}

TEST_F(PlaParserTest, orTest) {
    std::string circOr = test_circuits_dir + "or.pla";

    EXPECT_EQ(testPla.nInputs(), 0U);
    EXPECT_EQ(testPla.nOutputs(), 0U);
    EXPECT_EQ(testPla.ioCube().size(), 0U);

    EXPECT_TRUE(read_pla(testPla, circOr));

    EXPECT_EQ(testPla.nInputs(), 3U);
    EXPECT_EQ(testPla.nOutputs(), 3U);
    EXPECT_EQ(testPla.ioCube().size(), 4U);
}

TEST_F(PlaParserTest, exampleTest) {
    std::string circExample = test_circuits_dir + "example.pla";

    EXPECT_EQ(testPla.nInputs(), 0U);
    EXPECT_EQ(testPla.nOutputs(), 0U);
    EXPECT_EQ(testPla.ioCube().size(), 0U);

    EXPECT_TRUE(read_pla(testPla, circExample));

    EXPECT_EQ(testPla.nInputs(), 6U);
    EXPECT_EQ(testPla.nOutputs(), 6U);
    EXPECT_EQ(testPla.ioCube().size(), 32U);
}

TEST_F(PlaParserTest, example2Test) {
    std::string circExample2 = test_circuits_dir + "example2.pla";

    EXPECT_EQ(testPla.nInputs(), 0U);
    EXPECT_EQ(testPla.nOutputs(), 0U);
    EXPECT_EQ(testPla.ioCube().size(), 0U);

    EXPECT_TRUE(read_pla(testPla, circExample2));

    EXPECT_EQ(testPla.nInputs(), 4U);
    EXPECT_EQ(testPla.nOutputs(), 4U);
    EXPECT_EQ(testPla.ioCube().size(), 8U);
}

TEST_F(PlaParserTest, example3Test) {
    std::string circExample3 = test_circuits_dir + "example3.pla";

    EXPECT_EQ(testPla.nInputs(), 0U);
    EXPECT_EQ(testPla.nOutputs(), 0U);
    EXPECT_EQ(testPla.ioCube().size(), 0U);

    EXPECT_TRUE(read_pla(testPla, circExample3));
    EXPECT_EQ(testPla.nInputs(), 2U);
    EXPECT_EQ(testPla.nOutputs(), 2U);
    EXPECT_EQ(testPla.ioCube().size(), 4U);
}
