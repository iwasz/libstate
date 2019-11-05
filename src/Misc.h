/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#pragma once
#include <chrono>

namespace ls {
/// TODO typesafe api, no uint32
class Timer {
public:
        Timer (uint32_t intervalMs = 0) { start (intervalMs); }

        /// Resets the timer (it starts from 0) and sets the interval. So isExpired will return true after whole interval has passed.
        void start (uint32_t intervalMs)
        {
                this->intervalMs = intervalMs;
                this->startTime = getTick ();
        }

        /// Change interval without reseting the timer. Can extend as well as shorten.
        void extend (uint32_t intervalMs) { this->intervalMs = intervalMs; }

        /// Says if intervalMs has passed since start () was called.
        bool isExpired () const { return elapsed () >= intervalMs; }

        /// Returns how many ms has passed since start () was called.
        uint32_t elapsed () const
        {
                uint32_t actualTime = getTick ();
                return actualTime - startTime;
        }

        /// Convenience method, simple delay ms.
        static void delay (uint32_t delayMs)
        {
                Timer t{delayMs};
                while (!t.isExpired ()) {
                }
        }

        /// Returns system wide ms since system start.
        static uint32_t getTick ()
        {
                using namespace std::chrono;
                auto timePoint = steady_clock::now ();
                auto duration = duration_cast<milliseconds> (timePoint.time_since_epoch ());
                return duration.count ();
        }

private:
        uint32_t startTime = 0;
        uint32_t intervalMs = 0;
};
} // namespace ls