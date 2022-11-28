/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "Machine.h"
#include <boost/callable_traits/has_void_return.hpp>
#include <boost/callable_traits/return_type.hpp>
#include <catch2/catch.hpp>
#include <type_traits>
#include <utility>

using namespace ls;

// template <typename T> struct has_1_argument : public std::false_type {
// };

// template <typename R, typename A, typename Cls> struct has_1_argument<R (Cls::*) (A)> : public std::true_type {
// };

struct S {

        int operator() (int) { return 0; }
};

template <typename T> inline constexpr bool returns_bool_v = std::is_same_v<boost::callable_traits::return_type_t<T>, bool>;

template <typename T, typename... A, typename = std::enable_if_t<returns_bool_v<T> && (boost::callable_traits::has_void_return_v<A> && ...)>>
void transition (T &&t, A &&...a)
{
}

template <typename... A, typename = std::enable_if_t<(boost::callable_traits::has_void_return_v<A> && ...)>> void transition (A &&...a) {}

// struct S {
//         double operator() (char, int &);
//         float operator() (int) { return 1.0; }
// };

/**
 * This test only instantiates some bits of state machine and checks if it is even possible.
 * Does not do any REQUIRE checks.
 */
TEST_CASE ("Traits", "[Traits]")
{

        // transition ([] {});
        transition ([] () -> bool { return false; });
        transition ([] () -> bool { return false; }, [] {});
        transition ([] () -> bool { return false; }, [] {}, [] {});
        transition ([] () -> bool { return false; }, [] {}, [] {}, [] {});

        transition ([] {}, [] {}, [] {});

        // transition ([] {}, [] () -> bool { return false; });
        // transition ([] () -> bool { return false; }, [] {});

        S s;
        static_assert (std::is_same_v<boost::callable_traits::return_type_t<decltype (s)>, int>);

        auto x = [] () -> int { return 0; };
        static_assert (std::is_same_v<boost::callable_traits::return_type_t<decltype (x)>, int>);

        // auto la = [] (int) {};
        // static_assert (has_1_argument<decltype (la)>::value);
        // static_assert (has_1_argument<S>::value);
}
