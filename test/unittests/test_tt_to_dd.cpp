#include "core/truthTable/tt_to_dd.hpp"
#include "dd/FunctionalityConstruction.hpp"

#include "gtest/gtest.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

using namespace dd::literals;

using namespace syrec;

class truthTableDD: public testing::TestWithParam<std::string> {
protected:
    std::string testCircuitsDir = "./circuits/";

    dd::QubitCount                     nqubits = 3U;
    std::vector<std::vector<bool>>     inDummy;
    std::vector<std::vector<bool>>     outDummy;
    truthTable::cube_type              dummyVec;
    std::vector<truthTable::cube_type> inCube;
    std::vector<truthTable::cube_type> outCube;
    truthTable                         tt;
    std::unique_ptr<dd::Package<>>     dd;

    void SetUp() override {
        std::string   tt_param = GetParam();
        std::ifstream i(testCircuitsDir + "truth_table.json");
        json          j = json::parse(i);

        dd = std::make_unique<dd::Package<>>(nqubits);

        inDummy = j[tt_param]["in"];

        outDummy = j[tt_param]["out"];

        for (const auto& in: inDummy) {
            for (auto inVal: in) {
                std::optional<bool> dummy = inVal;
                dummyVec.push_back(dummy);
            }
            inCube.push_back(dummyVec);
            dummyVec.clear();
        }

        dummyVec.clear();

        for (const auto& out: outDummy) {
            for (auto outVal: out) {
                std::optional<bool> dummy = outVal;
                dummyVec.push_back(dummy);
            }
            outCube.push_back(dummyVec);
            dummyVec.clear();
        }

        for (std::size_t index = 0; index < inCube.size(); index++) {
            tt.add_entry(inCube[index], outCube[index]);
        }
    }
};

INSTANTIATE_TEST_SUITE_P(truthTableDD, truthTableDD,
                         testing::Values(
                                 "Id2bit",
                                 "CNOT",
                                 "SWAP",
                                 "NEGCXX"),
                         [](const testing::TestParamInfo<truthTableDD::ParamType>& info) {
                             auto s = info.param;
                             std::replace( s.begin(), s.end(), '-', '_');
                             return s; });

TEST_P(truthTableDD, GenericttDD) {
    EXPECT_TRUE(tt.num_inputs() != 0 && tt.num_inputs() != 0);

    auto circuit = (std::string)GetParam();

    tt.extend_truth_table();

    tt.HuffmanCodes();

    EXPECT_TRUE(tt.num_inputs() != 0 && tt.num_inputs() == tt.num_outputs());

    auto root = buildDD(tt, dd);

    qc::MatrixDD ddTest;

    if (circuit == "Id2bit") {
        ddTest = dd->makeIdent(2);
    }

    else if (circuit == "CNOT") {
        auto q = qc::QuantumComputation(2);
        q.x(0, 1_pc); // creates CNOT with target q0 and control q1
        ddTest = dd::buildFunctionality(&q, dd);
    }

    else if (circuit == "SWAP") {
        auto q = qc::QuantumComputation(2);
        q.swap(0, 1); // creates CNOT with target q0 and control q1
        ddTest = dd::buildFunctionality(&q, dd);
    }

    else if (circuit == "NEGCXX") {
        auto q = qc::QuantumComputation(3);
        q.x(1);
        q.x(2);
        q.x(0, dd::Controls{1_pc, 2_pc}); // creates negative CXX with target q0 and control q1 and q2
        q.x(1);
        q.x(2);
        ddTest = dd::buildFunctionality(&q, dd);
    }

    EXPECT_TRUE(root.p != nullptr);
    EXPECT_EQ(root, ddTest);
}
