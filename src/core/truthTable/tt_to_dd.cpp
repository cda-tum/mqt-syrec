#include "core/truthTable/tt_to_dd.hpp"

namespace syrec {

    dd::mEdge buildDD(const TruthTable& tt, std::unique_ptr<dd::Package<>>& ddk) {
        TruthTable::CubeVector dd_combination = tt.io_cube();

        TruthTable::ValueType falseVal = false;

        TruthTable::ValueType trueVal = true;

        auto f1 = dd::mEdge::zero;
        auto f2 = dd::mEdge::zero;
        auto f3 = dd::mEdge::zero;
        auto f4 = dd::mEdge::zero;

        syrec::TruthTable tt1;
        syrec::TruthTable tt2;
        syrec::TruthTable tt3;
        syrec::TruthTable tt4;

        if (tt.num_inputs() == 0) {
            return dd::mEdge::zero;
        }

        std::size_t label = (tt.num_inputs()) - 1;

        if (tt.num_inputs() == 1) {
            for (auto& [key, value]: dd_combination) {
                if (key.c[0] == falseVal && value.c[0] == falseVal) {
                    f1 = dd::mEdge::one;

                }

                else if (key.c[0] == trueVal && value.c[0] == falseVal) {
                    f2 = dd::mEdge::one;
                }

                else if (key.c[0] == falseVal && value.c[0] == trueVal) {
                    f3 = dd::mEdge::one;
                }

                else if (key.c[0] == trueVal && value.c[0] == trueVal) {
                    f4 = dd::mEdge::one;
                }

            }

            return ddk->makeDDNode(static_cast<dd::Qubit>(label), std::array{f1, f2, f3, f4});

        }

        else {
            for (auto& [key, value]: dd_combination) {
                if (key.c[0] == falseVal && value.c[0] == falseVal) {
                    TruthTable::CubeType firstVal = key;

                    TruthTable::CubeType secondVal = value;

                    firstVal.c.erase(firstVal.c.begin());

                    secondVal.c.erase(secondVal.c.begin());

                    tt1.add_entry(firstVal, secondVal);

                }

                else if (key.c[0] == trueVal && value.c[0] == falseVal) {
                    TruthTable::CubeType firstVal = key;

                    TruthTable::CubeType secondVal = value;

                    firstVal.c.erase(firstVal.c.begin());

                    secondVal.c.erase(secondVal.c.begin());

                    tt2.add_entry(firstVal, secondVal);

                }

                else if (key.c[0] == falseVal && value.c[0] == trueVal) {
                    TruthTable::CubeType firstVal = key;

                    TruthTable::CubeType secondVal = value;

                    firstVal.c.erase(firstVal.c.begin());

                    secondVal.c.erase(secondVal.c.begin());

                    tt3.add_entry(firstVal, secondVal);

                }

                else if (key.c[0] == trueVal && value.c[0] == trueVal) {
                    TruthTable::CubeType firstVal = key;

                    TruthTable::CubeType secondVal = value;

                    firstVal.c.erase(firstVal.c.begin());

                    secondVal.c.erase(secondVal.c.begin());

                    tt4.add_entry(firstVal, secondVal);
                }

            }

            auto p1 = buildDD(tt1, ddk);

            auto p2 = buildDD(tt2, ddk);

            auto p3 = buildDD(tt3, ddk);

            auto p4 = buildDD(tt4, ddk);

            return ddk->makeDDNode(static_cast<dd::Qubit>(label), std::array{p1, p2, p3, p4});
        }
    }

} // namespace syrec
