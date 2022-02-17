#include "Dummy.hpp"

namespace syrec {
    double Dummy::getVal() const {
        return val;
    }

    void Dummy::setVal(double v) {
        val = v;
    }
} // namespace syrec
