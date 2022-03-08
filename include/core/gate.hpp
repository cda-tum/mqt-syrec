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
 * @file gate.hpp
 *
 * @brief Gate class
 */

#ifndef GATE_HPP
#define GATE_HPP

#include <algorithm>
#include <any>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <iostream>
#include <set>
#include <vector>

namespace syrec {

    struct transform_line;
    struct filter_line;

    /**
   * @brief Represents a gate in a circuit
   *

   */
    class gate {
    public:
        /**
     * @brief Vector type of gates
     *

     */
        typedef std::vector<gate> vector;

        /**
     * @brief Type for accessing the line (line index)
     *

     */
        typedef unsigned line;

        /**
     * @brief Container for storing lines
     *

     */
        typedef std::set<line> line_container;

        /**
     * @brief Mutable Iterator for iterating through control or target lines
     *

     */
        typedef boost::transform_iterator<transform_line, boost::filter_iterator<filter_line, line_container::iterator>> iterator;

        /**
     * @brief Constant Iterator for iterating through control or target lines
     *

     */
        typedef boost::transform_iterator<transform_line, boost::filter_iterator<filter_line, line_container::const_iterator>> const_iterator;

    public:
        /**
     * @brief Default constructor
     *
     * Initializes private data
     *

     */
        gate();

        /**
     * @brief Copy Constructor
     *
     * Initializes private data and copies gate
     *
     * @param other Gate to be assigned
     *

     */
        gate(const gate& other);

        /**
     * @brief Default deconstructor
     *
     * Clears private data
     *

     */
        virtual ~gate();

        /**
     * @brief Assignment operator
     *
     * @param other Gate to be assigned
     *
     * @return Pointer to instance
     *

     */
        gate& operator=(const gate& other);

        /**
     * @brief Start iterator for accessing control lines. 
     *
     * Returns The start iterator of the line_container for accessing control lines. 
     *


     */
        [[nodiscard]] virtual const_iterator begin_controls() const;

        /**
     * @brief End iterator for accessing control lines (const).
     *
     * Returns The end iterator of the line_container for accessing control lines (const).
     *


     */
        [[nodiscard]] virtual const_iterator end_controls() const;

        /**
     * @brief Start iterator for accessing target lines (const).
     *
     * Returns The start iterator of the line_container for accessing target lines (const). 
     *


     */
        [[nodiscard]] virtual const_iterator begin_targets() const;

        /**
     * @brief End iterator for accessing target lines (const).
     *
     * Returns The end iterator of the line_container for accessing target lines (const). 
     *


     */
        [[nodiscard]] virtual const_iterator end_targets() const;

        /**
     * @brief Adds a control line to the gate
     *
     * @param c control line to add
     *


     */
        virtual void add_control(line c);

        /**
     * @brief Adds a target to the desired line
     *
     * @param l target line 
     *


     */
        virtual void add_target(line l);

        /**
     * @brief Sets the type of the target line(s)
     *
     * @param t target type
     *

     */
        virtual void set_type(const std::any& t);

        /**
     * @brief Returns the type of the target line(s)
     *
     * @return target type
     *

     */
        [[nodiscard]] virtual const std::any& type() const;

    private:
        struct priv;
        priv* const d;
    };

    struct transform_line {
        typedef gate::line result_type;

        transform_line() = default;

        explicit transform_line(const std::vector<unsigned>& filter):
            filter(&filter) {}

        gate::line operator()(gate::line l) const {
            return filter ? std::find(filter->begin(), filter->end(), l) - filter->begin() : l;
        }

    private:
        const std::vector<unsigned>* filter = nullptr;
    };
    /** @endcond */

    /** @cond */
    struct filter_line {
        filter_line() = default;

        explicit filter_line(const std::vector<unsigned>& filter):
            filter(&filter) {}

        bool operator()(const gate::line& l) const {
            return !filter || std::find(filter->begin(), filter->end(), l) != filter->end();
        }

    private:
        const std::vector<unsigned>* filter = nullptr;
    };
    /** @endcond */

    /**
   * @brief Gets the control lines of a gate
   *
   * This function stores all control lines of a gate into a container.
   *
   * @section Example
   * @code
   * gate g = ...;
   * std::vector<gate::line> controls;
   * control_lines( g, std::back_inserter( controls ) );
   * @endcode
   *
   * @param g      Gate
   * @param result Iterator to store the lines as gate::line type
   *
   * @return The iterator after adding the lines (pointing after the end)
   *

    */
    template<typename Iterator>
    Iterator control_lines(const gate& g, Iterator result) {
        for (gate::const_iterator c = g.begin_controls(); c != g.end_controls(); ++c) {
            *result++ = *c;
        }
        return result;
    }

    /**
   * @brief Gets the target lines of a gate
   *
   * This function stores all target lines of a gate into a container.
   *
   * @section Example
   * @code
   * gate g = ...;
   * std::vector<gate::line> targets;
   * target_lines( g, std::back_inserter( targets ) );
   * @endcode
   *
   * @param g      Gate
   * @param result Iterator to store the lines as gate::line type
   *
   * @return The iterator after adding the lines (pointing after the end)
   *

    */
    template<typename Iterator>
    Iterator target_lines(const gate& g, Iterator result) {
        for (gate::const_iterator c = g.begin_targets(); c != g.end_targets(); ++c) {
            *result++ = *c;
        }
        return result;
    }
} // namespace syrec

#endif /* GATE_HPP */
