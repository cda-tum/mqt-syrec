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

#include <boost/any.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <iostream>
#include <set>
#include <vector>

namespace revkit {

    struct transform_line;
    struct filter_line;
    class filtered_gate;

    /**
   * @brief Represents a gate in a circuit
   *
   * @author RevKit
   * @since  1.0
   */
    class gate {
    public:
        /**
     * @brief Vector type of gates
     *
     * @author RevKit
     * @since  1.0
     */
        typedef std::vector<gate> vector;

        /**
     * @brief Type for accessing the line (line index)
     *
     * @author RevKit
     * @since  1.0
     */
        typedef unsigned line;

        /**
     * @brief Container for storing lines
     *
     * @author RevKit
     * @since  1.0
     */
        typedef std::set<line> line_container;

        /**
     * @brief Mutable Iterator for iterating through control or target lines
     *
     * @author RevKit
     * @since  1.0
     */
        typedef boost::transform_iterator<transform_line, boost::filter_iterator<filter_line, line_container::iterator>> iterator;

        /**
     * @brief Constant Iterator for iterating through control or target lines
     *
     * @author RevKit
     * @since  1.0
     */
        typedef boost::transform_iterator<transform_line, boost::filter_iterator<filter_line, line_container::const_iterator>> const_iterator;

    public:
        /**
     * @brief Default constructor
     *
     * Initializes private data
     *
     * @author RevKit
     * @since  1.0
     */
        gate();

        /**
     * @brief Copy Constructor
     *
     * Initializes private data and copies gate
     *
     * @param other Gate to be assigned
     *
     * @author RevKit
     * @since  1.0
     */
        gate(const gate& other);

        /**
     * @brief Default deconstructor
     *
     * Clears private data
     *
     * @author RevKit
     * @since  1.0
     */
        virtual ~gate();

        /**
     * @brief Assignment operator
     *
     * @param other Gate to be assigned
     *
     * @return Pointer to instance
     *
     * @author RevKit
     * @since  1.0
     */
        gate& operator=(const gate& other);

        /**
     * @brief Start iterator for accessing control lines. 
     *
     * Returns The start iterator of the line_container for accessing control lines. 
     *
     * @author RevKit
     * @since 1.0
     */
        [[nodiscard]] virtual const_iterator begin_controls() const;

        /**
     * @brief End iterator for accessing control lines (const).
     *
     * Returns The end iterator of the line_container for accessing control lines (const).
     *
     * @author RevKit
     * @since 1.0
     */
        [[nodiscard]] virtual const_iterator end_controls() const;

        /**
     * @brief Start iterator for accessing control lines (non-const).
     *
     * Returns The start iterator of the line_container for accessing lines (non-const). 
     *
     * @author RevKit
     * @since 1.0
     */
        virtual iterator begin_controls();

        /**
     * @brief End iterator for accessing control lines (non-const).
     *
     * Returns the end iterator of the line_container for accessing control lines (non-const). 
     *
     * @author RevKit
     * @since 1.0
     */
        virtual iterator end_controls();

        /**
     * @brief Start iterator for accessing target lines (const).
     *
     * Returns The start iterator of the line_container for accessing target lines (const). 
     *
     * @author RevKit
     * @since 1.0
     */
        [[nodiscard]] virtual const_iterator begin_targets() const;

        /**
     * @brief End iterator for accessing target lines (const).
     *
     * Returns The end iterator of the line_container for accessing target lines (const). 
     *
     * @author RevKit
     * @since 1.0
     */
        [[nodiscard]] virtual const_iterator end_targets() const;

        /**
     * @brief Start iterator for accessing target lines (const).
     *
     * Returns The start iterator of the line_container for accessing target lines (const). 
     *
     * @author RevKit
     * @since 1.0
     */
        virtual iterator begin_targets();

        /**
     * @brief End iterator for accessing target lines (non-const).
     *
     * Returns The start iterator of the line_container for accessing target lines (non-const). 
     *
     * @author RevKit
     * @since 1.0
     */
        virtual iterator end_targets();

