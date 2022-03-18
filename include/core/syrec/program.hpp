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

    class program {
    public:
        program() = default;

        ~program() = default;

        void add_module(const module::ptr& module) {
            modules_vec.emplace_back(module);
        }

        [[nodiscard]] const module::vec& modules() const {
            return modules_vec;
        }

        [[nodiscard]] module::ptr find_module(const std::string& name) const {
            for (const module::ptr& p: modules_vec) {
                if (p->name() == name) {
                    return p;
                }
            }

            return {};
        }

    private:
        module::vec modules_vec;
    };

} // namespace syrec::applications

#endif /* PROGRAM_HPP */
