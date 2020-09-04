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

using namespace std::string_literals;
using namespace std::chrono_literals;
using namespace ls;

struct Instrumentation {
        template <typename... Args> static void log (Args &&... args) {}
};

TEST_CASE ("Action instantiation", "[Action]")
{
        auto res = [] (std::string const & /* message */) { return [] (auto /* a */) {}; };
        auto eq = [] (int what) { return [what] (auto const &i) { return i == what; }; };

        machine (state ("INIT"_ST, entry ([] () {}), exit ([] {}), transition ("B"_ST, eq (2), res ("aa"))), // 3

                 state ("INIT"_ST, exit ([] {}), transition ("B"_ST, eq (2), res ("aa"))),     // 2
                 state ("INIT"_ST, entry ([] () {}), transition ("B"_ST, eq (2), res ("aa"))), // 2
                 state ("INIT"_ST, entry ([] () {}), exit ([] {})),                            // 2

                 state ("INIT"_ST, entry ([] () {})),                        // 1
                 state ("INIT"_ST, exit ([] {})),                            // 1
                 state ("INIT"_ST, transition ("B"_ST, eq (2), res ("aa"))), // 1

                 state ("INIT"_ST), // 0

                 state ("TRA"_ST, transition ("B"_ST, eq (2), res ("aa"))), // 2
                 state ("TRA"_ST, transition ("B"_ST, eq (2)))              // 2
                                                                            //  state ("TRA"_ST, transition ("B"_ST))                      // 2

        );
}

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
                state ("INIT"_ST, entry (res ("INIT entry")), exit (res ("INIT exit")), transition ("B"_ST, eq (2), res ("INIT->B action"))),

                state ("B"_ST, entry (res ("B entry")), exit (res ("B exit")),
                       transition ("C"_ST, eq (3), res ("B->C action1"), res ("B->C action2"))),

                state ("C"_ST, entry (res ("C entry")), exit (res ("C exit")), transition ("B"_ST, eq (5), res ("C->B action")),
                       transition ("FINAL"_ST, eq (4), res ("C->FINAL action"))),

                state ("FINAL"_ST, entry (res ("FINAL entry")), exit (res ("")), transition ("FINAL"_ST, eq (77), res ("C->FINAL action")))

        );

        m.run (0);
        REQUIRE (m.getCurrentStateIndex () == "INIT"_ST.getIndex ());
        REQUIRE (results.at (0) == "INIT entry");
        REQUIRE (results.size () == 1);

        m.run (-1); // Nothing should happen
        REQUIRE (m.getCurrentStateIndex () == "INIT"_ST.getIndex ());
        REQUIRE (results.size () == 1);

        m.run (2); // State is successfully changed to "B"_ST.
        REQUIRE (m.getCurrentStateIndex () == "B"_ST.getIndex ());

        // TODO
        REQUIRE (results.size () == 4);
        REQUIRE (results.at (1) == "INIT exit");
        REQUIRE (results.at (2) == "INIT->B action");
        REQUIRE (results.at (3) == "B entry");

        m.run (3);
        REQUIRE (m.getCurrentStateIndex () == "C"_ST.getIndex ());
        REQUIRE (results.at (4) == "B exit");
        REQUIRE (results.at (5) == "B->C action1");
        REQUIRE (results.at (6) == "B->C action2");
        REQUIRE (results.at (7) == "C entry");
        REQUIRE (results.size () == 8);

        m.run (5);
        REQUIRE (m.getCurrentStateIndex () == "B"_ST.getIndex ());
        REQUIRE (results.at (8) == "C exit");
        REQUIRE (results.at (9) == "C->B action");
        REQUIRE (results.at (10) == "B entry");
        REQUIRE (results.size () == 11);

        m.run (3);
        REQUIRE (m.getCurrentStateIndex () == "C"_ST.getIndex ());
        REQUIRE (results.at (11) == "B exit");
        REQUIRE (results.at (12) == "B->C action1");
        REQUIRE (results.at (13) == "B->C action2");
        REQUIRE (results.at (14) == "C entry");
        REQUIRE (results.size () == 15);

        m.run (4); // Transition condition is satisfied.
        REQUIRE (m.getCurrentStateIndex () == "FINAL"_ST.getIndex ());
        REQUIRE (results.at (15) == "C exit");
        REQUIRE (results.at (16) == "C->FINAL action");
        REQUIRE (results.at (17) == "FINAL entry");
        REQUIRE (results.size () == 18);
}

