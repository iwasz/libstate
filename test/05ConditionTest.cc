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
using namespace ls;

///
struct MyTrue {

        bool operator() (auto const & /* event */) const
        {
                ++cnt;
                return true;
        }

        mutable int cnt{};
};

/**
 *
 */
TEST_CASE ("Value vs reference cond", "[Condition]")
{
        MyTrue myTrue;

        SECTION ("ByValue")
        {
                auto m = machine (state ("A"_ST, entry ([] {}), transition ("B"_ST, myTrue)),
                                  state ("B"_ST, entry ([] {}), transition ("A"_ST, myTrue)));

                CHECK_NOTHROW (m.run (0));
                CHECK (m.getCurrentStateIndex () == "A"_ST.getIndex ());

                CHECK_NOTHROW (m.run (2));
                CHECK (m.getCurrentStateIndex () == "B"_ST.getIndex ());

                CHECK_NOTHROW (m.run (3));
                CHECK (m.getCurrentStateIndex () == "A"_ST.getIndex ());

                /*
                 *  CNT is 0 because myTrue condition was copied both for A and B.
                 */
                CHECK (myTrue.cnt == 0);
        }

        SECTION ("ByReference")
        {
                auto m = machine (state ("A"_ST, entry ([] {}), transition ("B"_ST, std::ref (myTrue))),
                                  state ("B"_ST, entry ([] {}), transition ("A"_ST, std::ref (myTrue))));

                CHECK_NOTHROW (m.run (0));
                CHECK (m.getCurrentStateIndex () == "A"_ST.getIndex ());

                CHECK_NOTHROW (m.run (2));
                CHECK (m.getCurrentStateIndex () == "B"_ST.getIndex ());

                CHECK_NOTHROW (m.run (3));
                CHECK (m.getCurrentStateIndex () == "A"_ST.getIndex ());

                /*
                 *  CNT is 2 because we used std::ref so only 1 instance is ever created
                 */
                CHECK (myTrue.cnt == 2);
        }
}

TEST_CASE ("Operators and references", "[Condition]")
{
        MyTrue myTrueA;
        MyTrue myTrueB;

        SECTION ("By value")
        {
                auto m = machine (state ("A"_ST, entry ([] {}), transition ("B"_ST, And (myTrueA, myTrueB))),
                                  state ("B"_ST, entry ([] {}), transition ("A"_ST, Or (myTrueA, myTrueB))));

                CHECK_NOTHROW (m.run (0));
                CHECK (m.getCurrentStateIndex () == "A"_ST.getIndex ());

                CHECK_NOTHROW (m.run (2));
                CHECK (m.getCurrentStateIndex () == "B"_ST.getIndex ());

                CHECK_NOTHROW (m.run (3));
                CHECK (m.getCurrentStateIndex () == "A"_ST.getIndex ());

                CHECK (myTrueA.cnt == 0);
                CHECK (myTrueB.cnt == 0);
        }

        SECTION ("By reference")
        {
                auto m = machine (state ("A"_ST, entry ([] {}), transition ("B"_ST, And (std::ref (myTrueA), std::ref (myTrueB)))),
                                  state ("B"_ST, entry ([] {}), transition ("A"_ST, Or (std::ref (myTrueA), std::ref (myTrueB)))));

                CHECK_NOTHROW (m.run (0));
                CHECK (m.getCurrentStateIndex () == "A"_ST.getIndex ());

                CHECK_NOTHROW (m.run (2));
                CHECK (m.getCurrentStateIndex () == "B"_ST.getIndex ());

                CHECK_NOTHROW (m.run (3));
                CHECK (m.getCurrentStateIndex () == "A"_ST.getIndex ());

                /*
                 *  CNT is 2 because we used std::ref so only 1 instance is ever created
                 */
                CHECK (myTrueA.cnt == 2);
                CHECK (myTrueB.cnt == 1);
        }
}

TEST_CASE ("Global transitions", "[Condition]")
{
        auto eq = [] (int what) { return [what] (auto const &i) { return i == what; }; };

        // auto aaa = global (transition ("error"_ST, eq (666)), transition ("fatalError"_ST, eq (777)));

        auto m = machine (global (transition ("error"_ST, eq (666)),       //
                                  transition ("fatalError"_ST, eq (777))), //
                          state ("A"_ST, transition ("B"_ST, eq (1))),     //
                          state ("B"_ST, transition ("A"_ST, eq (2))),     //
                          state ("error"_ST),                              //
                          state ("fatalError"_ST));

        SECTION ("error")
        {

                CHECK_NOTHROW (m.run (0));
                CHECK (m.getCurrentStateIndex () == "A"_ST.getIndex ());

                CHECK_NOTHROW (m.run (1));
                CHECK (m.getCurrentStateIndex () == "B"_ST.getIndex ());

                CHECK_NOTHROW (m.run (666));
                CHECK (m.getCurrentStateIndex () == "error"_ST.getIndex ());
        }

        SECTION ("fatalError")
        {

                CHECK_NOTHROW (m.run (0));
                CHECK (m.getCurrentStateIndex () == "A"_ST.getIndex ());

                CHECK_NOTHROW (m.run (1));
                CHECK (m.getCurrentStateIndex () == "B"_ST.getIndex ());

                CHECK_NOTHROW (m.run (777));
                CHECK (m.getCurrentStateIndex () == "fatalError"_ST.getIndex ());
        }
}

TEST_CASE ("Global transitions with intrumentation", "[Condition]")
{
        auto eq = [] (int what) { return [what] (auto const &i) { return i == what; }; };

        struct Instr {
                void onEntry (const char *currentStateName, unsigned int /* currentStateIndex */) {}
                void onExit (const char *currentStateName, unsigned int /* currentStateIndex */, int acceptedTransNumber) {}
        };

        auto m = machine (Instr{},
                          global (transition ("error"_ST, eq (666)),       //
                                  transition ("fatalError"_ST, eq (777))), //
                          state ("A"_ST, transition ("B"_ST, eq (1))),     //
                          state ("B"_ST, transition ("A"_ST, eq (2))),     //
                          state ("error"_ST),                              //
                          state ("fatalError"_ST));

        CHECK_NOTHROW (m.run (0));
        CHECK (m.getCurrentStateIndex () == "A"_ST.getIndex ());

        CHECK_NOTHROW (m.run (1));
        CHECK (m.getCurrentStateIndex () == "B"_ST.getIndex ());

        CHECK_NOTHROW (m.run (666));
        CHECK (m.getCurrentStateIndex () == "error"_ST.getIndex ());
}
