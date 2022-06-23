#include "core/truthTable/tt_to_dd.hpp"

namespace syrec {

    dd::mEdge buildDD(syrec::truth_table& tt, std::unique_ptr<dd::Package<>>& ddk) {
        syrec::truth_table::io_vector dd_combination = tt.get_iocombination();

        auto zero_node = dd::mEdge::zero;
        auto one_node  = dd::mEdge::one;

        if (dd_combination.empty()) {
            return zero_node;
        }

        syrec::truth_table::io_vector p1_vector;

        syrec::truth_table::io_vector p2_vector;

        syrec::truth_table::io_vector p3_vector;

        syrec::truth_table::io_vector p4_vector;

        syrec::truth_table::io_type safe;

        auto p1 = zero_node;
        auto p2 = zero_node;
        auto p3 = zero_node;
        auto p4 = zero_node;

        auto f1 = zero_node;
        auto f2 = zero_node;
        auto f3 = zero_node;
        auto f4 = zero_node;

        int label = (dd_combination[0].size() / 2) - 1;

        if (dd_combination[0].size() == 2) {
            for (int i = 0; i < static_cast<int>(dd_combination.size()); i++) {
                if (dd_combination[i][0] == '0' && dd_combination[i][1] == '0') {
                    f1 = one_node;

                }

                else if (dd_combination[i][0] == '1' && dd_combination[i][1] == '0') {
                    f2 = one_node;
                }

                else if (dd_combination[i][0] == '0' && dd_combination[i][1] == '1') {
                    f3 = one_node;
                }

                else if (dd_combination[i][0] == '1' && dd_combination[i][1] == '1') {
                    f4 = one_node;
                }

                /*else if (dd_combination[i][0] == '0' && dd_combination[i][1] == '_')  {

		}

		else if (dd_combination[i][0] == '1' && dd_combination[i][1] == '_')  {

		}*/
            }

            return ddk->makeDDNode(static_cast<dd::Qubit>(label), std::array{f1, f2, f3, f4});

        }

        else {
            for (int i = 0; i < static_cast<int>(dd_combination.size()); i++) {
                if (dd_combination[i][0] == '0' && dd_combination[i][1] == '0') {
                    safe = (dd_combination[i]).erase(0, 2);

                    p1_vector.push_back(safe);

                }

                else if (dd_combination[i][0] == '1' && dd_combination[i][1] == '0') {
                    safe = (dd_combination[i]).erase(0, 2);

                    p2_vector.push_back(safe);

                }

                else if (dd_combination[i][0] == '0' && dd_combination[i][1] == '1') {
                    safe = (dd_combination[i]).erase(0, 2);

                    p3_vector.push_back(safe);

                }

                else if (dd_combination[i][0] == '1' && dd_combination[i][1] == '1') {
                    safe = (dd_combination[i]).erase(0, 2);

                    p4_vector.push_back(safe);
                }

                /*else if (dd_combination[i][0] == '0' && dd_combination[i][1] == '_'){
			
			}

		else if (dd_combination[i][0] == '1' && dd_combination[i][1] == '_'){
			
			}
		*/
            }

            syrec::truth_table tt1, tt2, tt3, tt4;

            tt1.set_iocombination(p1_vector);
            tt2.set_iocombination(p2_vector);
            tt3.set_iocombination(p3_vector);
            tt4.set_iocombination(p4_vector);

            p1 = buildDD(tt1, ddk);

            p2 = buildDD(tt2, ddk);

            p3 = buildDD(tt3, ddk);

            p4 = buildDD(tt4, ddk);

            return ddk->makeDDNode(static_cast<dd::Qubit>(label), std::array{p1, p2, p3, p4});
        }
    }

} // namespace syrec
