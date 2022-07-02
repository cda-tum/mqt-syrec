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

    dd::QubitCount                     nqubits = 2U;
    std::vector<std::vector<bool>>     inDummy;
    std::vector<std::vector<bool>>     outDummy;
    truthTable::cube_type              dummyVec;
    std::vector<truthTable::cube_type> inCube;
    std::vector<truthTable::cube_type> outCube;
    truthTable                         tt;
    std::unique_ptr<dd::Package<>>     dd;
    qc::MatrixDD                       ident{};

    void SetUp() override {
        std::string   tt_param = GetParam();
        std::ifstream i(testCircuitsDir + "truth_table.json");
        json          j = json::parse(i);

        dd = std::make_unique<dd::Package<>>(nqubits);

        inDummy = j[tt_param]["in"];

        outDummy = j[tt_param]["out"];

        ident = dd->makeIdent(nqubits);

        for (auto i: inDummy) {
            for (auto j: i) {
                std::optional<bool> dummy = j;
                dummyVec.push_back(dummy);
            }
            inCube.push_back(dummyVec);
            dummyVec.clear();
        }

        dummyVec.clear();

        for (auto i: outDummy) {
            for (auto j: i) {
                std::optional<bool> dummy = j;
                dummyVec.push_back(dummy);
            }
            outCube.push_back(dummyVec);
            dummyVec.clear();
        }

        for (int i = 0; i < (int)inCube.size(); i++) {
            tt.add_entry(inCube[i], outCube[i]);
        }
    }
};

INSTANTIATE_TEST_SUITE_P(truthTableDD, truthTableDD,
                         testing::Values(
                                 "Id_2_bit",
                                 "CNOT"),
                         [](const testing::TestParamInfo<truthTableDD::ParamType>& info) {
                             auto s = info.param;
                             std::replace( s.begin(), s.end(), '-', '_');
                             return s; });

TEST_P(truthTableDD, GenericttDD) {
    std::string circuit = (std::string)GetParam();

    extend_truth_table(tt);

    HuffmanCodes(tt);

    auto root = buildDD(tt, dd);

    qc::MatrixDD ddTest;

    if (circuit == "Id_2_bit") {
        ddTest = ident;
    }

    else if (circuit == "CNOT") {
        auto q = qc::QuantumComputation(2);
        q.x(0, 1_pc); // creates CNOT with target q0 and control q1
        ddTest = dd::buildFunctionality(&q, dd);
    }

    EXPECT_EQ(tt.num_inputs(), static_cast<int>(dd->qubits()));
    EXPECT_TRUE(root.p != nullptr);
    EXPECT_EQ(root, ddTest);
}
