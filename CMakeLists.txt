PROJECT (libstate)
SET (CMAKE_VERBOSE_MAKEFILE OFF)

#if(COMMAND cmake_policy)
#    cmake_policy(SET CMP0003 NEW)
#endif(COMMAND cmake_policy)

SET(CMAKE_CXX_FLAGS "-std=c++17 -Wall" CACHE INTERNAL "cxx compiler flags")
ADD_DEFINITIONS ("-DUNIT_TEST")

SET (CATCH2_INCLUDE "deps/Catch2/single_include/catch2")
INCLUDE_DIRECTORIES("${CATCH2_INCLUDE}")
include_directories (deps/GSL/include)

#SET (ETL_INCLUDE "../etl/include")
#INCLUDE_DIRECTORIES("${ETL_INCLUDE}")

INCLUDE_DIRECTORIES("src")
INCLUDE_DIRECTORIES("test")
    
ADD_EXECUTABLE (unittest
    "${CATCH2_INCLUDE}/catch.hpp"

    "src/StateMachine.h"

    #"test/etl_profile.h"
    "test/00CatchInit.cc"
    "test/01InstantiationTest.cc"
    "test/02DoneNotDoneActionTest.cc"
)