TEST_CASE ("Instrumentation", "[Action]")
{
        std::vector<std::string> results;

        // TODO this worked, and stoped to compile after introducing ErasedTransitions
        // TODO it work again only because I explicitlu implemented copy constructors
        auto res = [&results] (std::string const &message) { return [&results, message] (auto /* a */) { results.push_back (message); }; };
        // auto res = [&results] (std::string message) { return [&results, message] () { results.push_back (message); }; };
        auto eq = [] (int what) { return [what] (auto const &i) { return i == what; }; };

        struct Instr {
                Instr (std::vector<std::string> &r) : results (r) {}
                void onStateChange (gsl::czstring<> currentStateName, unsigned int /* currentStateIndex */)
                {
                        results.emplace_back (currentStateName);
                }

                std::vector<std::string> &results;
        };

        Instr iii (results);

        auto m = machinei (
                iii,
                state ("INIT"_ST, entry (res ("INIT entry")), exit (res ("INIT exit")), transition ("B"_ST, eq (2), res ("INIT->B action"))),

                state ("B"_ST, entry (res ("B entry")), exit (res ("B exit")),
                       transition ("C"_ST, eq (3), res ("B->C action1"), res ("B->C action2"))),

                state ("C"_ST, entry (res ("C entry")), exit (res ("C exit")), transition ("B"_ST, eq (5), res ("C->B action")),
                       transition ("FINAL"_ST, eq (4), res ("C->FINAL action"))),

                state ("FINAL"_ST, entry (res ("FINAL entry")), exit (res ("")), transition ("FINAL"_ST, eq (77), res ("C->FINAL action")))

        );

        m.run (0);
        REQUIRE (m.getCurrentStateIndex () == "INIT"_ST.getIndex ());
        REQUIRE (results.at (0) == "INIT entry");
        REQUIRE (results.size () == 1);

        m.run (-1); // Nothing should happen
        REQUIRE (m.getCurrentStateIndex () == "INIT"_ST.getIndex ());
        REQUIRE (results.size () == 1);

        m.run (2); // State is successfully changed to "B"_ST.
        REQUIRE (m.getCurrentStateIndex () == "B"_ST.getIndex ());

        // TODO
        REQUIRE (results.size () == 5);
        REQUIRE (results.at (1) == "INIT exit");
        REQUIRE (results.at (2) == "INIT->B action");
        REQUIRE (results.at (3) == "B");
        REQUIRE (results.at (4) == "B entry");

        m.run (3);
        REQUIRE (m.getCurrentStateIndex () == "C"_ST.getIndex ());
        REQUIRE (results.at (5) == "B exit");
        REQUIRE (results.at (6) == "B->C action1");
        REQUIRE (results.at (7) == "B->C action2");
        REQUIRE (results.at (8) == "C");
        REQUIRE (results.at (9) == "C entry");
        REQUIRE (results.size () == 10);

        m.run (5);
        REQUIRE (m.getCurrentStateIndex () == "B"_ST.getIndex ());
        REQUIRE (results.at (10) == "C exit");
        REQUIRE (results.at (11) == "C->B action");
        REQUIRE (results.at (12) == "B");
        REQUIRE (results.at (13) == "B entry");
        REQUIRE (results.size () == 14);

        m.run (3);
        REQUIRE (m.getCurrentStateIndex () == "C"_ST.getIndex ());
        REQUIRE (results.at (14) == "B exit");
        REQUIRE (results.at (15) == "B->C action1");
        REQUIRE (results.at (16) == "B->C action2");
        REQUIRE (results.at (17) == "C");
        REQUIRE (results.at (18) == "C entry");
        REQUIRE (results.size () == 19);

        m.run (4); // Transition condition is satisfied.
        REQUIRE (m.getCurrentStateIndex () == "FINAL"_ST.getIndex ());
        REQUIRE (results.at (19) == "C exit");
        REQUIRE (results.at (20) == "C->FINAL action");
        REQUIRE (results.at (21) == "FINAL");
        REQUIRE (results.at (22) == "FINAL entry");
        REQUIRE (results.size () == 23);
}

/**
 * Basic concepts.
 */
TEST_CASE ("Basic concepts", "[Action]") {}
