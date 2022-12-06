/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "Machine.h"
#include "Utils.h"
#include <catch2/catch.hpp>
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
                // state ("INIT"_ST, entry (At ("A"), At ("bla")), exit (At ("A"), At ("bla")),
                //        transition (
                //                "B"_ST, [] (int i) { return i == 2; }, At ("bla"), At ("bla"))),

                // Full single actions
                state ("INIT"_ST, entry (At ("bla")), exit (At ("bla")),
                       transition (
                               "B"_ST, [] (int i) { return i == 2; }, At ("bla"))),

                // Transition without actions
                state ("B"_ST, entry (At ("Z")), exit (At ("A")), transition ("C"_ST, [] (int i) { return i == 3; })),

                // State without transitions
                state ("C"_ST, entry (At ("Z")), exit (At ("A"))), // TODO this should not compile!

                state ("D"_ST, entry (At ("Z")),
                       transition (
                               ""_ST, [] (int ev) { return false; }, At (""))), // TODO this should not compile!

                /// No transition, no exit actions
                // state ("C"_ST, entry (At ("Z"))),

                // TODO Transition without actions and without condition (always true)
                // state ("C"_ST, entry (At ("Z")), exit (At ("DT")), transition ("D"_ST)),

                // TODO Transition WITH actions but without condition (always true)
                // state ("D"_ST, entry (At ("Z")), exit (At ("DT")), transition ("FINAL"_ST), At ("Z")),

                state ("FINAL"_ST, entry (At ("Z")), exit (At ("DT")),
                       transition (
                               ""_ST, [] (int ev) { return false; }, At ("")))

        );

        m.run (0);
}

/**
 * Machine instance and a few features tested.
 */
TEST_CASE ("Machine instance", "[Instantiation]")
{
        auto m = machine (state ("INIT"_ST, entry ([] (int event) { std::cout << "1st entry [" << event << "]" << std::endl; }),
                                 exit ([] (int) { std::cout << "1st exit" << std::endl; }),
                                 transition (
                                         "B"_ST, [] (int i) { return i == 2; }, At ("bla"))),

                          state ("B"_ST, entry (At ("Z")), exit ([] (int) { std::cout << "exit in the middle" << std::endl; }),
                                 transition (
                                         "C"_ST, [] (int i) { return i == 3; }, At ("A"), At ("B"))),

                          state ("C"_ST, entry ([] {}), exit ([] (auto) {}),
                                 transition (
                                         "B"_ST, [] (int ev) { return ev == 5; }, At ("")),
                                 transition (
                                         "FINAL"_ST, [] (int ev) { return ev == 4; }, At ("Ble"))),

                          state ("FINAL"_ST, entry ([] () {}, [] () {}), exit ([] (auto) {}, [] (auto) {}))

        );

        /*
         * We run the machine for the first time with two events in the queue. Both
         * events will be checked by the conditions. Because the transition of "INIT"_ST
         * fires upon event '2', the state is changed.
         */
        m.run (0);
        REQUIRE (m.getCurrentStateIndex () == "INIT"_ST.getIndex ());

        m.run (2);
        REQUIRE (m.getCurrentStateIndex () == "B"_ST.getIndex ());

        m.run (3);
        REQUIRE (m.getCurrentStateIndex () == "C"_ST.getIndex ());

        m.run (5);
        REQUIRE (m.getCurrentStateIndex () == "B"_ST.getIndex ());

        m.run (3);
        REQUIRE (m.getCurrentStateIndex () == "C"_ST.getIndex ());

        m.run (4);
        REQUIRE (m.getCurrentStateIndex () == "FINAL"_ST.getIndex ());
}

TEST_CASE ("Optional arguments", "[Instantiation]")
{
        auto m = machine (state ("INIT"_ST, entry ([] (int event) { std::cout << "1st entry [" << event << "]" << std::endl; }),
                                 exit ([] (int) { std::cout << "1st exit" << std::endl; }),
                                 transition (
                                         "B"_ST, [] (int i) { return i == 2; }, At ("bla"))),

                          state ("B"_ST, entry (At ("Z")),
                                 transition (
                                         "C"_ST, [] (int i) { return i == 3; }, At ("A"), At ("B"))),

                          state ("C"_ST,
                                 transition (
                                         "B"_ST, [] (int ev) { return ev == 5; }, At ("")),
                                 transition (
                                         "FINAL"_ST, [] (int ev) { return ev == 4; }, At ("Ble"))),

                          state ("FINAL"_ST)

        );

        /*
         * We run the machine for the first time with two events in the queue. Both
         * events will be checked by the conditions. Because the transition of "INIT"_ST
         * fires upon event '2', the state is changed.
         */
        m.run (0);
        REQUIRE (m.getCurrentStateIndex () == "INIT"_ST.getIndex ());

        m.run (2);
        REQUIRE (m.getCurrentStateIndex () == "B"_ST.getIndex ());

        m.run (3);
        REQUIRE (m.getCurrentStateIndex () == "C"_ST.getIndex ());

        m.run (5);
        REQUIRE (m.getCurrentStateIndex () == "B"_ST.getIndex ());

        m.run (3);
        REQUIRE (m.getCurrentStateIndex () == "C"_ST.getIndex ());

        m.run (4);
        REQUIRE (m.getCurrentStateIndex () == "FINAL"_ST.getIndex ());
}

TEST_CASE ("Type Traits", "[Instantiation]")
{
        auto eee = entry ([] {});

        static_assert (is_entry_action<decltype (eee)>::value);
        static_assert (!is_entry_action<int>::value);

        auto xxx = exit ([] {});

        static_assert (!is_entry_action<decltype (xxx)>::value);
        static_assert (is_exit_action<decltype (xxx)>::value);
        static_assert (!is_entry_action<int>::value);

        auto ss = state ("lkjsd"_ST);
        static_assert (!is_entry_action<decltype (xxx)>::value);
        static_assert (is_state<decltype (ss)>::value);
}