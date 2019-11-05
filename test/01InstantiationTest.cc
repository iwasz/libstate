/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "Action.h"
#include "StateMachine.h"
#include "catch.hpp"
#include <chrono>
#include <cstring>
//#include <etl/cstring.h>
#include <deque>
#include <iostream>
#include <type_traits>
#include <unistd.h>

namespace hana = boost::hana;
using namespace hana::literals;
using namespace std::string_literals;
using namespace std::chrono_literals;
using namespace ls;

// Nigdzie nie używam czegoś takiego, ale miło by było móc.
template <typename T> void actionFunctionTemplate (T &&event) { std::cout << "actionFunctionTemplate (" << event << ")" << std::endl; }

void actionFunction (std::string const &event) { std::cout << "actionFunction (" << event << ")" << std::endl; }

struct Class {
        Class (std::string v) : value (std::move (v)) {}

        template <typename T> void operator() (T &&ev) { std::cout << value << " (" << ev << ")" << std::endl; }

private:
        std::string value;
};

/**
 * This test only instantiates some bits of state machine and checks if it is even possible.
 * Does not do any REQUIRE checks.
 */
TEST_CASE ("First test", "[Instantiation]")
{
        State noAction ("A"_STATE);

        State state1 (
                "A"_STATE,
                entry ([] (auto &&ev) { std::cout << "Inline lambda action (" << ev << ")" << std::endl; }, // First entry action is a lambda
                       Class ("Class instance action"),                                                     // Second is a function object.
                       actionFunction,                                                                      // Third is a regular function.
                       actionFunctionTemplate<std::string const &>));                                       // Fourth is a function template.

        State state2 ("A"_STATE,
                      entry ([] (auto &&ev) { std::cout << "Inline lambda action (" << ev << ")" << std::endl; },
                             Class ("Class instance action"), actionFunction, actionFunctionTemplate<std::string const &>),
                      exit ([] (auto &&ev) { std::cout << "Inline lambda action (" << ev << ")" << std::endl; }, Class ("Class instance action"),
                            actionFunction, actionFunctionTemplate<std::string const &>));

        auto state3 = State ("A"_STATE, entry (At{"ATZ"}, At{"ATDT"}));

        state2.entry ("hello"s);
        state2.exit ("hello"s);

        state3.entry ("hello"s);
}

/**
 * Machine instantiated for the first time.
 */
TEST_CASE ("Machine instance", "[Instantiation]")
{
        using namespace hana::literals;

        auto m = machine (
                state ("INIT"_STATE,
                       entry (
                               // Action variant 1 : event passed as an argument, but no return value. Runs immediately.
                               [] (int event) { std::cout << "1st entry [" << event << "]" << std::endl; },
                               // Action variant 2 : the simplest. Just like above, but no argument.
                               [] () { std::cout << "2nd entry" << std::endl; },
                               // Action variant 3 : delays the machine operation.
                               [] (int event) {
                                       std::cout << "3rd entry" << std::endl;
                                       return 500ms;
                               },
                               // Action variant 4 : like the above, but no ragument.
                               [] () {
                                       std::cout << "4th entry" << std::endl;
                                       return 500ms;
                               }),
                       exit ([] { std::cout << "1st exit" << std::endl; }), transition ("B"_STATE, [] (int i) { return i == 2; })),

                state ("B"_STATE, entry (At ("Z")), exit ([] { return 10000us; }, [] { std::cout << "exit in the middle" << std::endl; }),
                       transition (
                               "C"_STATE, [] (int i) { return i == 3; }, At ("A"), At ("B"))),

                state ("C"_STATE, entry (At ("Z")), exit (At ("DT")), transition ("FINAL"_STATE, [] (int ev) { return ev == 4; })),

                state ("FINAL"_STATE, entry (At ("Z")), exit (At ("DT")))

        );

        /*
         * We run the machine for the first time with two events in the queue. Both
         * events will be checked by the conditions. Because the transition of "INIT"_STATE
         * fires upon event '2', the state is changed.
         */
        m.run (std::deque{1, 2});

        // State is successfully changed to "B"_STATE.
        REQUIRE (m.getCurrentStateName ());
        REQUIRE (*m.getCurrentStateName () == std::type_index (typeid ("B"_STATE)));

        std::deque deq{3};
        m.run (deq); // Transition condition is met, so run exit action which waits for 1000ms

        while (m.isWaiting ()) {
        }

        m.run (deq); // After the wait run following exit actions (if any) and change state.

        REQUIRE (*m.getCurrentStateName () == std::type_index (typeid ("C"_STATE)));

        m.run (std::deque{4, 5}); // Transition condition is satisfied.
        REQUIRE (*m.getCurrentStateName () == std::type_index (typeid ("FINAL"_STATE)));
}

template <typename T, typename = std::void_t<>> struct IsDelay : public std::false_type {
};

// template <typename T, std::void_t<decltype (std::is_same_v<std::invoke_result_t<T>, Delay>)>> struct IsDelay : public std::false_type {
// };
template <typename T> struct IsDelay<T, std::void_t<decltype (std::declval<T &> () ())>> : public std::true_type {
};

TEST_CASE ("Traits", "[Instantiation]")
{
        auto ac = [] () {};
        static_assert (IsDelay<decltype (ac)>::value);

        auto a = int ();
        static_assert (!IsDuration<decltype (a)>::value);

        auto ts = std::chrono::steady_clock::now ();
        auto d = ts.time_since_epoch ();
        static_assert (IsDuration<decltype (d)>::value);
}
