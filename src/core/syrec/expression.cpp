#include "core/syrec/expression.hpp"

#include <utility>

namespace syrec::applications {

    class expression::priv {
    public:
        priv() = default;
    };

    expression::expression():
        d(new priv()) {
    }

    expression::~expression() {
        delete d;
    }

    class numeric_expression::priv {
    public:
        priv() = default;

        number::ptr value = nullptr;
        unsigned    bitwidth{};
    };

    numeric_expression::numeric_expression(const number::ptr& value, unsigned bitwidth):
        d(new priv()) {
        d->value    = value;
        d->bitwidth = bitwidth;
    }

    numeric_expression::~numeric_expression() {
        delete d;
    }

    const number::ptr& numeric_expression::value() const {
        return d->value;
    }

    unsigned numeric_expression::bitwidth() const {
        return d->bitwidth;
    }

    class variable_expression::priv {
    public:
        priv() = default;

        variable_access::ptr var = nullptr;
    };

    variable_expression::variable_expression(variable_access::ptr var):
        d(new priv()) {
        d->var = std::move(var);
    }

    variable_expression::~variable_expression() {
        delete d;
    }

    variable_access::ptr variable_expression::var() const {
        return d->var;
    }

    unsigned variable_expression::bitwidth() const {
        return d->var->bitwidth();
    }

    class binary_expression::priv {
    public:
        priv() = default;

        expression::ptr lhs = nullptr;
        expression::ptr rhs = nullptr;
        unsigned        op{};
    };

    binary_expression::binary_expression(expression::ptr lhs,
                                         unsigned        op,
                                         expression::ptr rhs):
        d(new priv()) {
        d->lhs = std::move(lhs);
        d->op  = op;
        d->rhs = std::move(rhs);
    }

    binary_expression::~binary_expression() {
        delete d;
    }

    expression::ptr binary_expression::lhs() const {
        return d->lhs;
    }

    expression::ptr binary_expression::rhs() const {
        return d->rhs;
    }

    unsigned binary_expression::op() const {
        return d->op;
    }

    unsigned binary_expression::bitwidth() const {
        switch (d->op) {
            case logical_and:
            case logical_or:
            case less_than:
            case greater_than:
            case equals:
            case not_equals:
            case less_equals:
            case greater_equals:
                return 1;

            default:
                // lhs and rhs are assumed to have the same bit-width
                return d->lhs->bitwidth();
        }
    }

    class shift_expression::priv {
    public:
        priv() = default;

        expression::ptr lhs = nullptr;
        number::ptr     rhs = nullptr;
        unsigned        op{};
    };

    shift_expression::shift_expression(expression::ptr    lhs,
                                       unsigned           op,
                                       const number::ptr& rhs):
        d(new priv()) {
        d->lhs = std::move(lhs);
        d->op  = op;
        d->rhs = rhs;
    }

    shift_expression::~shift_expression() {
        delete d;
    }

    expression::ptr shift_expression::lhs() const {
        return d->lhs;
    }

    const number::ptr& shift_expression::rhs() const {
        return d->rhs;
    }

    unsigned shift_expression::op() const {
        return d->op;
    }

    unsigned shift_expression::bitwidth() const {
        return d->lhs->bitwidth();
    }

} // namespace syrec::applications
