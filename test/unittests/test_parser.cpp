#include "core/syrec/program.hpp"
#include "core/test_functions.hpp"

#include "gtest/gtest.h"

using namespace syrec;

class SyrecParserTest: public testing::TestWithParam<std::string> {
protected:

    std::string test_circuits_dir  = "./circuits/";
    std::string file_name;

    void SetUp() override {
        file_name = test_circuits_dir + GetParam() + ".src";
    }
};

INSTANTIATE_TEST_SUITE_P(SyrecParserTest, SyrecParserTest,
                         testing::Values(
                                 "alu_2",
                                 "swap_2"
                                 "shift_4"
                                 ),
                         [](const testing::TestParamInfo<SyrecParserTest::ParamType>& info) {
                             auto s = info.param;
                             std::replace( s.begin(), s.end(), '-', '_');
                             return s; });

TEST_P(SyrecParserTest, GenericParserTest) {
    applications::program prog;
    std::string           error_string;
    error_string = my_read_program(prog, file_name);
    EXPECT_TRUE(error_string.empty());
}