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

/**
 * Checks if value returned from actions is honored or not.
 * If an action returns Done::YES (i.e. it reports that it is finished,
 * and don't have to be revoked later on), then further calls to this
 * action should result in empty action.
 */
TEST_CASE ("Done", "[DoneNotDoneAction]")
{
        int counter = 0;

        State state (entry ([&counter](auto &) {
                ++counter;
                return Done::YES;
        }));

        state.entry (""s);
        REQUIRE (counter == 1);

        state.entry (""s);
        REQUIRE (counter == 1);

        state.entry (""s);
        REQUIRE (counter == 1);

        state.entry (""s);
        REQUIRE (counter == 1);
}

/**
 * In other words an action can be run many times until it returns Done::YES.
 * After returning Done::YES, calls to the action take no effect. This way we can
 * implement a non blocking delay action which in turn makes async state machine
 * implementation possible.
 */
TEST_CASE ("NotDone", "[DoneNotDoneAction]")
{
        int counter = 0;

        State state (entry ([&counter](auto &) {
                ++counter;
                return Done::NO;
        }));

        state.entry (""s);
        REQUIRE (counter == 1);

        state.entry (""s);
        REQUIRE (counter == 2);

        state.entry (""s);
        REQUIRE (counter == 3);

        state.entry (""s);
        REQUIRE (counter == 4);
}
