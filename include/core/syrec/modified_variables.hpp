/**
 * @brief TODO
 *
 * @file modified_variables.hpp
 */

#include <map>
#include <set>

#include <core/syrec/program.hpp>
#include <core/syrec/statement.hpp>
#include <core/syrec/variable.hpp>


namespace revkit
{
namespace syrec
{

  void modified_variables( const program& prog,
                           std::map<statement::ptr, std::set<variable::ptr> >& );

}
}
