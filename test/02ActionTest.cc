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

using namespace std::string_literals;
using namespace std::chrono_literals;
using namespace ls;

struct Instrumentation {
        template <typename... Args> static void log (Args &&...args) {}
};

/**
 * Machine instance and a few features tested.
 */

TEST_CASE ("Instrumentation", "[Action]")
{
        std::vector<std::string> results;
        using namespace std::string_literals;

        auto eq = [] (int what) { return [what] (auto const &i) { return i == what; }; };

        struct Instr {
                Instr (std::vector<std::string> &r) : results (r) {}

                void onEntry (const char *currentStateName, unsigned int /* currentStateIndex */)
                {
                        results.emplace_back (currentStateName + " entry"s);
                }

                void onExit (const char *currentStateName, unsigned int /* currentStateIndex */, int acceptedTransNumber)
                {
                        results.emplace_back (currentStateName + " exit "s + std::to_string (acceptedTransNumber));
                }

                std::vector<std::string> &results;
        };

        Instr iii (results);

        auto m = machine (iii, state ("INIT"_ST, transition ("B"_ST, eq (2))),

                          state ("B"_ST, transition ("C"_ST, eq (3)), transition ("C"_ST, eq (66))

                                         ),

                          state ("C"_ST, transition ("B"_ST, eq (5), [] (auto) {}), //
                                 transition ("FINAL"_ST, eq (4), [] (auto) {})),

                          state ("FINAL"_ST, transition ("FINAL"_ST, eq (77), [] (auto) {}))

        );

        m.run (0);
        CHECK (m.getCurrentStateIndex () == "INIT"_ST.getIndex ());
        CHECK (results.at (0) == "INIT entry");
        CHECK (results.size () == 1);

        m.run (-1); // Nothing should happen
        CHECK (m.getCurrentStateIndex () == "INIT"_ST.getIndex ());
        REQUIRE (results.size () == 1);

        m.run (2); // State is successfully changed to "B"_ST.
        CHECK (m.getCurrentStateIndex () == "B"_ST.getIndex ());

        REQUIRE (results.size () == 3);
        CHECK (results.at (1) == "INIT exit 0");
        CHECK (results.at (2) == "B entry");

        m.run (66);
        CHECK (m.getCurrentStateIndex () == "C"_ST.getIndex ());
        REQUIRE (results.size () == 5);
        CHECK (results.at (3) == "B exit 1");
        CHECK (results.at (4) == "C entry");

        m.run (4);
        CHECK (m.getCurrentStateIndex () == "FINAL"_ST.getIndex ());
        REQUIRE (results.size () == 7);
        CHECK (results.at (5) == "C exit 1");
        CHECK (results.at (6) == "FINAL entry");

        m.run (3);
        CHECK (m.getCurrentStateIndex () == "FINAL"_ST.getIndex ());
        REQUIRE (results.size () == 7);

        m.run (77); // Transition condition is satisfied.
        CHECK (m.getCurrentStateIndex () == "FINAL"_ST.getIndex ());
        REQUIRE (results.size () == 9);
        CHECK (results.at (7) == "FINAL exit 0");
        CHECK (results.at (8) == "FINAL entry");
}

TEST_CASE ("Instrumentation by ref", "[Action]")
{
        using namespace std::string_literals;

        auto eq = [] (int what) { return [what] (auto const &i) { return i == what; }; };

        struct Instr {

                void onEntry (const char *currentStateName, unsigned int /* currentStateIndex */)
                {
                        results.emplace_back (currentStateName + " entry"s);
                }

                void onExit (const char *currentStateName, unsigned int /* currentStateIndex */, int acceptedTransNumber)
                {
                        results.emplace_back (currentStateName + " exit "s + std::to_string (acceptedTransNumber));
                }

                std::vector<std::string> results;
        };

        Instr iii;

        auto m = machine (std::ref (iii), state ("INIT"_ST, transition ("B"_ST, eq (2))),
                          state ("B"_ST, transition ("C"_ST, eq (3)), transition ("C"_ST, eq (66))),
                          state ("FINAL"_ST, transition ("FINAL"_ST, eq (77), [] (auto) {}))

        );

        m.run (0);
        CHECK (m.getCurrentStateIndex () == "INIT"_ST.getIndex ());
        CHECK (iii.results.at (0) == "INIT entry");
        CHECK (iii.results.size () == 1);
}

/**
 * Exception
 */
