#if 1
/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "Action.h"
#include "Machine.h"
#include "Utils.h"
#include "catch.hpp"
#include <chrono>
#include <cstring>
#include <deque>
#include <iostream>
#include <type_traits>
#include <unistd.h>

using namespace std::string_literals;
using namespace std::chrono_literals;
using namespace ls;

struct Instrumentation {
        template <typename... Args> static void log (Args &&... args) {}
};

/**
 * Machine instance and a few features tested.
 */
TEST_CASE ("Check if all called", "[Action]")
{
        std::vector<std::string> results;

        // TODO this worked, and stoped to compile after introducing ErasedTransitions
        // TODO it work again only because I explicitlu implemented copy constructors
        auto res = [&results] (std::string const &message) { return [&results, message] (auto /* a */) { results.push_back (message); }; };
        // auto res = [&results] (std::string message) { return [&results, message] () { results.push_back (message); }; };
        auto eq = [] (int what) { return [what] (auto const &i) { return i == what; }; };

        auto m = machine (
                state ("INIT"_STATE, entry (res ("INIT entry")), exit (res ("INIT exit")),
                       transition ("B"_STATE, eq (2), res ("INIT->B action"))),

                state ("B"_STATE, entry (res ("B entry")), exit (res ("B exit")),
                       transition ("C"_STATE, eq (3), res ("B->C action1"), res ("B->C action2"))),

                state ("C"_STATE, entry (res ("C entry")), exit (res ("C exit")), transition ("B"_STATE, eq (5), res ("C->B action")),
                       transition ("FINAL"_STATE, eq (4), res ("C->FINAL action"))),

                state ("FINAL"_STATE, entry (res ("FINAL entry")), exit (res ("")), transition ("FINAL"_STATE, eq (77), res ("C->FINAL action")))

        );

        m.run (0);
        REQUIRE (m.getCurrentStateIndex () == "INIT"_STATE.getIndex ());
        REQUIRE (results.at (0) == "INIT entry");
        REQUIRE (results.size () == 1);

        m.run (-1); // Nothing should happen
        REQUIRE (m.getCurrentStateIndex () == "INIT"_STATE.getIndex ());
        REQUIRE (results.size () == 1);

        m.run (2); // State is successfully changed to "B"_STATE.
        REQUIRE (m.getCurrentStateIndex () == "B"_STATE.getIndex ());

        // TODO
        REQUIRE (results.size () == 4);
        REQUIRE (results.at (1) == "INIT exit");
        REQUIRE (results.at (2) == "INIT->B action");
        REQUIRE (results.at (3) == "B entry");

        m.run (3);
        REQUIRE (m.getCurrentStateIndex () == "C"_STATE.getIndex ());
        REQUIRE (results.at (4) == "B exit");
        REQUIRE (results.at (5) == "B->C action1");
        REQUIRE (results.at (6) == "B->C action2");
        REQUIRE (results.at (7) == "C entry");
        REQUIRE (results.size () == 8);

        m.run (5);
        REQUIRE (m.getCurrentStateIndex () == "B"_STATE.getIndex ());
        REQUIRE (results.at (8) == "C exit");
        REQUIRE (results.at (9) == "C->B action");
        REQUIRE (results.at (10) == "B entry");
        REQUIRE (results.size () == 11);

        m.run (3);
        REQUIRE (m.getCurrentStateIndex () == "C"_STATE.getIndex ());
        REQUIRE (results.at (11) == "B exit");
        REQUIRE (results.at (12) == "B->C action1");
        REQUIRE (results.at (13) == "B->C action2");
        REQUIRE (results.at (14) == "C entry");
        REQUIRE (results.size () == 15);

        m.run (4); // Transition condition is satisfied.
        REQUIRE (m.getCurrentStateIndex () == "FINAL"_STATE.getIndex ());
        REQUIRE (results.at (15) == "C exit");
        REQUIRE (results.at (16) == "C->FINAL action");
        REQUIRE (results.at (17) == "FINAL entry");
        REQUIRE (results.size () == 18);
}
#endif