#include "core/gate.hpp"

namespace syrec {

    gate::gate() {
        controls.clear();
        targets.clear();
        target_type = gateType::None;
    }

    gate::gate(const gate& other) {
        target_type = gateType::None;
        operator    =(other);
    }

    gate::~gate() = default;

    gate& gate::operator=(const gate& other) {
        if (this != &other) {
            controls.clear();
            std::copy(other.begin_controls(), other.end_controls(), std::insert_iterator<gate::line_container>(controls, controls.begin()));
            targets.clear();
            std::copy(other.begin_targets(), other.end_targets(), std::insert_iterator<gate::line_container>(targets, targets.begin()));
            target_type = other.type();
        }
        return *this;
    }

    gate::const_iterator gate::begin_controls() const {
        return controls.begin();
    }

    gate::const_iterator gate::end_controls() const {
        return controls.end();
    }

    gate::const_iterator gate::begin_targets() const {
        return targets.begin();
    }

    gate::const_iterator gate::end_targets() const {
        return targets.end();
    }

    void gate::add_control(line c) {
        controls.insert(c);
    }

    void gate::add_target(line l) {
        targets.insert(l);
    }

    void gate::set_type(gateType t) {
        target_type = t;
    }

    gateType gate::type() const {
        return target_type;
    }

} // namespace syrec
