/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2011  The RevKit Developers <revkit@informatik.uni-bremen.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
