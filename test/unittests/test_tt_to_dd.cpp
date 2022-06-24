#include "core/truthTable/tt_to_dd.hpp"

#include "gtest/gtest.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

class truthTable_to_dd_test: public testing::TestWithParam<std::string> {
protected:
    std::string test_circuits_dir = "./circuits/";

    int                           expected_node_size  = 0;
    int                           expected_qubit_size = 0;
    double                        expected_hit_ratio  = 0;
    syrec::truth_table::io_vector io_comb;

    void SetUp() override {
        std::string   tt_param = GetParam();
        std::ifstream i(test_circuits_dir + "truth_table.json");
        json          j = json::parse(i);

        io_comb             = j[tt_param]["io"];
        expected_node_size  = j[tt_param]["node_size"];
        expected_qubit_size = j[tt_param]["qubit_size"];
        expected_hit_ratio  = j[tt_param]["hit_ratio"];
    }
};

INSTANTIATE_TEST_SUITE_P(truthTable_to_dd_test, truthTable_to_dd_test,
                         testing::Values(
                                 "Id_2_bit",
                                 "Id_3_bit",
                                 "CNOT_2_bit",
                                 "Custom_tt"),
                         [](const testing::TestParamInfo<truthTable_to_dd_test::ParamType>& info) {
                             auto s = info.param;
                             std::replace( s.begin(), s.end(), '-', '_');
                             return s; });

TEST_P(truthTable_to_dd_test, Generic_tt_to_dd) {
    syrec::truth_table test_tt;

    test_tt.set_iocombination(io_comb);

    int input_size = io_comb[0].size() / 2;

    std::unique_ptr<dd::Package<>> dd1 = std::make_unique<dd::Package<>>(input_size);

    auto check = syrec::buildDD(test_tt, dd1);

    EXPECT_TRUE(check.p != nullptr);

    EXPECT_EQ(expected_qubit_size, static_cast<int>(dd1->qubits()));
    EXPECT_EQ(expected_node_size, static_cast<int>(dd1->mUniqueTable.getNodeCount()));
    //EXPECT_EQ(expected_hit_ratio, static_cast<double>(dd1->mUniqueTable.hitRatio()));
}
