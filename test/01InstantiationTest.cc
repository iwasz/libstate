/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "Machine.h"
#include "Utils.h"
#include "catch.hpp"
#include <chrono>
#include <cstring>
#include <deque>
#include <iostream>
#include <type_traits>
#include <unistd.h>

/****************************************************************************/

using namespace std::string_literals;
using namespace std::chrono_literals;
using namespace ls;

/****************************************************************************/

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
        // std::string s = "ala ma kota";
        // std::cout << s << std::endl;

        auto m = machine (
                // TODO Full, many actions!
                // state ("INIT"_STATE, entry (At ("A"), At ("bla")), exit (At ("A"), At ("bla")),
                //        transition (
                //                "B"_STATE, [] (int i) { return i == 2; }, At ("bla"), At ("bla"))),

                // Full single actions
                state ("INIT"_STATE, entry (At ("bla")), exit (At ("bla")),
                       transition (
                               "B"_STATE, [] (int i) { return i == 2; }, At ("bla"))),

                // Transition without actions
                state ("B"_STATE, entry (At ("Z")), exit (At ("A")), transition ("C"_STATE, [] (int i) { return i == 3; })),

                // TODO Transition without actions and without condition (always true)
                // state ("C"_STATE, entry (At ("Z")), exit (At ("DT")), transition ("D"_STATE)),

                // TODO Transition WITH actions but without condition (always true)
                // state ("D"_STATE, entry (At ("Z")), exit (At ("DT")), transition ("FINAL"_STATE), At ("Z")),

                state ("FINAL"_STATE, entry (At ("Z")), exit (At ("DT")),
                       transition (
                               ""_STATE, [] (int ev) { return false; }, At ("")))

        );

        m.run (0);
}

/**
 * Machine instance and a few features tested.
 */
TEST_CASE ("Machine instance", "[Instantiation]")
{
        auto m = machine (state ("INIT"_STATE, entry ([] (int event) { std::cout << "1st entry [" << event << "]" << std::endl; }),
                                 exit ([] (int) { std::cout << "1st exit" << std::endl; }),
                                 transition (
                                         "B"_STATE, [] (int i) { return i == 2; }, At ("bla"))),

                          state ("B"_STATE, entry (At ("Z")), exit ([] (int) { std::cout << "exit in the middle" << std::endl; }),
                                 transition (
                                         "C"_STATE, [] (int i) { return i == 3; }, At ("A"), At ("B"))),

                          state ("C"_STATE, entry ([] {}), exit ([] (auto) {}),
                                 transition (
                                         "B"_STATE, [] (int ev) { return ev == 5; }, At ("")),
                                 transition (
                                         "FINAL"_STATE, [] (int ev) { return ev == 4; }, At ("Ble"))),

                          state ("FINAL"_STATE, entry ([] () {}, [] () {}), exit ([] (auto) {}, [] (auto) {}),
                                 transition (
                                         ""_STATE, [] (int ev) { return false; }, At ("")))

        );

        /*
         * We run the machine for the first time with two events in the queue. Both
         * events will be checked by the conditions. Because the transition of "INIT"_STATE
         * fires upon event '2', the state is changed.
         */
        m.run (0);
        REQUIRE (m.getCurrentStateIndex () == "INIT"_STATE.getIndex ());

        m.run (2);
        REQUIRE (m.getCurrentStateIndex () == "B"_STATE.getIndex ());

        m.run (3);
        REQUIRE (m.getCurrentStateIndex () == "C"_STATE.getIndex ());

        m.run (5);
        REQUIRE (m.getCurrentStateIndex () == "B"_STATE.getIndex ());

        m.run (3);
        REQUIRE (m.getCurrentStateIndex () == "C"_STATE.getIndex ());

        m.run (4);
        REQUIRE (m.getCurrentStateIndex () == "FINAL"_STATE.getIndex ());
}
