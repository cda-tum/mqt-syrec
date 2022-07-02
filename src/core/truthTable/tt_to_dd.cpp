#include "core/truthTable/tt_to_dd.hpp"

namespace syrec {

    dd::mEdge buildDD(truthTable& tt, std::unique_ptr<dd::Package<>>& ddk) {
        truthTable::cube_vector dd_combination = tt.io_cube();

        auto zero_node = dd::mEdge::zero;
        auto one_node  = dd::mEdge::one;

        truthTable::value_type falseVal = false;

        //truthTable::value_type emptyVal;

        truthTable::value_type trueVal = true;

        auto p1 = zero_node;
        auto p2 = zero_node;
        auto p3 = zero_node;
        auto p4 = zero_node;

        auto f1 = zero_node;
        auto f2 = zero_node;
        auto f3 = zero_node;
        auto f4 = zero_node;

        syrec::truthTable tt1, tt2, tt3, tt4;

        if (tt.num_inputs() == 0) {
            return zero_node;
        }

        int label = (tt.num_inputs()) - 1;

        if (tt.num_inputs() == 1) {
            for (auto& i: dd_combination) {
                if (i.first[0] == falseVal && i.second[0] == falseVal) {
                    f1 = one_node;

                }

                else if (i.first[0] == trueVal && i.second[0] == falseVal) {
                    f2 = one_node;
                }

                else if (i.first[0] == falseVal && i.second[0] == trueVal) {
                    f3 = one_node;
                }

                else if (i.first[0] == trueVal && i.second[0] == trueVal) {
                    f4 = one_node;
                }

                /*else if (i.first[0] == falseVal && i.second[0] == emptyVal)  {

			}

			else if (i.first[0] == trueVal && i.second[0] == emptyVal)  {

			}*/
            }

            return ddk->makeDDNode(static_cast<dd::Qubit>(label), std::array{f1, f2, f3, f4});

        }

        else {
            for (auto& i: dd_combination) {
                if (i.first[0] == falseVal && i.second[0] == falseVal) {
                    truthTable::cube_type firstVal = i.first;

                    truthTable::cube_type secondVal = i.second;

                    firstVal.erase(firstVal.begin());

                    secondVal.erase(secondVal.begin());

                    tt1.add_entry(firstVal, secondVal);

                }

                else if (i.first[0] == trueVal && i.second[0] == falseVal) {
                    truthTable::cube_type firstVal = i.first;

                    truthTable::cube_type secondVal = i.second;

                    firstVal.erase(firstVal.begin());

                    secondVal.erase(secondVal.begin());

                    tt2.add_entry(firstVal, secondVal);

                }

                else if (i.first[0] == falseVal && i.second[0] == trueVal) {
                    truthTable::cube_type firstVal = i.first;

                    truthTable::cube_type secondVal = i.second;

                    firstVal.erase(firstVal.begin());

                    secondVal.erase(secondVal.begin());

                    tt3.add_entry(firstVal, secondVal);

                }

                else if (i.first[0] == trueVal && i.second[0] == trueVal) {
                    truthTable::cube_type firstVal = i.first;

                    truthTable::cube_type secondVal = i.second;

                    firstVal.erase(firstVal.begin());

                    secondVal.erase(secondVal.begin());

                    tt4.add_entry(firstVal, secondVal);
                }

                /*else if (i.first[0] == falseVal && i.second[0] == emptyVal)  {

            }

            else if (i.first[0] == trueVal && i.second[0] == emptyVal)  {

            }*/
            }

            p1 = buildDD(tt1, ddk);

            p2 = buildDD(tt2, ddk);

            p3 = buildDD(tt3, ddk);

            p4 = buildDD(tt4, ddk);

            return ddk->makeDDNode(static_cast<dd::Qubit>(label), std::array{p1, p2, p3, p4});
        }
    }

} // namespace syrec
