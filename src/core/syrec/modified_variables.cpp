#include "core/syrec/modified_variables.hpp"

#define UNUSED(x) (void)(x)

namespace revkit {
    namespace syrec {

        void modified_variables(const program& prog,
                                std::map<statement::ptr, std::set<variable::ptr>>&) {
            UNUSED(prog);
        }

    } // namespace syrec
} // namespace revkit
