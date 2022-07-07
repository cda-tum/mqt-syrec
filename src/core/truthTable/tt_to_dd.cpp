#include "core/truthTable/tt_to_dd.hpp"

namespace syrec {

    dd::mEdge buildDD(truthTable& tt, std::unique_ptr<dd::Package<>>& ddk) {
        truthTable::cube_vector dd_combination = tt.io_cube();

        truthTable::value_type falseVal = false;

        truthTable::value_type trueVal = true;

        auto f1 = dd::mEdge::zero;
        auto f2 = dd::mEdge::zero;
        auto f3 = dd::mEdge::zero;
        auto f4 = dd::mEdge::zero;

        syrec::truthTable tt1;
        syrec::truthTable tt2;
        syrec::truthTable tt3;
        syrec::truthTable tt4;

        if (tt.num_inputs() == 0) {
            return dd::mEdge::zero;
        }

        std::size_t label = (tt.num_inputs()) - 1;

        if (tt.num_inputs() == 1) {
            for (auto& [key, value]: dd_combination) {
                if (key[0] == falseVal && value[0] == falseVal) {
                    f1 = dd::mEdge::one;

                }

                else if (key[0] == trueVal && value[0] == falseVal) {
                    f2 = dd::mEdge::one;
                }

                else if (key[0] == falseVal && value[0] == trueVal) {
                    f3 = dd::mEdge::one;
                }

                else if (key[0] == trueVal && value[0] == trueVal) {
                    f4 = dd::mEdge::one;
                }

            }

            return ddk->makeDDNode(static_cast<dd::Qubit>(label), std::array{f1, f2, f3, f4});

        }

        else {
            for (auto& [key, value]: dd_combination) {
                if (key[0] == falseVal && value[0] == falseVal) {
                    truthTable::cube_type firstVal = key;

                    truthTable::cube_type secondVal = value;

                    firstVal.erase(firstVal.begin());

                    secondVal.erase(secondVal.begin());

                    tt1.add_entry(firstVal, secondVal);

                }

                else if (key[0] == trueVal && value[0] == falseVal) {
                    truthTable::cube_type firstVal = key;

                    truthTable::cube_type secondVal = value;

                    firstVal.erase(firstVal.begin());

                    secondVal.erase(secondVal.begin());

                    tt2.add_entry(firstVal, secondVal);

                }

                else if (key[0] == falseVal && value[0] == trueVal) {
                    truthTable::cube_type firstVal = key;

                    truthTable::cube_type secondVal = value;

                    firstVal.erase(firstVal.begin());

                    secondVal.erase(secondVal.begin());

                    tt3.add_entry(firstVal, secondVal);

                }

                else if (key[0] == trueVal && value[0] == trueVal) {
                    truthTable::cube_type firstVal = key;

                    truthTable::cube_type secondVal = value;

                    firstVal.erase(firstVal.begin());

                    secondVal.erase(secondVal.begin());

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