TEST_CASE ("Entry action exception", "[Action]")
{
        std::vector<std::string> results;

        auto res = [&results] (std::string const &message) { return [&results, message] (auto /* a */) { results.push_back (message); }; };
        auto eq = [] (int what) { return [what] (auto const &i) { return i == what; }; };

        auto m = machine (state ("INIT"_ST, entry (res ("INIT entry")), transition ("B"_ST, eq (2), res ("INIT->B action"))),

                          state ("B"_ST, entry ([&results] {
                                         results.push_back ("B entry");
                                         throw 1.0F;
                                 }),
                                 transition ("C"_ST, eq (3))),

                          state ("C"_ST, entry (res ("C entry")))

        );

        CHECK_NOTHROW (m.run (0));
        CHECK (m.getCurrentStateIndex () == "INIT"_ST.getIndex ());
        CHECK (results.at (0) == "INIT entry");
        CHECK (results.size () == 1);

        CHECK_THROWS (m.run (2));
        CHECK (m.getCurrentStateIndex () == "B"_ST.getIndex ());
        CHECK (results.size () == 3);
        CHECK (results.at (1) == "INIT->B action");
        CHECK (results.back () == "B entry");

        // Even thougfh an exception was thrown, we can proceed as usual. User has to deal with the exception.
        CHECK_NOTHROW (m.run (3));
        CHECK (m.getCurrentStateIndex () != "B"_ST.getIndex ());
        CHECK (m.getCurrentStateIndex () == "C"_ST.getIndex ());
        CHECK (results.size () == 4);
        CHECK (results.back () == "C entry");
}

TEST_CASE ("Exit action exception", "[Action]")
{
        // std::vector<std::string> results;

        // auto res = [&results] (std::string const &message) { return [&results, message] (auto /* a */) { results.push_back (message); }; };
        // auto eq = [] (int what) { return [what] (auto const &i) { return i == what; }; };

        // auto m = machine (state ("INIT"_ST, entry (res ("INIT entry")), transition ("B"_ST, eq (2), res ("INIT->B action"))),

        //                   state ("B"_ST, entry (res ("B entry")), exit ([&results] {
        //                                  results.push_back ("B exit");
        //                                  throw 1.0F;
        //                          }),
        //                          transition ("C"_ST, eq (3))), //
        //                   state ("C"_ST, entry (res ("C entry")))

        // );

        // CHECK_NOTHROW (m.run (0));
        // CHECK (m.getCurrentStateIndex () == "INIT"_ST.getIndex ());
        // CHECK (results.at (0) == "INIT entry");
        // CHECK (results.size () == 1);

        // CHECK_NOTHROW (m.run (2));
        // CHECK (m.getCurrentStateIndex () == "B"_ST.getIndex ());
        // CHECK (results.size () == 3);
        // CHECK (results.at (1) == "INIT->B action");
        // CHECK (results.back () == "B entry");

        // // Even thougfh an exception was thrown, we can proceed as usual. User has to deal with the exception.
        // CHECK_THROWS (m.run (3));
        // CHECK (m.getCurrentStateIndex () != "B"_ST.getIndex ());
        // CHECK (m.getCurrentStateIndex () == "C"_ST.getIndex ());
        // CHECK (results.size () == 4);
        // CHECK (results.back () == "C entry");
}

struct MyAction {

        bool operator() (auto const & /* event */) const
        {
                ++cnt;
                return true;
        }

        mutable int cnt{};
};

TEST_CASE ("Value vs reference", "[Action]")
{
        MyAction action;

        SECTION ("ByValue")
        {
                auto m = machine (state ("A"_ST, entry (action), transition ("B"_ST, True)),
                                  state ("B"_ST, entry (action), transition ("A"_ST, True)));

                CHECK_NOTHROW (m.run (0));
                CHECK (m.getCurrentStateIndex () == "A"_ST.getIndex ());

                CHECK_NOTHROW (m.run (2));
                CHECK (m.getCurrentStateIndex () == "B"_ST.getIndex ());

                CHECK_NOTHROW (m.run (3));
                CHECK (m.getCurrentStateIndex () == "A"_ST.getIndex ());

                /*
                 *  CNT is 0 because myTrue condition was copied both for A and B.
                 */
                CHECK (action.cnt == 0);
        }

        SECTION ("ByReference")
        {
                auto m = machine (
                        state ("A"_ST, entry (std::ref (action)), exit (std::ref (action)), transition ("B"_ST, True, std::ref (action))),
                        state ("B"_ST, entry (std::ref (action)), exit (std::ref (action)), transition ("A"_ST, True, std::ref (action))));

                CHECK_NOTHROW (m.run (0));
                CHECK (m.getCurrentStateIndex () == "A"_ST.getIndex ());

                CHECK_NOTHROW (m.run (2));
                CHECK (m.getCurrentStateIndex () == "B"_ST.getIndex ());

                CHECK_NOTHROW (m.run (3));
                CHECK (m.getCurrentStateIndex () == "A"_ST.getIndex ());

                /*
                 * CNT is 2 because we used std::ref so only 1 instance is ever created
                 */
                CHECK (action.cnt == 7); // because we get back to A
        }
}