/**
 * @file number.hpp
 *
 * @brief SyReC number data type
 */
#ifndef NUMBER_HPP
#define NUMBER_HPP

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <variant>

template<class... Ts>
struct overloaded: Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

namespace syrec {

    class number {
    public:
        typedef std::shared_ptr<number> ptr;

        typedef std::map<std::string, unsigned> loop_variable_mapping;

        explicit number(std::variant<unsigned, std::string> number):
            number_var(std::move(number)) {}

        explicit number(unsigned value):
            number_var(value) {
        }

        explicit number(const std::string& value):
            number_var(value) {
        }

        ~number() = default;

        [[nodiscard]] bool is_loop_variable() const {
            return std::holds_alternative<std::string>(number_var);
        }

        [[nodiscard]] bool is_constant() const {
            return std::holds_alternative<unsigned>(number_var);
        }

        [[nodiscard]] const std::string& variable_name() const {
            return std::get<std::string>(number_var);
        }

        [[nodiscard]] unsigned evaluate(const loop_variable_mapping& map) const {
            return std::visit(overloaded{
                                      [](unsigned arg) { return arg; },
                                      [&](const std::string& value) { return map.find(value)->second; }},
                              number_var);
        }

    private:
        std::variant<unsigned, std::string> number_var;
    };

} // namespace syrec

#endif /* NUMBER_HPP */
