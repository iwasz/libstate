/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#pragma once
#include <gsl/gsl>
#include <iostream>

class At {
public:
        At (const char *c) : cmd (c) {}

        template <typename Arg> void operator() (Arg const & /* a */) { std::cout << "usart <- " << cmd << std::endl; }

private:
        const char *cmd;
};