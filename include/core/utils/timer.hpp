/**
 * @file timer.hpp
 *
 * @brief A generic way for measuring time
 */

#ifndef TIMER_HPP
#define TIMER_HPP

#include "core/properties.hpp"

#include <chrono>
#include <iostream>
#include <utility>

namespace syrec {

    /**
   * @brief Functor for the timer class which assigns the run-time to a property map
   *
   * This functor writes the \em runtime field of a property map
   * after the time was measured and is thus similar to the
   * reference_timer.
   *

   */
    struct properties_timer {
        /**
     * Result value of the reference_timer is double,
     * since it returns the value of the run-time
     * in the operator call. This is only useful, when using the intermediate measurement
     * in timer.
     *
     * @sa timer::operator()()
     *

     */
        typedef double result_type;

        /**
     * @brief Default constructor
     *
     * Available for delayed starting of the timer.
     *

     */
        properties_timer() = default;

        /**
     * @brief Default constructor
     *
     * @param _statistics A smart pointer to a statistics properties object. Can be empty.
     *

     */
        explicit properties_timer(properties::ptr _statistics):
            statistics(std::move(_statistics)) {}

        /**
     * @brief Saves the run-time to the \b runtime field of the statistics variable
     *
     * @param r The run-time
     *

     */
        double operator()(double r) const {
            if (statistics) {
                statistics->set("runtime", r);
            }
            return r;
        }

    private:
        properties::ptr statistics;
    };

    /**
   * @brief A generic timer class
   *
   * This class measures the time between its constructor and
   * its deconstructor is called. In the deconstructor a
   * functor is called which can be assigned in the constructor.
   *
   * Because of this design the code for which the run-time has
   * to be checked has to be enclosed as a block and the timer
   * needs to be initialized as local variable in the beginning
   * of the block, so that the deconstructor get called automatically
   * at the end of the block.
   *
   * The Outputter has to implement <b>operator()(double runtime) const</b>.
   *
   * Two functors are already implemented in the library:
   * - print_timer: Prints the run-time to a given output stream
   * - reference_timer: Assigns the run-time to a variable
   *
   * @section sec_example_timer Example
   *
   * This example demonstrates how to create a timer functor which
   * outputs the run-time to STDOUT and how to use the timer class
   * with that functor.
   *
   * It is important to specify a result_type which is the result of the
   * () operator.
   *
   * @sa timer::operator()()
   *
   * @code
   * #include <core/utils/timer.hpp>
   *
   * struct output_timer {
   *   typedef void result_type;
   *
   *   void operator()( double runtime ) const {
   *     std::cout << runtime << std::endl;
   *   }
   * };
   *
   * ...
   *
   * // some other code for which no time should be measured
   * {
   *   output_timer ot;
   *   syrec::timer<output_timer> t( ot );
   *   // code for which time should be measured
   * }
   * // some other code for which no time should be measured
   *
   * 
   * @endcode
   *

   */
    template<typename Outputter>
    class timer {
    public:
        /**
     * @brief Constructor which does not start measuring the time
     *
     * When delayed starting should be done (using start(const Outputter&)) this
     * constructor has to be used.
     *

     */
        timer():
            started(false) {
        }

        /**
     * @brief Constructor which starts measuring the time
     *
     * The \p outputter is copied and called in the
     * deconstructor with the measured time.
     *
     * @param outputter Functor which does something with the measured run-time
     *

     */
        explicit timer(const Outputter& outputter):
            p(outputter),
            started(true) {
            begin = std::chrono::steady_clock::now();
        }

        /**
     * @brief Delayed start
     *
     * There might be reasons when the starting of the measurement
     * should be delayed and not started with the constructor.
     * For this cases this method can be used.
     *
     * @param outputter Functor which does something with the measured run-time
     *

     */
        void start(const Outputter& outputter) {
            p       = outputter;
            started = true;
            begin   = std::chrono::steady_clock::now();
        }

        /**
     * @brief Intermediate Call of the timer functor
     *
     * This operator also returns the return type of the functor.
     * This is useful, when using the reference_timer which returns
     * the value of the reference in the functor call.
     *
     * @section sec_example_timer_intermediate Example
     * In this function the timer is used explicitly without a scope
     * and the start method as well as the intermediate operator.
     *
     * @code
     * double runtime;
     * reference_timer rt( &runtime );
     * timer<reference_timer> t;
     * // before starting the measurement
     * t.start( rt );
     * // calculate a bit
     * double intermediate_time = t();
     * // calculate a bit more
     * intermediate_time = t();
     * @endcode
     *
     * @return The result value of the functor operator (if available)
     *

     */
        typename Outputter::result_type operator()() const {
            assert(started);
            double runtime = (std::chrono::steady_clock::now() - begin).count();
            return p(runtime);
        }

        /**
     * @brief Deconstructor which stops measuring the time
     *
     * In this function time is measured again and the functor
     * is called with the runtime.
     *

     */
        virtual ~timer() {
            if (started) {
                operator()();
            }
        }

    private:
        decltype(std::chrono::steady_clock::now()) begin;
        Outputter                                  p; // NOTE: has to be copy
        bool                                       started;
    };

} // namespace syrec

#endif
