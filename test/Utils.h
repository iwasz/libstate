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
        At (gsl::czstring<> c) : cmd (c) {}
        void operator() (int) { std::cout << "usart <- " << cmd << std::endl; }

private:
        gsl::czstring<> cmd;
};