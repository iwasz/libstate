/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#pragma once
#include "Action.h"
#include "Misc.h"
#include <boost/hana.hpp>
#include <boost/hana/fwd/unpack.hpp>
#include <chrono>
#include <gsl/gsl>
#include <optional>
#include <pthread.h>
#include <type_traits>
#include <typeindex>

// TODO remove
#include <string>
// TODO remove
#include <iostream>

namespace ls {

// TODO This is a GNU extension. Provide macro as an option. Or better still, use hana::string explicitly
template <typename C, C... c> constexpr auto operator""_STATE () { return boost::hana::string_c<c...>; }

/**
 *
 */
template <typename Sn = int, typename C = int, typename T = int> class Transition {
public:
        explicit Transition (Sn sn) : stateName (std::move (sn)) {}
        Transition (Sn sn, C c) : stateName (std::move (sn)), condition (std::move (c)) {}
        Transition (Sn sn, C c, T t) : stateName (std::move (sn)), condition (std::move (c)), actions (std::move (t)) {}

        // TODO accessors, getters
        // private:
        Sn stateName;
        C condition;
        T actions;
};

template <typename Sn, typename Cond, typename... Acts> auto transition (Sn &&sn, Cond &&cond, Acts &&... acts)
{
        return Transition (std::forward<decltype (sn)> (sn), std::forward<decltype (cond)> (cond),
                           boost::hana::tuple (std::forward<decltype (acts)> (acts)...));
};

// template <typename... Ts> auto transitions (Ts &&... ts) { return hana::make_tuple (std::forward<Ts> (ts)...); }

template <typename Sn, typename T1 = void, typename T2 = void, typename T3 = int> class State {
public:
        State () = delete;
        explicit State (Sn sn) : name (std::move (sn)) {}
        State (Sn sn, Entry<T1> en) : name (std::move (sn)), entry (std::move (en)) {}
        State (Sn sn, Entry<T1> en, Exit<T2> ex) : name (std::move (sn)), entry (std::move (en)), exit (std::move (ex)) {}
        State (Sn sn, Entry<T1> en, Exit<T2> ex, T3 ts)
            : name (std::move (sn)), entry (std::move (en)), exit (std::move (ex)), transitions (std::move (ts))
        {
        }

        // private:
        Sn name;
        Entry<T1> entry;
        Exit<T2> exit;
        T3 transitions;
};

template <typename Sn, typename Entry, typename Exit, typename... Trans> auto state (Sn &&sn, Entry &&entry, Exit &&exit, Trans &&... trans)
{
        return State (std::forward<decltype (sn)> (sn), std::forward<decltype (entry)> (entry), std::forward<decltype (exit)> (exit),
                      boost::hana::tuple (std::forward<decltype (trans)> (trans)...));
}

// TODO not here
class At {
public:
        At (gsl::czstring<> c) : cmd (c) {}
        void operator() () { std::cout << "usart <- " << cmd << std::endl; }

private:
        gsl::czstring<> cmd;
};

/// Event : string with operator ==
// TODO move from here
struct Eq {
        Eq (std::string t) : t (std::move (t)) {}

