#include "core/gate.hpp"

namespace syrec {

    struct gate::priv {
        priv() = default;

        line_container controls;
        line_container targets;
        std::any       target_type;
    };

    gate::gate():
        d(new priv()) {
    }

    gate::gate(const gate& other):
        d(new priv()) {
        operator=(other);
    }

    gate::~gate() {
        delete d;
    }

    gate& gate::operator=(const gate& other) {
        if (this != &other) {
            d->controls.clear();
            std::copy(other.begin_controls(), other.end_controls(), std::insert_iterator<gate::line_container>(d->controls, d->controls.begin()));
            d->targets.clear();
            std::copy(other.begin_targets(), other.end_targets(), std::insert_iterator<gate::line_container>(d->targets, d->targets.begin()));
            d->target_type = other.type();
        }
        return *this;
    }

    gate::const_iterator gate::begin_controls() const {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(d->controls.begin(), d->controls.end()), transform_line());
    }

    gate::const_iterator gate::end_controls() const {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(d->controls.end(), d->controls.end()), transform_line());
    }

    gate::const_iterator gate::begin_targets() const {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(d->targets.begin(), d->targets.end()), transform_line());
    }

    gate::const_iterator gate::end_targets() const {
        return boost::make_transform_iterator(boost::make_filter_iterator<filter_line>(d->targets.end(), d->targets.end()), transform_line());
    }

    void gate::add_control(line c) {
        d->controls.insert(c);
    }

    void gate::add_target(line l) {
        d->targets.insert(l);
    }

    void gate::set_type(const std::any& t) {
        d->target_type = t;
    }

    const std::any& gate::type() const {
        return d->target_type;
    }

} // namespace syrec
