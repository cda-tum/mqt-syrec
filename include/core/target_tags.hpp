/**
 * @file target_tags.hpp
 *
 * @brief Predefined target type tags for common gate types
 *
 * @sa \ref sub_target_tags
 */

#ifndef TARGET_TAGS_HPP
#define TARGET_TAGS_HPP

#include "core/circuit.hpp"
#include "core/gate.hpp"

#include <any>
#include <memory>

namespace syrec {

    /**
   * @brief Target Tag for Toffoli gates.
   *
   * @sa \ref sub_target_tags
   *

   */
    struct toffoli_tag {};

    /**
   * @brief Target Tag for Fredkin gates.
   *
   * @sa \ref sub_target_tags
   *

   */
    struct fredkin_tag {};

    /**
   * @brief Returns whether a gate is a Toffoli gate
   *
   * @param g Gate
   * @return true, if \p g is a Toffoli gate
   *

   */
    bool is_toffoli(const gate& g);

    /**
   * @brief Returns whether a gate is a Fredkin gate
   *
   * @param g Gate
   * @return true, if \p g is a Fredkin gate
   *

   */
    bool is_fredkin(const gate& g);

} // namespace syrec

#endif /* TARGET_TAGS_HPP */
