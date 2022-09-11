#pragma once

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
  */
    struct PropertiesTimer {
        /**
    * @brief Default constructor
    *
    * Available for delayed starting of the timer.
    */
        PropertiesTimer() = default;

        /**
    * @brief Default constructor
    *
    * @param _statistics A smart pointer to a statistics properties object. Can be empty.
    */
        explicit PropertiesTimer(Properties::ptr statistics):
            statistics(std::move(statistics)) {}

        /**
    * @brief Saves the run-time to the \b runtime field of the statistics variable
    *
    * @param r The run-time
    */
        void operator()(double r) const {
            if (statistics) {
                statistics->set("runtime", r);
            }
        }

    private:
        Properties::ptr statistics;
    };

    /**
  * @brief A generic timer class
  */

    template<typename Outputter>
    class Timer {
    public:
        /**
    * @brief Constructor which does not start measuring the time
    *
    * When delayed starting should be done (using start(const Outputter&)) this
    * constructor has to be used.
    */
        Timer() = default;

        /**
    * @brief function which starts measuring the time
    */

        void start(const Outputter& outputter) {
            p       = outputter;
            started = true;
            begin   = std::chrono::steady_clock::now();
        }

        /**
    * @brief function which stops measuring the time
    */

        void stop() {
            if (started) {
                assert(started);
                std::chrono::duration<double> runtime = (std::chrono::steady_clock::now() - begin);
                p(runtime.count());
            }
        }

    private:
        decltype(std::chrono::steady_clock::now()) begin;
        Outputter                                  p; // NOTE: has to be copy
        bool                                       started = false;
    };

} // namespace syrec