        template <typename Ev> bool operator() (Ev const &ev) const { return ev == t; }
        std::string t;
};

/**
 *
 */
template <typename S> class Machine {
public:
        explicit Machine (S s) : states{std::move (s)}
        {
                // Look for initial state if current is empty (std::optional is used).
                auto initialState = findInitialState ();

                // currentName = typeid (boost::hana::first (initialStatePair));
                // auto initialState = boost::hana::second (initialStatePair);
                currentName = std::type_index (typeid (initialState->name));

                // auto initialState = boost::hana::second (initialStatePair);
                // initialState.runEntryActions (); // There is no event that caused this action to be fired. We are just starting.

                std::cout << "Initial : " << initialState->name.c_str () << std::endl;
        }

        template <typename Q> void run (Q &&queue);
        template <typename Q> void waitAndRun (Q &&queue)
        {
                while (isWaiting ()) {
                }
                run (std::forward<Q> (queue));
        }

        auto getCurrentStateName () const { return currentName; }
        bool isWaiting () const { return !timer.isExpired (); }

private:
        auto findInitialState () const;
        Done runLongActions ();

private:
        S states;                                     /// boost::hana::tuple of States. TODO hana map
        std::optional<std::type_index> currentName{}; /// Current state name. // TODO use type_index
        Timer timer{};
};

/*--------------------------------------------------------------------------*/

/// Helper for creating a machine.
template <typename... Sts> auto machine (Sts &&... states) { return Machine (boost::hana::make_tuple (std::forward<Sts> (states)...)); }

/****************************************************************************/

template <typename S> auto Machine<S>::findInitialState () const
{
        auto initialState
                = boost::hana::find_if (states, [] (auto const &state) { return state.name == boost::hana::string_c<'I', 'N', 'I', 'T'>; });
        static_assert (initialState != boost::hana::nothing);
        return initialState;
}

/****************************************************************************/

struct StateProcessResult {
        std::optional<std::type_index> newStateName{};
        Delay delay{};
};

template <typename Q, typename S, typename T, typename... Tr>
StateProcessResult processTransitions (Q &&eventQueue, S &state, T &transition, Tr &... rest)
{
        for (auto event : eventQueue) {
                static_assert (std::is_invocable<decltype (transition.condition), decltype (event)>::value,
                               "Type mismatch between an event, and a condition(s) which checks this event.");

                // Perform the transition
                if (transition.condition (event)) {
                        std::cout << "Transition to : " << transition.stateName.c_str () << std::endl;

                        // Run curent.exit
                        Delay d = state.exit (event);

                        if (d != Delay::zero ()) {
                                std::cerr << "Delay requested : " << std::chrono::duration_cast<std::chrono::milliseconds> (d).count () << "ms"
                                          << std::endl;
                                return {{}, d};
                        }

                        // TODO Action tuple, action runner.
                        // Run transition.action
                        boost::hana::for_each (transition.actions, [&event] (auto &action) {
                                if constexpr (std::is_invocable_v<decltype (action), decltype (event)>) {
                                        action (event);
                                }
                                else {
                                        action ();
                                }
                        });

                        // Change current name.
                        auto ret = std::optional<std::type_index> (typeid (transition.stateName));

                        // - run current.entry

                        eventQueue.clear ();
                        return {ret, {}};
                }
        }

        if constexpr (sizeof...(rest)) {
                return processTransitions (eventQueue, state, rest...);
        }

        return {};
}

/****************************************************************************/

template <typename S, typename Q, typename... Rs>
StateProcessResult processStates (std::type_index const &currentStateTi, Q &&eventQueue, S &state, Rs &... rest)
{
        if (std::type_index (typeid (state.name)) == currentStateTi) {
                std::cout << "Current  : " << state.name.c_str () << std::endl;

                return boost::hana::unpack (state.transitions, [&eventQueue, &state] (auto &... trans) -> StateProcessResult {
                        if constexpr (sizeof...(trans)) {
                                return processTransitions (eventQueue, state, trans...);
                        }

                        return {};
                });
        }

        if constexpr (sizeof...(rest) > 0) {
                return processStates (currentStateTi, std::forward<Q> (eventQueue), rest...);
        }

        return {};
}

/****************************************************************************/

template <typename S> template <typename Q> void Machine<S>::run (Q &&eventQueue)
{
        if (!timer.isExpired ()) {
                return;
        }

        std::cout << "== Run ==" << std::endl;
        Expects (currentName);
        std::type_index &currentStateNameCopy = *currentName;

        StateProcessResult result = boost::hana::unpack (states, [&currentStateNameCopy, &eventQueue] (auto &... arg) -> StateProcessResult {
                if constexpr (sizeof...(arg)) {
                        return processStates (currentStateNameCopy, eventQueue, arg...);
                }

                return {};
        });

        if (result.newStateName) {
                currentName = result.newStateName;
        }
        else if (result.delay != Delay::zero ()) {
                // TODO modify timer, and get rid of the cast.
                timer.start (std::chrono::duration_cast<std::chrono::milliseconds> (result.delay).count ());
        }
}

} // namespace ls
