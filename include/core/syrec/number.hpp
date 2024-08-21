#pragma once

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <optional>

template<class... Ts>
struct Overloaded: Ts... {
    using Ts::operator()...;
};
template<class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

namespace syrec {

    class Number {
    public:
        using ptr = std::shared_ptr<Number>;

        using LoopVariableMapping = std::map<std::string, unsigned int>;

        explicit Number(std::variant<unsigned, std::string> number):
            numberVar(std::move(number)) {}

        explicit Number(unsigned value):
            numberVar(value) {
        }

        explicit Number(const std::string& value):
            numberVar(value) {
        }

        ~Number() = default;

        [[nodiscard]] bool isLoopVariable() const {
            return std::holds_alternative<std::string>(numberVar);
        }

        [[nodiscard]] bool isConstant() const {
            return std::holds_alternative<unsigned>(numberVar);
        }

        [[nodiscard]] const std::string& variableName() const {
            return std::get<std::string>(numberVar);
        }

        [[nodiscard]] unsigned evaluate(const LoopVariableMapping& map) const {
            return std::visit(Overloaded{
                                      [](unsigned arg) { return arg; },
                                      [&map](const std::string& value) { return map.find(value)->second; }},
                              numberVar);
        }

        [[nodiscard]] std::optional<unsigned> tryEvaluate(const LoopVariableMapping& loopVariableValueLookup) const {
            if (isConstant())
                return std::get<unsigned>(numberVar);
            if (isLoopVariable() && loopVariableValueLookup.count(std::get<std::string>(numberVar)) > 0)
                return loopVariableValueLookup.at(std::get<std::string>(numberVar));

            return std::nullopt;
        }

    private:
        std::variant<unsigned, std::string> numberVar;
    };

} // namespace syrec
