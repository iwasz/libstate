cmake_minimum_required(VERSION 3.15)

PROJECT (libstate)
SET (CMAKE_VERBOSE_MAKEFILE OFF)

SET(CMAKE_CXX_FLAGS "-std=c++17 -Wall -fdata-sections -ffunction-sections -fno-unwind-tables" CACHE INTERNAL "cxx compiler flags")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections" CACHE INTERNAL "exe link flags")
ADD_DEFINITIONS ("-DUNIT_TEST")

SET (CATCH2_INCLUDE "deps/Catch2/single_include/catch2")
INCLUDE_DIRECTORIES("${CATCH2_INCLUDE}")
include_directories ("deps/GSL/include" "deps/etl/include") 

INCLUDE_DIRECTORIES("src")
INCLUDE_DIRECTORIES("test")
    
ADD_EXECUTABLE (unittest
    "${CATCH2_INCLUDE}/catch.hpp"

    "src/StateMachine.h"
    "src/Action.h"
    "src/Misc.h"
    "src/State.h"
    "src/Transition.h"

    "test/Utils.cc"
    "test/Utils.h"
    "test/00CatchInit.cc"
    "test/01InstantiationTest.cc"
    "test/02ActionTest.cc"
    "test/03TraitsTest.cc"
)

ADD_EXECUTABLE (benchmarktest
    "src/StateMachine.h"
    "src/Action.h"
    "src/Misc.h"
    "test/benchmark.cc"
)

ADD_EXECUTABLE (tupleTest
    "test/tupleTest.cc"
)

target_compile_definitions(benchmarktest PRIVATE NDEBUG=1)
