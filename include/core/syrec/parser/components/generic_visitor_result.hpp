#ifndef CORE_SYREC_PARSER_COMPONENTS_GENERIC_VISITOR_RESULT_HPP
#define CORE_SYREC_PARSER_COMPONENTS_GENERIC_VISITOR_RESULT_HPP
#pragma once

#include <memory>

namespace syrecParser {
    template <typename Base>
    class GenericVisitorResult {
    public:
        template<typename Derived, typename std::enable_if_t<std::is_base_of_v<Base, Derived>>* = nullptr>
        GenericVisitorResult(std::shared_ptr<Derived> result) {
            data = std::move(result);
        }

        [[nodiscard]] std::shared_ptr<Base> getData() const {
            return data;
        }

    protected:
        // To be able to cast to the std::any type, the GenericVisitorResult needs to be copy constructable (and thus forces the use of std::shared_ptr instead of std::unique_ptr)
        // see: https://en.cppreference.com/w/cpp/utility/any
        std::shared_ptr<Base> data;
    };
}
#endif