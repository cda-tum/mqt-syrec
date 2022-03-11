#include "core/syrec/program.hpp"

#include <iterator>

namespace syrec::applications {

    class program::priv {
    public:
        priv() = default;

        module::vec modules;
    };

    program::program():
        d(new priv()) {
    }

    program::~program() {
        delete d;
    }

    void program::add_module(const module::ptr& module) {
        d->modules.emplace_back(module);
    }

    const module::vec& program::modules() const {
        return d->modules;
    }

    module::ptr program::find_module(const std::string& name) const {
        for (module::ptr& p: d->modules) {
            if (p->name() == name) {
                return p;
            }
        }

        return {};
    }

} // namespace syrec::applications
