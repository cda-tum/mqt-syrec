#include "core/syrec/program.hpp"
#include "core/test_functions.hpp"

#include "gtest/gtest.h"

using namespace syrec;

class SyrecParserTest: public testing::TestWithParam<std::string> {
protected:
    std::string test_circuits_dir = "./circuits/";
    std::string file_name;

    void SetUp() override {
        file_name = test_circuits_dir + GetParam() + ".src";
    }
};

INSTANTIATE_TEST_SUITE_P(SyrecParserTest, SyrecParserTest,
                         testing::Values(
                                 "alu_2",
                                 "binary_numeric",
                                 "bitwise_and_2",
                                 "bitwise_or_2",
                                 "bn_2",
                                 "call_8",
                                 "divide_2",
                                 "for_4",
                                 "for_32",
                                 "gray_binary_conversion_16",
                                 "input_repeated_2",
                                 "input_repeated_4",
                                 "logical_and_1",
                                 "logical_or_1",
                                 "modulo_2",
                                 "multiply_2",
                                 "negate_8",
                                 "numeric_2",
                                 "operators_repeated_4",
                                 "parity_4",
                                 "parity_check_16",
                                 "shift_4",
                                 "simple_add_2",
                                 "single_longstatement_4",
                                 "skip",
                                 "swap_2"),
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