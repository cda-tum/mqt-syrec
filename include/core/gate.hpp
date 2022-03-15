/**
 * @file gate.hpp
 *
 * @brief Gate class
 */

#ifndef GATE_HPP
#define GATE_HPP

#include <algorithm>
#include <any>
#include <iostream>
#include <set>
#include <vector>

namespace syrec {
    /**
   * @brief Represents a gate tags
   *

    */

    enum class gateType { None,
                          Fredkin,
                          Toffoli };

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
        typedef line_container::iterator iterator;
        /**
     * @brief Constant Iterator for iterating through control or target lines
     *

     */
        typedef line_container::const_iterator const_iterator;

    public:
        /**
     * @brief Default constructor
     *
     * Initializes private data
     *

     */
        gate();

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
        virtual void set_type(gateType t);

        /**
     * @brief Returns the type of the target line(s)
     *
     * @return target type
     *

     */
        [[nodiscard]] virtual gateType type() const;

    private:
        line_container controls;
        line_container targets;
        gateType       target_type;
    };

} // namespace syrec

#endif /* GATE_HPP */
