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
 * Another feature which helps to minimise verbosity is that one can define actions
 * which do not take an event argument. In this unit test we test both Done::YES and
 * Done::NO cases.
 *
 * Another thing you'll see in this use-case is that events passed to the state.entry,
 * and state.exit invocations can be of arbitrary type. Here we are using integers as
 * events.
 */
TEST_CASE ("NoArgsReturnsDone", "[EventNoEvent]")
{
        {
                // Done::YES case, where action doesn not want to be run again.
                int counter = 0;

                State state (entry ([&counter]() {
                        ++counter;
                        return Done::YES;
                }));

                state.entry (1);
                REQUIRE (counter == 1);

                state.entry (2);
                REQUIRE (counter == 1);
        }

        {
                // Done::NO case, where action didn't do everything it wanted, so it can be run again later on.
                int counter = 0;

                State state (entry ([&counter]() {
                        ++counter;
                        return Done::NO;
                }));

                state.entry (0);
                REQUIRE (counter == 1);

                state.entry (0);
                REQUIRE (counter == 2);

                state.entry (0);
                REQUIRE (counter == 3);
        }
}

/**
 * And finally the shortest case of an action, which does not care about the event that
 * casused it to run, and does not return anything which means the same as returning Done::YES.
 */
TEST_CASE ("NoArgsNoReturn", "[EventNoEvent]")
{
        int counter = 0;

        State state (entry ([&counter]() { ++counter; }));

        state.entry (1);
        REQUIRE (counter == 1);

        state.entry (2);
        REQUIRE (counter == 1);
}
