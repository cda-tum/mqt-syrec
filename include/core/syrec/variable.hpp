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

    class expression;

    /**
     * @brief SyReC variable data type
     *
     * This class represents variable data type. A variable
     * can either be a parameter passed to a module, or a
     * local variable.
     *

     */
    class variable {
    public:
        /**
       * @brief Type of variable
       *


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
       *


       */
        typedef std::shared_ptr<variable> ptr;

        /**
       * @brief Vector of smart pointers
       *


       */
        typedef std::vector<ptr> vec;

        variable(unsigned type, const std::string& name, const std::vector<unsigned>& dimensions, unsigned bitwidth);

        /**
       * @brief Deconstructor
       *


       */
        ~variable();

        /**
       * @brief Returns the type of the variable
       *
       * @return Type
       *


       */
        [[nodiscard]] unsigned type() const;

        /**
       * @brief Returns the name of the variable
       *
       * @return Name
       *


       */
        [[nodiscard]] const std::string& name() const;

        /**
       * @brief Returns the bit-width of the variable
       *
       * @return Bit-width
       *


       */
        [[nodiscard]] unsigned bitwidth() const;

        void                        set_reference(variable::ptr reference);
        [[nodiscard]] variable::ptr reference() const;

        //[[maybe_unused]] void                      set_dimensions(const std::vector<unsigned>& dimensions);
        [[nodiscard]] const std::vector<unsigned>& dimensions() const;

    private:
        class priv;
        priv* const d = nullptr;
    };

    /**
     * @brief Variable Access
     *
     * This class represents the access of a variable inside
     * a statement or an expression.
     *

     */
    class variable_access {
    public:
        /**
       * @brief Smart pointer
       *


       */
        typedef std::shared_ptr<variable_access> ptr;

        /**
       * @brief Standard constructor
       *
       * Initializes default values
       *


       */
        variable_access();

        /**
       * @brief Deconstructor
       *


       */
        ~variable_access();

        /**
       * @brief Sets the variable of this access
       *
       * @param var Variable
       *


       */
        void set_var(variable::ptr var);

        /**
       * @brief Returns the variable of this access
       *
       * @return Variable
       *


       */
        [[nodiscard]] variable::ptr var() const;

        /**
       * @brief Sets the range of this access
       *
       * If the parameter is empty, i.e. the default vaule @code boost::optional<std::pair<number::ptr, number::ptr> >() @endcode
       * the full variable is considered. Otherwise, the values of the numbers are evaluated for determining the range.
       *
       * The first number can be larger than the second one. In this case, the range is also considered backwards.
       *
       * @param range Range
       *


       */
        void set_range(const std::optional<std::pair<number::ptr, number::ptr>>& range);

        /**
       * @brief Returns the range of this access
       *
       * @return Range
       *


       */
        [[nodiscard]] const std::optional<std::pair<number::ptr, number::ptr>>& range() const;

        /**
       * @brief Returns the bit-width of the variable access
       *
       * The bit-width can only be evaluated, if
       * - both bounds in the range are no loop variables, or
       * - both bounds in the range are the same loop variable.
       * In other cases, this message throws an assertion.
       *
       * @return Bit-width of the range, if possible to calculate
       *


       */
        [[nodiscard]] unsigned bitwidth() const;

        void                                                          set_indexes(const std::vector<std::shared_ptr<expression>>& indexes);
        [[nodiscard]] const std::vector<std::shared_ptr<expression>>& indexes() const;

    private:
        class priv;
        priv* const d = nullptr;
    };

} // namespace syrec

#endif /* VARIABLE_HPP */
