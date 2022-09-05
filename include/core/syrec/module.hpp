#pragma once

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
     * This class represents a SyReC module. It consists of a name(), parameters(),
     * local variables(), and a list of statements().
     */
    struct module {
        /**
       * @brief Smart pointer
       */
        typedef std::shared_ptr<module> ptr;

        /**
       * @brief Vector of smart pointers
       */
        typedef std::vector<ptr> vec;

        /**
       * @brief Constructor
       *
       * Initializes a module with a name
       *
       * @param name Name of the module
       */
        explicit module(const std::string& name):
            name(name) {}

        /**
       * @brief Adds a parameter to the module
       *
       * @param parameter Parameter
       */
        void add_parameter(const variable::ptr& parameter) {
            parameters.emplace_back(parameter);
        }

        /**
       * @brief Finds a parameter or variable in the module
       *
       * This methods tries to find a parameter or a variable
       * by its name. If no such parameter or variable exists,
       * then the empty smart pointer variable::ptr() is returned.
       * Otherwise, using the \ref variable::type() "type" it can
       * be determined, whether it is a parameter of a variable.
       */
        [[nodiscard]] variable::ptr find_parameter_or_variable(const std::string& n) const {
            for (variable::ptr var: parameters) {
                if (var->name == n) {
                    return var;
                }
            }

            return {};
        }

        /**
       * @brief Adds a statement to the module
       *
       * @param statement Statement
       */
        void add_statement(const std::shared_ptr<statement>& statement) {
            statements.emplace_back(statement);
        }

        std::string    name{};
        variable::vec  parameters{};
        variable::vec  variables{};
        statement::vec statements{};
    };

} // namespace syrec
