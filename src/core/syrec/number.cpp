#include "core/syrec/number.hpp"

#include <cassert>
#include <utility>
#include <variant>

namespace syrec::applications {

    struct evaluate_visitor {
        explicit evaluate_visitor(const number::loop_variable_mapping& map):
            map(map) {}

        unsigned operator()(unsigned value) const {
            return value;
        }

        unsigned operator()(const std::string& value) const {
            auto it = map.find(value);
            assert(it != map.end());
            return it->second;
        }

    private:
        const number::loop_variable_mapping& map;
    };

    class number::priv {
    public:
        explicit priv(std::variant<unsigned, std::string> number):
            number(std::move(number)) {}

        std::variant<unsigned, std::string> number;
    };

    number::number(unsigned value):
        d(new priv(value)) {
    }

    number::number(const std::string& value):
        d(new priv(value)) {
    }

    number::~number() {
        delete d;
    }

    bool number::is_loop_variable() const {
        return std::holds_alternative<std::string>(d->number);
    }

    bool number::is_constant() const {
        return std::holds_alternative<unsigned>(d->number);
    }

    const std::string& number::variable_name() const {
        return std::get<std::string>(d->number);
    }

    unsigned number::evaluate(const loop_variable_mapping& map) const {
        return std::visit(evaluate_visitor(map), d->number);
    }

} // namespace syrec::applications
