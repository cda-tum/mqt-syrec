#ifndef TT_TO_DD_HPP
#define TT_TO_DD_HPP

#include "core/truthTable/truth_table.hpp"
#include "dd/Package.hpp"

namespace syrec {

    dd::mEdge buildDD(syrec::truth_table& tt, std::unique_ptr<dd::Package<>>& ddk);

}

#endif /* TT_TO_DD_HPP */
