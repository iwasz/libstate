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
#include <boost/hana/fwd/tuple.hpp>
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

/****************************************************************************/

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

/****************************************************************************/

/**
 * A state - typesafe version.
 */
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

        constexpr const char *getName () const { return name.c_str (); }

        template <typename Ev> Delay runEntryActions (Ev const &ev) { return entry (ev); }

        template <typename Ev> Delay runExitActions (Ev const &ev) { return exit (ev); }

        size_t getTransitionSizeOf (size_t index) const;

        // private:
        Sn name;
        Entry<T1> entry;
        Exit<T2> exit;
        T3 transitions;
        // constexpr size_t transitionsNumber;
};

template <typename T, typename... Rs>
constexpr size_t processGetTransitionSizeOf (size_t index, size_t current, T const &transition, Rs const &... rest)
{
        if (index == current) {
                return sizeof (transition);
        }

        if constexpr (sizeof...(rest)) {
                return processGetTransitionSizeOf (index, current + 1, rest...);
        }

        return 0;
}

template <typename Sn, typename T1, typename T2, typename T3> size_t State<Sn, T1, T2, T3>::getTransitionSizeOf (size_t index) const
{
        // TODO does not work, why can't I get the size from T3 type directly?
        // if constexpr (boost::hana::length (transitions) != boost::hana::size_c<0>) {
        return boost::hana::unpack (transitions, [index] (auto const &... trans) -> size_t {
                if constexpr (sizeof...(trans) > 0) {
                        return processGetTransitionSizeOf (index, 0, trans...);
                }

                return 0;
        });

        std::cout << std::endl;
        // }

        return 0;
}

template <typename Sn, typename Entry, typename Exit, typename... Trans> auto state (Sn &&sn, Entry &&entry, Exit &&exit, Trans &&... trans)
{
        return State (std::forward<decltype (sn)> (sn), std::forward<decltype (entry)> (entry), std::forward<decltype (exit)> (exit),
                      boost::hana::tuple (std::forward<decltype (trans)> (trans)...));
}

template <typename Sn, typename Entry> auto state (Sn &&sn, Entry &&entry)
{
        return State (std::forward<decltype (sn)> (sn), std::forward<decltype (entry)> (entry));
}

template <typename Ev> class ErasedStateBase {
public:
        ErasedStateBase () = default;
        virtual ~ErasedStateBase () = default;
        ErasedStateBase (ErasedStateBase const &e) = default;
        ErasedStateBase &operator= (ErasedStateBase const &e) = default;
        ErasedStateBase (ErasedStateBase &&e) noexcept = default;
        ErasedStateBase &operator= (ErasedStateBase &&e) noexcept = default;

        virtual Delay runEntryActions (Ev const &ev) = 0;
        virtual Delay runExitActions (Ev const &ev) = 0;

        // Transitions - but how?
        virtual size_t getTransitionSizeOf (size_t index) const = 0;
};

template <typename Ev, typename S> class ErasedState : public ErasedStateBase<Ev> {
public:
        explicit ErasedState (S s) : internal (std::move (s)) {}
        Delay runEntryActions (Ev const &ev) override { return internal.runEntryActions (ev); }
        Delay runExitActions (Ev const &ev) override { return internal.runExitActions (ev); }
        size_t getTransitionSizeOf (size_t index) const override { return internal.getTransitionSizeOf (index); }

        S internal;
};

template <typename Ev, typename S> ErasedState<Ev, S> erasedState (S s) { return ErasedState<Ev, S> (std::move (s)); }

/****************************************************************************/

// TODO if processX were moved into the class, move this as well
struct StateProcessResult {
        std::optional<std::type_index> newStateName{};
        Delay delay{};
};

/**
 *
 */
