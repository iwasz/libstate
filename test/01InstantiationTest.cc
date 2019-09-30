#include "StateMachine.h"
#include "catch.hpp"
#include <cstring>
//#include <etl/cstring.h>
#include <iostream>
#include <unistd.h>

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
 * @brief TEST_CASE
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
}
