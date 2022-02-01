#pragma once

#include <boost/dynamic_bitset.hpp>

namespace syrec {
    class Dummy {
    public:
        Dummy() = default;

        explicit Dummy(double val):
            val(val) {}

        [[nodiscard]] double getVal() const;

        void setVal(double v);

    protected:
        double                  val{};
        boost::dynamic_bitset<> bitset{};
    };
} // namespace syrec
