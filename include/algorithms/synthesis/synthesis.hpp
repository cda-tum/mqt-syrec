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
 * @file synthesis.hpp
 *
 * @brief General Synthesis type definitions
 */

#ifndef SYNTHESIS_HPP
#define SYNTHESIS_HPP

#include <string>

#include <boost/function.hpp>

#include <core/circuit.hpp>
#include <core/functor.hpp>
#include <core/truth_table.hpp>

namespace revkit
{

  /**
   * @brief Functor for synthesis based on a truth table
   *
   * @author RevKit
   * @since  1.0
   */
  typedef functor<bool(circuit&, const binary_truth_table&)> truth_table_synthesis_func;

  /**
   * @brief Functor for synthesis based on a file-name (PLA or BLIF)
   *
   * @author RevKit
   * @since  1.0
   */
  typedef functor<bool(circuit&, const std::string&)> pla_blif_synthesis_func;

  /**
   * @brief Functor for embedding a binary truth table in place
   *
   * @author RevKit
   * @since  1.0
   */
  typedef functor<bool(binary_truth_table&, const binary_truth_table&)> embedding_func;

  /**
   * @brief Functor for decomposing a reversible circuit into a quantum circuit
   *
   * @author RevKit
   * @since  1.0
   */
  typedef functor<bool(circuit&, const circuit&)> decomposition_func;
}

#endif /* SYNTHESIS_HPP */
