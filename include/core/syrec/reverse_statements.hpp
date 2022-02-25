/**
 * @brief Reverse Statements
 *
 * @file reverse_statements.hpp
 */

#include <core/syrec/statement.hpp>

namespace revkit::syrec {

        struct reverse_statements {
            typedef statement::ptr result_type;

            statement::ptr operator()(statement::ptr _statement) const;
        };

    } // namespace revkit
