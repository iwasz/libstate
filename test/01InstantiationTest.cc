/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "StateMachine.h"
#include "catch.hpp"
#include <cstring>
//#include <etl/cstring.h>
#include <deque>
#include <iostream>
#include <unistd.h>

namespace hana = boost::hana;
using namespace hana::literals;
using namespace std::string_literals;
using namespace ls;

// Nigdzie nie używam czegoś takiego, ale miło by było móc.
template <typename T> Done actionFunctionTemplate (T &&event)
{
        std::cout << "actionFunctionTemplate (" << event << ")" << std::endl;
        return Done::YES;
}

Done actionFunction (std::string const &event)
{
        std::cout << "actionFunction (" << event << ")" << std::endl;
        return Done::YES;
}

struct Class {
        Class (std::string v) : value (std::move (v)) {}

        template <typename T> Done operator() (T &&ev)
        {
                std::cout << value << " (" << ev << ")" << std::endl;
                return Done::YES;
        }

private:
        std::string value;
};

/**
 * This test only instantiates some bits of state machine and checks if it is even possible.
 */
TEST_CASE ("First test", "[Instantiation]")
{
        State noAction ("A"_STATE);

        State state1 ("A"_STATE,
                      entry (
                              [] (auto &&ev) { // First entry action is a lambda
                                      std::cout << "Inline lambda action (" << ev << ")" << std::endl;
                                      return Done::YES;
                              },
                              Class ("Class instance action"),               // Second is a function object.
                              actionFunction,                                // Third is a regular function.
                              actionFunctionTemplate<std::string const &>)); // Fourth is a function template.

        State state2 ("A"_STATE,
                      entry (
                              [] (auto &&ev) {
                                      std::cout << "Inline lambda action (" << ev << ")" << std::endl;
                                      return Done::YES;
                              },
                              Class ("Class instance action"), actionFunction, actionFunctionTemplate<std::string const &>),
                      exit (
                              [] (auto &&ev) {
                                      std::cout << "Inline lambda action (" << ev << ")" << std::endl;
                                      return Done::YES;
                              },
                              Class ("Class instance action"), actionFunction, actionFunctionTemplate<std::string const &>));

        auto state3 = State ("A"_STATE, entry (At{"ATZ"}, At{"ATDT"}));

        state2.entry ("hello"s);
        state2.exit ("hello"s);

        state3.entry ("hello"s);
        state3.exit ("hello"s);
}

/**
 * Machine instantiated for the first time.
 */
TEST_CASE ("Machine instance", "[Instantiation]")
{
        using namespace hana::literals;

        auto m = machine (state ("INIT"_STATE,
                                 entry (
                                         // Action variant 1 : event passed as an argument, and return value with type Done.
                                         [] (int event) {
                                                 std::cout << "1st entry [" << event << "]" << std::endl;
                                                 return Done::YES;
                                         },
                                         // Action variant 2 : event passed as an argument, but no return value (Done::YES is assumed).
                                         [] (int event) { std::cout << "1st entry [" << event << "]" << std::endl; },
                                         // Action variant 3 : no input argument and return value.
                                         [] () {
                                                 std::cout << "1st entry" << std::endl;
                                                 return Done::YES;
                                         },
                                         // Action variant 4 (the simplest).
                                         [] () { std::cout << "1st entry" << std::endl; }),
                                 exit ([] {
                                         std::cout << "1st exit" << std::endl;
                                         return Done::YES;
                                 }),
                                 transition ("B"_STATE, [] (int i) { return i == 2; })),

                          state ("B"_STATE, entry (At ("Z")), exit (At ("DT")),
                                 transition (
                                         "C"_STATE, [] (int i) { return i == 3; }, At ("A"), At ("B"))),

                          state ("C"_STATE, entry (At ("Z")), exit (At ("DT"))));

        m.run (std::deque{1, 2});
        m.run (std::deque{3});
        // m.run (std::deque{4});
        // m.run (std::deque{5});
}
