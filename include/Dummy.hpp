#pragma once

namespace syrec {
    class Dummy {
    public:
        Dummy() = default;

        explicit Dummy(double val):
            val(val) {}

        [[nodiscard]] double getVal() const;

        void setVal(double v);

    protected:
        double val{};
    };
} // namespace syrec
