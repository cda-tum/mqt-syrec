/**
 * @file module.hpp
 *
 * @brief SyReC module data type
 */
#ifndef MODULE_HPP
#define MODULE_HPP

#include "core/syrec/statement.hpp"
#include "core/syrec/variable.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace syrec {

    class statement;

    /**
     * @brief SyReC module data type
     *
     * This class represents a SyReC module. It containes of a name(), parameters(),
     * local variables(), and a list of statements().
     *

     */
    class module {
    public:
        /**
       * @brief Smart pointer
       *


       */
        typedef std::shared_ptr<module> ptr;

        /**
       * @brief Vector of smart pointers
       *


       */
        typedef std::vector<ptr> vec;

        /**
       * @brief Constructor
       *
       * Initializes a module with a name
       *
       * @param name Name of the module
       *


       */
        explicit module(const std::string& name);

        /**
       * @brief Deconstructor
       */
        ~module();

        /**
       * @brief Returns the name of the module
       *
       * @return Name
       *


       */
        [[nodiscard]] const std::string& name() const;

        /**
       * @brief Adds a parameter to the module
       *
       * @param parameter Parameter
       *


       */
        void add_parameter(const variable::ptr& parameter);

        /**
       * @brief Returns all parameters of the module
       *
       * @return Vector of parameters
       *


       */
        [[nodiscard]] const variable::vec& parameters() const;

        /**
       * @brief Returns all variables of the module
       *
       * @return Vector of variables
       *


       */
        [[nodiscard]] const variable::vec& variables() const;

        /**
       * @brief Finds a parameter or variable in the module
       *
       * This methods tries to find a parameter or a variable
       * by its name. If no such parameter or variable exists,
       * then the empty smart pointer variable::ptr() is returned.
       * Otherwise, using the \ref variable::type() "type" it can
       * be determined, whether it is a parameter of a variable.
       *


       */
        [[nodiscard]] variable::ptr find_parameter_or_variable(const std::string& name) const;

        /**
       * @brief Adds a statement to the module
       *
       * @param statement Statement
       *


       */
        void add_statement(const std::shared_ptr<statement>& statement);

        /**
       * @brief Returns all statements of the module
       *
       * @return Vector of statements
       *


       */
        [[nodiscard]] const std::vector<std::shared_ptr<statement>>& statements() const;

    private:
        class priv;
        priv* const d = nullptr;
    };

} // namespace syrec

#endif /* MODULE_HPP */
