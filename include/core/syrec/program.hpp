/**
 * @file program.hpp
 *
 * @brief SyReC program
 */
#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include "core/syrec/module.hpp"

#include <vector>

namespace syrec::applications {

    /**
     * @brief SyReC program
     *
     * This class represents a SyReC program, which
     * is a collection of modules.
     *

     */
    class program {
    public:
        /**
       * @brief Standard constructor
       * 
       * Initializes default values
       *


       */
        program();

        /**
       * @brief Deconstructor
       *


       */
        ~program();

        /**
       * @brief Adds a module to the program
       * 
       * @param module Module
       *


       */
        void add_module(const module::ptr& module);

        /**
       * @brief Returns all modules of the program
       *
       * @return List of modules 
       *


       */
        [[nodiscard]] const module::vec& modules() const;

        /**
       * @brief Finds a module by its name
       * 
       * @param name Name of the module
       * 
       * @return Returns a smart pointer to the module if there is a module with the name \p name.
       *         Otherwise, the empty smart pointer is returned.
       *


       */
        [[nodiscard]] module::ptr find_module(const std::string& name) const;

    private:
        class priv;
        priv* const d = nullptr;
    };

} // namespace syrec::applications

#endif /* PROGRAM_HPP */
