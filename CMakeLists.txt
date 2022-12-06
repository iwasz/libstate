cmake_minimum_required(VERSION 3.15)

PROJECT (libstate)
SET (CMAKE_VERBOSE_MAKEFILE OFF)

SET(CMAKE_CXX_FLAGS "-std=c++20 -Wall -fdata-sections -ffunction-sections -fno-unwind-tables" CACHE INTERNAL "cxx compiler flags")
#SET(CMAKE_CXX_FLAGS "-fsanitize=address -fno-omit-frame-pointer -std=c++17 -Wall -fdata-sections -ffunction-sections -fno-unwind-tables" CACHE INTERNAL "cxx compiler flags")
# SET(CMAKE_CXX_FLAGS "-fsanitize=memory -fsanitize-memory-track-origins -fPIE -fno-omit-frame-pointer -std=c++17 -Wall -fdata-sections -ffunction-sections -fno-unwind-tables" CACHE INTERNAL "cxx compiler flags")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections" CACHE INTERNAL "exe link flags")
ADD_DEFINITIONS ("-DUNIT_TEST")

find_package(Catch2 2 REQUIRED)
include_directories ("deps/GSL/include" "deps/etl/include") 

INCLUDE_DIRECTORIES("src")
INCLUDE_DIRECTORIES("test")
    
ADD_EXECUTABLE (unittest
    "test/Utils.cc"
    "test/Utils.h"
    "test/00CatchInit.cc"
    "test/01InstantiationTest.cc"
    "test/02ActionTest.cc"    
    "test/03TraitsTest.cc"
    "test/04EventQueueTest.cc"
    "test/05ConditionTest.cc"
)
target_link_libraries("unittest" ncursesw Catch2::Catch2)

# ADD_EXECUTABLE (benchmarktest
#     "test/benchmark.cc"
# )

ADD_EXECUTABLE (tupleTest
    "test/tupleTest.cc"
)

# target_compile_definitions(benchmarktest PRIVATE NDEBUG=1)
