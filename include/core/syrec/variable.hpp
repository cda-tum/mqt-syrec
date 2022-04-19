/**
 * @file variable.hpp
 *
 * @brief SyReC variable data type
 */
#ifndef VARIABLE_HPP
#define VARIABLE_HPP

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
    struct variable {
        /**
       * @brief Type of variable
       */
        enum types {
            /**
         * @brief Module Input Parameter (garbage output)
         */
            in,

            /**
         * @brief Module Output Parameter (constant inputs with value 0)
         */
            out,

            /**
         * @brief Module Input/Output Parameter
         */
            inout,

            /**
         * @brief State Variable (local in top module)
         */
            state,

            /**
         * @brief Local variable (constant inputs with value 0 and garbage output)
         */
            wire
        };

        /**
       * @brief Smart pointer
       */
        typedef std::shared_ptr<variable> ptr;

        /**
       * @brief Vector of smart pointers
       */
        typedef std::vector<ptr> vec;

        variable(unsigned type, std::string name, std::vector<unsigned> dimensions, unsigned bitwidth);

        void set_reference(variable::ptr reference);

        unsigned              type{};
        std::string           name{};
        std::vector<unsigned> dimensions{};
        unsigned              bitwidth{};
        variable::ptr         reference = nullptr;
    };

    /**
     * @brief Variable Access
     *
     * This class represents the access of a variable inside
     * a statement or an expression.
     */
    struct variable_access {
        /**
       * @brief Smart pointer
       */
        typedef std::shared_ptr<variable_access> ptr;

        variable_access() = default;

        /**
       * @brief Sets the variable of this access
       *
       * @param var Variable
       */
        void set_var(variable::ptr var);

        /**
       * @brief Returns the variable of this access
       *
       * @return Variable
       */
        [[nodiscard]] variable::ptr get_var() const;

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

        variable::ptr                                      var{};
        std::optional<std::pair<number::ptr, number::ptr>> range{};
        std::vector<std::shared_ptr<expression>>           indexes{};
    };

} // namespace syrec

#endif /* VARIABLE_HPP */