        /**
     * @brief Returns the number of control and target lines as sum
     *
     * This method returns the number of control and target
     * lines as sum and can be used for e.g. calculating costs.
     *
     * @author RevKit
     * @since  1.0
     *
     * @return Number of control and target lines.
     */
        [[nodiscard]] virtual unsigned size() const;

        /**
     * @brief Adds a control line to the gate
     *
     * @param c control line to add
     *
     * @author RevKit
     * @since 1.0 
     */
        virtual void add_control(line c);

        /**
     * @brief Remove control line to the gate
     *
     * @param c control line to remove
     *
     * @author RevKit
     * @since 1.0 
     */
        virtual void remove_control(line c);

        /**
     * @brief Adds a target to the desired line
     *
     * @param l target line 
     *
     * @author RevKit
     * @since 1.0
     */
        virtual void add_target(line l);

        /**
     * @brief Removes a target from the desired line
     *
     * @param l target line 
     *
     * @author RevKit
     * @since 1.0
     */
        virtual void remove_target(line l);

        /**
     * @brief Sets the type of the target line(s)
     *
     * @param t target type
     *
     * @author RevKit
     * @since  1.0
     */
        virtual void set_type(const boost::any& t);

        /**
     * @brief Returns the type of the target line(s)
     *
     * @return target type
     *
     * @author RevKit
     * @since  1.0
     */
        [[nodiscard]] virtual const boost::any& type() const;

        friend class filtered_gate;

    private:
        struct priv;
        priv* const d;
    };

    /**
   * @brief Wrapper for a gate to filter some lines
   *
   * This class wraps a underline \p base gate to just
   * access some lines which are specified in filter.
   *
   * You will never have to create a filtered_gate object
   * on your own, but you will get this as reference object
   * to your iterators in a subcircuit.
   *
   * @author RevKit
   * @since  1.0
   */
    class filtered_gate: public gate {
    public:
        /**
     * @brief Standard constructor
     *
     * Creates a filtered_gate from a base gate and
     * a list of indices which should be included in this
     * gate.
     * 
     * @param base   The underlying referenced gate
     * @param filter A vector with line indices which are included in this gate
     *
     * @author RevKit
     * @since  1.0
     */
        filtered_gate(gate& base, std::vector<unsigned>& filter);

        /**
     * @brief Copy constructor
     *
     * @param other Object to be copied from
     *
     * @author RevKit
     * @since  1.0
     */
        filtered_gate(const filtered_gate& other);

        /**
     * @brief Deconstructor
     */
        ~filtered_gate() override;

        /**
     * @brief Assignment operator
     *
     * @param other Gate to be assigned
     *
     * @return Pointer to instance
     *
     * @author RevKit
     * @since  1.0
     */
        filtered_gate& operator=(const filtered_gate& other);

        [[nodiscard]] const_iterator begin_controls() const override;
        [[nodiscard]] const_iterator end_controls() const override;
        iterator                     begin_controls() override;
        iterator                     end_controls() override;

        [[nodiscard]] const_iterator begin_targets() const override;
        [[nodiscard]] const_iterator end_targets() const override;
        iterator                     begin_targets() override;
        iterator                     end_targets() override;

        [[nodiscard]] unsigned size() const override;
        void                   add_control(line c) override;
        void                   remove_control(line c) override;
        void                   add_target(line l) override;
        void                   remove_target(line l) override;

        void                            set_type(const boost::any& t) override;
        [[nodiscard]] const boost::any& type() const override;

    private:
        struct priv;
        priv* const d;
    };

    /** @cond */
    struct transform_line {
        typedef gate::line result_type;

        transform_line() = default;

        explicit transform_line(const std::vector<unsigned>& filter):
            filter(&filter) {}

        gate::line operator()(gate::line l) const {
            return filter ? std::find(filter->begin(), filter->end(), l) - filter->begin() : l;
            return l;
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

} // namespace revkit

#endif /* GATE_HPP */
