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

/**
 *
 */
TEST_CASE ("Action basic test", "[Action]")
{
        auto action = [] {
                std::cout << "Action : " << std::endl;
                return Done::YES;
        };

        ErasedAction ea{action};
        ea ();
}