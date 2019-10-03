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
        //        State noAction ("A"_STATE);

        //        State state1 ("A"_STATE,
        //                      entry (
        //                              [](auto &&ev) {
        //                                      std::cout << "Inline lambda action (" << ev << ")" << std::endl;
        //                                      return Done::YES;
        //                              },
        //                              Class ("Class instance action"), actionFunction, actionFunctionTemplate<std::string const &>));

        //        State state2 ("A"_STATE,
        //                      entry (
        //                              [](auto &&ev) {
        //                                      std::cout << "Inline lambda action (" << ev << ")" << std::endl;
        //                                      return Done::YES;
        //                              },
        //                              Class ("Class instance action"), actionFunction, actionFunctionTemplate<std::string const &>),
        //                      exit (
        //                              [](auto &&ev) {
        //                                      std::cout << "Inline lambda action (" << ev << ")" << std::endl;
        //                                      return Done::YES;
        //                              },
        //                              Class ("Class instance action"), actionFunction, actionFunctionTemplate<std::string const &>));

        //        auto state3 = State ("A"_STATE, entry (At{"ATZ"}, At{"ATDT"}));

        //        state2.entry ("hello"s);
        //        state2.exit ("hello"s);
}

/**
 * Machine instantiated for the forst time.
 */
TEST_CASE ("Machine instance", "[Instantiation]")
{
        auto m = machine (
                state (hana::int_c<0>, entry ([] {
                               std::cout << "1st entry" << std::endl;
                               return Done::YES;
                       }),
                       exit ([] {
                               std::cout << "1st exit" << std::endl;
                               return Done::YES;
                       }),
                       transition (hana::int_c<1>, [] { return true; })),

                state (hana::int_c<1>, entry (At ("Z")), exit (At ("DT")), transition (hana::int_c<2>, Eq ("OK"), At ("A"), At ("B"))),
                state (hana::int_c<2>, entry (At ("Z")), exit (At ("DT")), transition (hana::int_c<0>, Eq ("OK"), At ("A"), At ("B"))));

        m.run (std::deque{1, 2, 3});
}
