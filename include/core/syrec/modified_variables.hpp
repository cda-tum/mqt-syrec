/**
 * @brief TODO
 *
 * @file modified_variables.hpp
 */

#include <core/syrec/program.hpp>
#include <core/syrec/statement.hpp>
#include <core/syrec/variable.hpp>
#include <map>
#include <set>

namespace revkit::syrec {

    [[maybe_unused]] void modified_variables(const program& prog,
                                             std::map<statement::ptr, std::set<variable::ptr>>&);

} // namespace revkit::syrec
