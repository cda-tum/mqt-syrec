#pragma once

#include "core/syrec/number.hpp"

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace syrec {

    struct expression;

    /**
     * @brief SyReC variable data type
     *
     * This class represents variable data type. A variable
     * can either be a parameter passed to a module, or a
     * local variable.
     */
    struct Variable {
        /**
       * @brief Type of variable
       */
        enum Types {
            /**
         * @brief Module Input Parameter (garbage output)
         */
            In,

            /**
         * @brief Module Output Parameter (constant inputs with value 0)
         */
            Out,

            /**
         * @brief Module Input/Output Parameter
         */
            Inout,

            /**
         * @brief State Variable (local in top module)
         */
            State,

            /**
         * @brief Local variable (constant inputs with value 0 and garbage output)
         */
            Wire
        };

        /**
       * @brief Smart pointer
       */
        using ptr = std::shared_ptr<Variable>;

        /**
       * @brief Vector of smart pointers
       */
        using vec = std::vector<ptr>;

        Variable(unsigned type, std::string name, std::vector<unsigned> dimensions, unsigned bitwidth);

        void setReference(Variable::ptr reference);

        unsigned              type{};
        std::string           name{};
        std::vector<unsigned> dimensions{};
        unsigned              bitwidth{};
        Variable::ptr         reference = nullptr;
    };

    /**
     * @brief Variable Access
     *
     * This class represents the access of a variable inside
     * a statement or an expression.
     */
    struct VariableAccess {
        /**
       * @brief Smart pointer
       */
        using ptr = std::shared_ptr<VariableAccess>;

        VariableAccess() = default;

        /**
       * @brief Sets the variable of this access
       *
       * @param var Variable
       */
        void setVar(Variable::ptr var);

        /**
       * @brief Returns the variable of this access
       *
       * @return Variable
       */
        [[nodiscard]] Variable::ptr getVar() const;

        /**
       * @brief Returns the bit-width of the variable access
       *
       * The bit-width can only be evaluated, if
       * - both bounds in the range are no loop variables, or
       * - both bounds in the range are the same loop variable.
       * In other cases, this message throws an assertion.
       *
       * @return Bit-width of the range, if possible to calculate
       */
        [[nodiscard]] unsigned bitwidth() const;

        Variable::ptr                                      var{};
        std::optional<std::pair<Number::ptr, Number::ptr>> range{};
        std::vector<std::shared_ptr<expression>>           indexes{};
    };

} // namespace syrec
