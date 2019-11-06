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
#include "gsl/gsl_assert"
#include <boost/hana.hpp>
#include <boost/hana/fwd/integral_constant.hpp>
#include <boost/hana/fwd/length.hpp>
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
                           transitionAction (std::forward<decltype (acts)> (acts)...));
};

// template <typename... Ts> auto transitions (Ts &&... ts) { return hana::make_tuple (std::forward<Ts> (ts)...); }

template <typename Sn, typename T1 = void, typename T2 = void, typename T3 = boost::hana::tuple<>> class State {
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

template <typename Sn, typename Entry> auto state (Sn &&sn, Entry &&entry)
{
        return State (std::forward<decltype (sn)> (sn), std::forward<decltype (entry)> (entry));
}

// TODO if processX were moved into the class, move this as well
struct StateProcessResult {
        std::optional<std::type_index> newStateName{};
        Delay delay{};
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

                if (Delay d = initialState->entry (1); d != Delay::zero ()) {
                        std::cerr << "Delay requested : " << std::chrono::duration_cast<std::chrono::milliseconds> (d).count () << "ms"
                                  << std::endl;
                        timer.start (std::chrono::duration_cast<std::chrono::milliseconds> (d).count ());
                }

                // currentName = typeid (boost::hana::first (initialStatePair));
                // auto initialState = boost::hana::second (initialStatePair);
                currentName = std::type_index (typeid (initialState->name));

                // auto initialState = boost::hana::second (initialStatePair);
                // initialState.runEntryActions (); // There is no event that caused this action to be fired. We are just starting.

#ifndef NDEBUG
                std::cout << "Initial : " << initialState->name.c_str () << std::endl;
#endif
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

        template <typename St, typename Q, typename... Rs>
        StateProcessResult processStates (std::type_index const &currentStateTi, Q &&eventQueue, St &state, Rs &... rest);

        template <typename Q, typename St, typename T, typename... Tr>
        StateProcessResult processTransitions (Q &&eventQueue, St &state, T &transition, Tr &... rest);

        template <typename Ev> Delay runResetEntryActions (std::type_index const &stateTi, Ev const &ev);

private:
        S states;                                     /// boost::hana::tuple of States. TODO hana map
        std::optional<std::type_index> currentName{}; /// Current state name. // TODO use type_index
        Timer timer{};
};

/*--------------------------------------------------------------------------*/

/// Helper for creating a machine.
template <typename... Sts> auto machine (Sts &&... states)
{
        // TODO this first artificial state -> unique name, no actions.
        return Machine (
                boost::hana::make_tuple (state ("_"_STATE, entry ([] {}), exit ([] {}), transition ("INIT"_STATE, [] (auto) { return true; })),
                                         std::forward<Sts> (states)...));
}

/****************************************************************************/

template <typename S> auto Machine<S>::findInitialState () const
{
        auto initialState
                = boost::hana::find_if (states, [] (auto const &state) { return state.name == boost::hana::string_c<'I', 'N', 'I', 'T'>; });
        static_assert (initialState != boost::hana::nothing, "Initial state has to be named \"INIT\"_STATE and it must be defined.");

        auto boostrapState = boost::hana::find_if (states, [] (auto const &state) { return state.name == boost::hana::string_c<'_'>; });
        return boostrapState;
}

/****************************************************************************/

template <typename Ev, typename St, typename... Rs>
Delay processRunResetEntryActions (std::type_index const &stateTi, Ev const &ev, St &state, Rs &... rest)
{
        if (std::type_index (typeid (state.name)) == stateTi) {
                if (Delay d = state.entry (ev); d != Delay::zero ()) {
                        std::cerr << "Delay requested : " << std::chrono::duration_cast<std::chrono::milliseconds> (d).count () << "ms"
                                  << std::endl;
                        return d;
                }

                // All done, reset.
                state.entry.reset ();
                return Delay::zero ();
        }

        if constexpr (sizeof...(rest) > 0) {
                return processRunResetEntryActions (stateTi, ev, rest...);
        }

        return Delay::zero ();
}

/****************************************************************************/

template <typename S> template <typename Ev> Delay Machine<S>::runResetEntryActions (std::type_index const &stateTi, Ev const &ev)
{
        return boost::hana::unpack (states, [&stateTi, &ev] (auto &... states) { return processRunResetEntryActions (stateTi, ev, states...); });
}

/****************************************************************************/

template <typename Ev, typename Cn> bool checkCondition (Ev const &ev, Cn &cn)
{
        if constexpr (std::is_invocable_v<Cn, Ev>) {
                return cn (ev);
        }
        else {
                static_assert (std::is_invocable_v<Cn>,
                               "Incompatibility between an event, and a condition(s) which checks this event. Conditions may have a single "
                               "event argument or no arguments at all.");

                return cn ();
        }
}

template <typename S>
template <typename Q, typename St, typename T, typename... Tr>
StateProcessResult Machine<S>::processTransitions (Q &&eventQueue, St &state, T &transition, Tr &... rest)
{
        for (auto const &event : eventQueue) {

                // Perform the transition
                // TODO accept conditions without an argument
                // if (checkCondition (transition.condition, event)) {
                if (transition.condition (event)) {
#ifndef NDEBUG
                        std::cout << "Transition to : " << transition.stateName.c_str () << std::endl;
#endif
                        // Run curent.exit
                        if (Delay d = state.exit (event); d != Delay::zero ()) {
                                std::cerr << "Delay requested : " << std::chrono::duration_cast<std::chrono::milliseconds> (d).count () << "ms"
                                          << std::endl;
                                return {{}, d};
                        }

                        if (Delay d = transition.actions (event); d != Delay::zero ()) {
                                std::cerr << "Delay requested : " << std::chrono::duration_cast<std::chrono::milliseconds> (d).count () << "ms"
                                          << std::endl;
                                return {{}, d};
                        }

                        // Change current name.
                        auto ret = std::optional<std::type_index> (typeid (transition.stateName));
                        Expects (ret);

                        // - run current.entry
                        runResetEntryActions (*ret, event);

                        eventQueue.clear (); // TODO not quite like that
                        state.exit.reset ();
                        transition.actions.reset ();
                        return {ret, {}};
                }
        }

        if constexpr (sizeof...(rest)) {
                return processTransitions (eventQueue, state, rest...);
        }

        return {};
}

/****************************************************************************/

template <typename S>
template <typename St, typename Q, typename... Rs>
StateProcessResult Machine<S>::processStates (std::type_index const &currentStateTi, Q &&eventQueue, St &state, Rs &... rest)
{
        if (std::type_index (typeid (state.name)) == currentStateTi) {
#ifndef NDEBUG
                std::cout << "Current  : " << state.name.c_str () << std::endl;
#endif
                return boost::hana::unpack (state.transitions, [this, &eventQueue, &state] (auto &... trans) -> StateProcessResult {
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

#ifndef NDEBUG
        std::cout << "== Run ==" << std::endl;
#endif
        Expects (currentName);
        std::type_index &currentStateNameCopy = *currentName;

        StateProcessResult result
                = boost::hana::unpack (states, [this, &currentStateNameCopy, &eventQueue] (auto &... arg) -> StateProcessResult {
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
