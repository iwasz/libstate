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

/**
 *
 */
TEST_CASE ("No Default self-transiution", "[Transition]") {}
