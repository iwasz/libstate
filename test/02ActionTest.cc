/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "Action.h"
#include "StateMachine.h"
#include "Utils.h"
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

/**
 * Machine instance and a few features tested.
 */
TEST_CASE ("Check if all called", "[Action]")
{
        using namespace hana::literals;

        std::vector<std::string> results;

        auto res = [&results] (std::string const &message) { return [&results, message] { results.push_back (message); }; };
        auto eq = [] (int what) { return [what] (auto const &i) { return i == what; }; };

        auto m = machine (state ("INIT"_STATE, entry (res ("INIT entry")), exit (res ("INIT exit")),
                                 transition ("B"_STATE, eq (2), res ("INIT->B action"))),

                          state ("B"_STATE, entry (res ("B entry")), exit (res ("B exit")),
                                 transition ("C"_STATE, eq (3), res ("B->C action1"), res ("B->C action2"))),

                          state ("C"_STATE, entry (res ("C entry")), exit (res ("C exit")), transition ("B"_STATE, eq (5), res ("C->B action")),
                                 transition ("FINAL"_STATE, eq (4), res ("C->FINAL action"))),

                          state ("FINAL"_STATE, entry (res ("FINAL entry")))

        );

        // TODO this should be called with ampty queue, but it does not work, because checks are inside loop iterating over events.
        m.run (std::deque<int>{2});
        REQUIRE (m.getCurrentStateName ());
        REQUIRE (*m.getCurrentStateName () == std::type_index (typeid ("INIT"_STATE)));
        REQUIRE (results.at (0) == "INIT entry");

        m.run (std::deque{2}); // State is successfully changed to "B"_STATE.
        REQUIRE (m.getCurrentStateName ());
        REQUIRE (*m.getCurrentStateName () == std::type_index (typeid ("B"_STATE)));
        REQUIRE (results.at (1) == "INIT exit");
        REQUIRE (results.at (2) == "INIT->B action");
        REQUIRE (results.at (3) == "B entry");
        REQUIRE (results.size () == 4);

        m.run (std::deque{3});
        REQUIRE (*m.getCurrentStateName () == std::type_index (typeid ("C"_STATE)));
        REQUIRE (results.at (4) == "B exit");
        REQUIRE (results.at (5) == "B->C action1");
        REQUIRE (results.at (6) == "B->C action2");
        REQUIRE (results.at (7) == "C entry");
        REQUIRE (results.size () == 8);

        m.run (std::deque{5});
        REQUIRE (*m.getCurrentStateName () == std::type_index (typeid ("B"_STATE)));
        REQUIRE (results.at (8) == "C exit");
        REQUIRE (results.at (9) == "C->B action");
        REQUIRE (results.at (10) == "B entry");
        REQUIRE (results.size () == 11);

        m.run (std::deque{3});
        REQUIRE (*m.getCurrentStateName () == std::type_index (typeid ("C"_STATE)));
        REQUIRE (results.at (11) == "B exit");
        REQUIRE (results.at (12) == "B->C action1");
        REQUIRE (results.at (13) == "B->C action2");
        REQUIRE (results.at (14) == "C entry");
        REQUIRE (results.size () == 15);

        m.run (std::deque{4, 7}); // Transition condition is satisfied.
        REQUIRE (*m.getCurrentStateName () == std::type_index (typeid ("FINAL"_STATE)));
        REQUIRE (results.at (15) == "C exit");
        REQUIRE (results.at (16) == "C->FINAL action");
        REQUIRE (results.at (17) == "FINAL entry");
        REQUIRE (results.size () == 18);
}