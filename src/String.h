/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#pragma once

namespace ls {

template <char... s> struct string {
        static constexpr char const *c_str () { return "TODO"; }
};

template <char... s> constexpr string<s...> string_c{};

} // namespace ls