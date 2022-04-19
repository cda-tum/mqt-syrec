#include "core/syrec/module.hpp"

#include <iterator>

namespace syrec {

    class module::priv {
    public:
        priv() = default;

        std::string    name;
        variable::vec  parameters;
        variable::vec  variables;
        statement::vec statements;
    };

    module::module(const std::string& name) :d(new priv()) {
        d->name = name;
    }

    module::~module() {
        delete d;
    }

    const std::string& module::name() const {
        return d->name;
    }

    void module::add_parameter(const variable::ptr& parameter) {
        d->parameters.emplace_back(parameter);
    }

    const variable::vec& module::parameters() const {
        return d->parameters;
    }

    const variable::vec& module::variables() const {
        return d->variables;
    }

    variable::ptr module::find_parameter_or_variable(const std::string& name) const {
        for (variable::ptr var: d->parameters) {
            if (var->name() == name) {
                return var;
            }
        }

        return {};
    }

    void module::add_statement(const statement::ptr& statement) {
        d->statements.emplace_back(statement);
    }

    const statement::vec& module::statements() const {
        return d->statements;
    }

} // namespace syrec
