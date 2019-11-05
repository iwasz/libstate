/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#pragma once
#include <boost/hana.hpp>
#include <gsl/gsl>
#include <optional>
#include <type_traits>
#include <typeindex>

// TODO Remove
#include <functional>
#include <list>

namespace ls {

/// Action return type.
enum class Done { NO, YES };

/**
 * It's a wrapper for an action which , depending on its interface will run it (if the
 * action does not return Done), or will add the action to a list for further processing.
 */
template <typename T> class ActionRunner {
public:
        explicit ActionRunner (T t) : action (std::move (t)) {}

        /**
         * It does not have to use paramater pack. I think actions API will always have
         * only one parameter, but let leave it for now.
         */
        template <typename... Arg> void operator() (Arg &&... a)
        {
                if (!active) {
                        return;
                }

                // using IsInvocable = std::is_invocable<T, Arg...>;
                // TODO this following check does not work, because T() is evaluated. And if action has an argument, the this fails. Should use
                // some smarter one. using DoesReturn = std::is_same<typename std::result_of<T ()>::type, Done>;
                // TODO better checks, static asserts and meaningful error mesages.

                if constexpr (std::is_invocable_v<T, Arg...>) { // Action accepts arguments
                        action (std::forward<Arg> (a)...);
                        static_assert (std::is_same_v<std::invoke_result_t<T, Arg...>, void>, "Wrong action interface.");
                }
                else {                                                                 // Action does not accept arguments
                        if constexpr (std::is_same_v<std::invoke_result_t<T>, Done>) { // But it returns Done.
                                // erasedActionList.push_back (Command (action));
                                // TODO!!! Wait
                        }
                        else {
                                action (); // Doesn't either accept an arg or return.
                        }
                }

                active = false;
        }

        void reset () { active = true; }
        bool isActive () const { return active; }

private:
        T action;
        bool active{true};
};

/**
 * Tuple of actions.
 */
template <typename T> class ActionTuple {
public:
        explicit ActionTuple (T a) : actions{std::move (a)} {}

        // TODO event (parameter pack) ...
        template <typename Ev> void operator() (Ev const &event)
        {
                boost::hana::for_each (actions, [&event] (auto &f) { f (event); });
        }

private:
        T actions;
};

/**
 * This is for creating "default"
 * TODO consider boost::hana::optional
 */
template <> class ActionTuple<void> {
public:
        ActionTuple () = default;
        template <typename Ev> void operator() (Ev const & /*event*/) {}
};

template <typename T> struct Entry : public ActionTuple<T> {
        using ActionTuple<T>::ActionTuple;
};

template <typename T> struct Exit : public ActionTuple<T> {
        using ActionTuple<T>::ActionTuple;
};

template <template <typename T> class Tu, typename... Ar> auto actionTuple (Ar &&... args)
{
        auto tuple = boost::hana::make_tuple (ActionRunner (std::forward<Ar> (args))...);
        auto at = Tu<decltype (tuple)> (std::move (tuple));
        return at;
}

template <typename... Ar> auto entry (Ar &&... args) { return actionTuple<Entry, Ar...> (std::forward<Ar> (args)...); }
template <typename... Ar> auto exit (Ar &&... args) { return actionTuple<Exit, Ar...> (std::forward<Ar> (args)...); }

} // namespace ls