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

Done fun1 () { return Done::YES; }
Done fun2 (int /* event */) { return Done::YES; }
void fun3 () {}

class Command2 {
public:
        using ActionInterface = Done ();

        template <typename A> Command2 (A a) : action{std::move (a)} {}
        // Command (void *a) : action{a} {}

        Done operator() () { return action (); }

        std::function<ActionInterface> action;
};

/**
 *
 */
TEST_CASE ("No arg", "[Command]")
{
        {
                auto action = [] {
                        std::cout << "Action : " << std::endl;
                        return Done::YES;
                };

                Command2 c{action};
                c ();
        }

        {
                Command2 c1{fun1};
                REQUIRE (c1 () == Done::YES);

                // Command c2{fun2};
                // REQUIRE (c2 (1) == Done::YES);
        }
}