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
        State noAction;

        State state1 (entry (
                [](auto &&ev) {
                        std::cout << "Inline lambda action (" << ev << ")" << std::endl;
                        return Done::YES;
                },
                Class ("Class instance action"), actionFunction, actionFunctionTemplate<std::string const &>));

        State state2 (entry (
                              [](auto &&ev) {
                                      std::cout << "Inline lambda action (" << ev << ")" << std::endl;
                                      return Done::YES;
                              },
                              Class ("Class instance action"), actionFunction, actionFunctionTemplate<std::string const &>),
                      exit (
                              [](auto &&ev) {
                                      std::cout << "Inline lambda action (" << ev << ")" << std::endl;
                                      return Done::YES;
                              },
                              Class ("Class instance action"), actionFunction, actionFunctionTemplate<std::string const &>));

        auto state3 = State{entry (At{"ATZ"}, At{"ATDT"})};

        state2.entry ("hello"s);
        state2.exit ("hello"s);
}

/**
 * Machine instantiated for the forst time.
 */
TEST_CASE ("Machine instance", "[Instantiation]")
{
        int a, b;
        auto m = machine (State (entry ([] {
                                         std::cout << "1st entry" << std::endl;
                                         return Done::YES;
                                 }),
                                 exit ([] {
                                         std::cout << "1st exit" << std::endl;
                                         return Done::YES;
                                 }),
                                 transitions (Transition (StateName::A, Condition ()))),

                          State (entry (At ("Z")), exit (At ("DT"))),
                          transitions (Transition (StateName::B, Eq ("OK"), action (At ("A"), At ("B")))),

                          state (StateName::A, entry (At ("Z")), exit (At ("DT")), transition (StateName::C, Eq ("OK"), At ("A"), At ("B"))));
}
