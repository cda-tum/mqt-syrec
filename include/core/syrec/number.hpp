#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>

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
        struct ConstantExpression {
            using ptr = std::shared_ptr<ConstantExpression>;

            enum class Operation : std::uint8_t {
                Addition,
                Subtraction,
                Multiplication,
                Division
            };
            Number::ptr lhsOperand;
            Operation   operation;
            Number::ptr rhsOperand;

            explicit ConstantExpression(Number::ptr lhsOperand, const Operation operation, Number::ptr rhsOperand):
                lhsOperand(std::move(lhsOperand)), operation(operation), rhsOperand(std::move(rhsOperand)) {}

            [[nodiscard]] std::optional<unsigned int> tryEvaluate(const LoopVariableMapping& loopVariableValueLookup) const {
                const std::optional<unsigned int> lhsOperandEvaluated = lhsOperand ? lhsOperand->tryEvaluate(loopVariableValueLookup) : std::nullopt;
                const std::optional<unsigned int> rhsOperandEvaluated = rhsOperand ? rhsOperand->tryEvaluate(loopVariableValueLookup) : std::nullopt;
                switch (operation) {
                    case Operation::Addition:
                        if (lhsOperandEvaluated.has_value() && *lhsOperandEvaluated == 0) {
                            return rhsOperandEvaluated;
                        }
                        if (rhsOperandEvaluated.has_value() && *rhsOperandEvaluated == 0) {
                            return lhsOperandEvaluated;
                        }
                        return lhsOperandEvaluated.has_value() && rhsOperandEvaluated.has_value() ? std::make_optional(*lhsOperandEvaluated + *rhsOperandEvaluated) : std::nullopt;
                    case Operation::Subtraction:
                        if (rhsOperandEvaluated.has_value() && *rhsOperandEvaluated == 0) {
                            return lhsOperandEvaluated;
                        }
                        return lhsOperandEvaluated.has_value() && rhsOperandEvaluated.has_value() ? std::make_optional(*lhsOperandEvaluated - *rhsOperandEvaluated) : std::nullopt;
                    case Operation::Multiplication: {
                        if ((lhsOperandEvaluated.has_value() && *lhsOperandEvaluated == 0) || (rhsOperandEvaluated.has_value() && *rhsOperandEvaluated == 0)) {
                            return 0;
                        }
                        return lhsOperandEvaluated.has_value() && rhsOperandEvaluated.has_value() ? std::make_optional(*lhsOperandEvaluated * *rhsOperandEvaluated) : std::nullopt;
                    }
                    case Operation::Division:
                        if (!lhsOperandEvaluated.has_value() || (rhsOperandEvaluated.has_value() && *rhsOperandEvaluated == 0)) {
                            return std::nullopt;
                        }
                        if (lhsOperandEvaluated.has_value() && *lhsOperandEvaluated == 0) {
                            return 0;
                        }
                        return rhsOperandEvaluated.has_value() ? std::make_optional(*lhsOperandEvaluated / *rhsOperandEvaluated) : std::nullopt;
                }
                return std::nullopt;
            }
        };

        explicit Number(std::variant<unsigned, std::string, ConstantExpression> number):
            numberVar(std::move(number)) {}

        explicit Number(unsigned value):
            numberVar(value) {
        }

        explicit Number(const std::string& value):
            numberVar(value) {
        }

        explicit Number(ConstantExpression value):
            numberVar(std::move(value)) {
        }

        ~Number() = default;

        [[nodiscard]] bool isLoopVariable() const {
            return std::holds_alternative<std::string>(numberVar);
        }

        [[nodiscard]] bool isConstant() const {
            return std::holds_alternative<unsigned>(numberVar);
        }

        [[nodiscard]] bool isConstantExpression() const {
            return std::holds_alternative<ConstantExpression>(numberVar);
        }

        [[nodiscard]] const std::string& variableName() const {
            return std::get<std::string>(numberVar);
        }

        [[nodiscard]] std::optional<ConstantExpression> constantExpression() const {
            return std::get<ConstantExpression>(numberVar);
        }

        [[nodiscard]] unsigned evaluate(const LoopVariableMapping& map) const {
            return std::visit(Overloaded{
                                      [](unsigned arg) { return arg; },
                                      [&map](const std::string& value) { return map.find(value)->second; },
                                      [&map](const ConstantExpression& constantExpression) { return constantExpression.tryEvaluate(map).value(); }},
                              numberVar);
        }

        [[nodiscard]] std::optional<unsigned> tryEvaluate(const LoopVariableMapping& loopVariableValueLookup) const {
            if (isConstant()) {
                return std::get<unsigned>(numberVar);
            }
            if (isLoopVariable() && loopVariableValueLookup.count(std::get<std::string>(numberVar)) > 0) {
                return loopVariableValueLookup.at(std::get<std::string>(numberVar));
            }
            if (isConstantExpression()) {
                return std::get<ConstantExpression>(numberVar).tryEvaluate(loopVariableValueLookup);
            }
            return std::nullopt;
        }

    private:
        std::variant<unsigned, std::string, ConstantExpression> numberVar;
    };

} // namespace syrec