template <typename Ev, typename S> class Machine {
public:
        explicit Machine (S s) : states{std::move (s)}
        {
                // Look for initial state if current is empty (std::optional is used).
                auto initialState = findInitialState ();
                currentName = std::type_index (typeid (initialState.internal.name));

#ifndef NDEBUG
                std::cout << "Initial : " << initialState.internal.name.c_str () << std::endl;
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

        Delay runResetEntryActions (std::type_index const &stateTi, Ev const &ev);

        ErasedStateBase<Ev> *findState (std::type_index const &stateIndex) {}

private:
        S states;                                     /// boost::hana::tuple of States. TODO hana map
        std::optional<std::type_index> currentName{}; /// Current state name. // TODO use type_index
        ErasedStateBase<Ev> *currentState{};
        Timer timer{};
};

/*--------------------------------------------------------------------------*/

/// Helper for creating a machine.
template <typename Ev, typename... Sts> auto machine (Sts &&... states)
{
        auto s = boost::hana::make_tuple (
                erasedState<Ev> (state ("_"_STATE, entry ([] {}), exit ([] {}), transition ("INIT"_STATE, [] (auto) { return true; }))),
                erasedState<Ev> (std::forward<Sts> (states))...);

        return Machine<Ev, decltype (s)> (std::move (s));
}

/****************************************************************************/

template <typename Ev, typename S> auto Machine<Ev, S>::findInitialState () const
{
        auto initialState = boost::hana::find_if (
                states, [] (auto const &state) { return state.internal.name == boost::hana::string_c<'I', 'N', 'I', 'T'>; });

        static_assert (initialState != boost::hana::nothing, "Initial state has to be named \"INIT\"_STATE and it must be defined.");

        return *boost::hana::find_if (states, [] (auto const &state) { return state.internal.name == boost::hana::string_c<'_'>; });
}

/****************************************************************************/

template <typename Ev, typename St, typename... Rs>
Delay processRunResetEntryActions (std::type_index const &stateTi, Ev const &ev, St &state, Rs &... rest)
{
        if (std::type_index (typeid (state.internal.name)) == stateTi) {
                if (Delay d = state.internal.entry (ev); d != Delay::zero ()) {
                        std::cerr << "Delay requested : " << std::chrono::duration_cast<std::chrono::milliseconds> (d).count () << "ms"
                                  << std::endl;
                        return d;
                }

                // All done, reset.
                state.internal.entry.reset ();
                return Delay::zero ();
        }

        if constexpr (sizeof...(rest) > 0) {
                return processRunResetEntryActions (stateTi, ev, rest...);
        }

        return Delay::zero ();
}

/****************************************************************************/

template <typename Ev, typename S> Delay Machine<Ev, S>::runResetEntryActions (std::type_index const &stateTi, Ev const &ev)
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

template <typename Ev, typename S>
template <typename Q, typename St, typename T, typename... Tr>
StateProcessResult Machine<Ev, S>::processTransitions (Q &&eventQueue, St &state, T &transition, Tr &... rest)
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
                        // if (Delay d = currentState->runExitActions (event); d != Delay::zero ()) {
                        //         std::cerr << "Delay requested : " << std::chrono::duration_cast<std::chrono::milliseconds> (d).count () <<
                        //         "ms"
                        //                   << std::endl;
                        //         return {{}, d};
                        // }

                        if (Delay d = state.internal.exit (event); d != Delay::zero ()) {
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
                        state.internal.exit.reset ();
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

template <typename Ev, typename S>
template <typename St, typename Q, typename... Rs>
StateProcessResult Machine<Ev, S>::processStates (std::type_index const &currentStateTi, Q &&eventQueue, St &state, Rs &... rest)
{
        if (std::type_index (typeid (state.internal.name)) == currentStateTi) {
#ifndef NDEBUG
                std::cout << "Current  : " << state.internal.name.c_str () << std::endl;
#endif
                return boost::hana::unpack (state.internal.transitions, [this, &eventQueue, &state] (auto &... trans) -> StateProcessResult {
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

template <typename Ev, typename S> template <typename Q> void Machine<Ev, S>::run (Q &&eventQueue)
{
        if (!timer.isExpired ()) {
                return;
        }

#ifndef NDEBUG
        std::cout << "== Run ==" << std::endl;
#endif
        Expects (currentName);
        std::type_index &currentStateNameCopy = *currentName;

        if (!currentState) {
                auto initialState = findInitialState ();
                currentState = &initialState;
                std::cout << currentState->getTransitionSizeOf (0) << std::endl;
        }

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
