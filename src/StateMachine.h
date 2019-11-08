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
#include "State.h"
#include "Transition.h"
#include <boost/hana.hpp>
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
                currentName = std::type_index (typeid (initialState.name));

#ifndef NDEBUG
                std::cout << "Initial : " << initialState.name.c_str () << std::endl;
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
        auto getState (std::type_index const & /* ti*/);

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
        ErasedStateBase<Ev> *prevState{};
        Timer timer{};
};

/*--------------------------------------------------------------------------*/

/// Helper for creating a machine.
template <typename Ev, typename... Sts> auto machine (Sts &&... states)
{
        auto s = boost::hana::make_tuple (
                erasedState<Ev> ("_"_STATE, entry ([] {}), exit ([] {}),
                                 erasedTransitions<Ev> (boost::hana::make_tuple (transition ("INIT"_STATE, [] (auto /*ev*/) { return true; })))),
                erasedState<Ev> (states.name, states.entry, states.exit, erasedTransitions<Ev> (states.transitions))...);

        return Machine<Ev, decltype (s)> (std::move (s));
}

/****************************************************************************/

template <typename Ev, typename S> auto Machine<Ev, S>::findInitialState () const
{
        auto initialState
                = boost::hana::find_if (states, [] (auto const &state) { return state.name == boost::hana::string_c<'I', 'N', 'I', 'T'>; });

        static_assert (initialState != boost::hana::nothing, "Initial state has to be named \"INIT\"_STATE and it must be defined.");

        return *boost::hana::find_if (states, [] (auto const &state) { return state.name == boost::hana::string_c<'_'>; });
}

template <typename Ev, typename St, typename... Rs> ErasedStateBase<Ev> *processGetState (std::type_index const &ti, St &st, Rs &... rest)
{
        if (ti == std::type_index (typeid (st))) {
                return &st;
        }

        if constexpr (sizeof...(rest)) {
                return processGetState<Ev> (ti, rest...);
        }

        return nullptr;
}

template <typename Ev, typename S> auto Machine<Ev, S>::getState (std::type_index const &ti)
{
        return boost::hana::unpack (states, [&ti] (auto &... sts) -> ErasedStateBase<Ev> * {
                if constexpr (sizeof...(sts)) {
                        return processGetState<Ev> (ti, sts...);
                }

                return nullptr;
        });
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

template <typename Ev, typename S> Delay Machine<Ev, S>::runResetEntryActions (std::type_index const &stateTi, Ev const &ev)
{
        return boost::hana::unpack (states, [&stateTi, &ev] (auto &... states) { return processRunResetEntryActions (stateTi, ev, states...); });
}

/****************************************************************************/

template <typename Ev, typename S>
template <typename Q, typename St, typename T, typename... Tr>
StateProcessResult Machine<Ev, S>::processTransitions (Q &&eventQueue, St &state, T &transition, Tr &... rest)
{
        for (auto const &event : eventQueue) {

                // Perform the transition
                // TODO accept conditions without an argument
                // if (checkCondition (transition.condition, event)) {
                if (transition.checkCondition (event)) {
#ifndef NDEBUG
                        std::cout << "Transition to : " << transition.getStateName () << std::endl;
#endif
                        // Run curent.exit
                        // if (Delay d = currentState->runExitActions (event); d != Delay::zero ()) {
                        //         std::cerr << "Delay requested : " << std::chrono::duration_cast<std::chrono::milliseconds> (d).count () <<
                        //         "ms"
                        //                   << std::endl;
                        //         return {{}, d};
                        // }

                        if (Delay d = state.exit (event); d != Delay::zero ()) {
                                std::cerr << "Delay requested : " << std::chrono::duration_cast<std::chrono::milliseconds> (d).count () << "ms"
                                          << std::endl;
                                return {{}, d};
                        }

                        if (Delay d = transition.runActions (event); d != Delay::zero ()) {
                                std::cerr << "Delay requested : " << std::chrono::duration_cast<std::chrono::milliseconds> (d).count () << "ms"
                                          << std::endl;
                                return {{}, d};
                        }

                        // Change current name.
                        auto ret = transition.getStateIndex ();

                        // - run current.entry
                        runResetEntryActions (ret, event);

                        eventQueue.clear (); // TODO not quite like that
                        state.exit.reset ();
                        transition.resetActions ();
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

template <typename Ev, typename S> template <typename Q> void Machine<Ev, S>::run (Q &&eventQueue)
{
        if (!timer.isExpired ()) {
                return;
        }

#ifndef NDEBUG
        std::cout << "== Run ==" << std::endl;
#endif
        Expects (currentName);
        // std::type_index &currentStateNameCopy = *currentName;

        if (!currentState) {
                auto initialState = findInitialState ();
                currentState = &initialState;
                // std::cout << currentState->getTransitionSizeOf (0) << std::endl;
        }

        int i = 0;
        ErasedTransitionBase<Ev> *trans{};
        while ((trans = currentState->getTransition (i++))) {

                for (auto const &event : eventQueue) {

                        // Perform the transition
                        if (trans->checkCondition (event)) {
#ifndef NDEBUG
                                std::cout << "Transition to : " << trans->getStateName () << std::endl;
#endif
                                if (Delay d = currentState->runExitActions (event); d != Delay::zero ()) {
                                        std::cerr << "Delay requested : " << std::chrono::duration_cast<std::chrono::milliseconds> (d).count ()
                                                  << "ms" << std::endl;

                                        timer.start (std::chrono::duration_cast<std::chrono::milliseconds> (d).count ());
                                        goto end;
                                }

                                if (Delay d = trans->runActions (event); d != Delay::zero ()) {
                                        std::cerr << "Delay requested : " << std::chrono::duration_cast<std::chrono::milliseconds> (d).count ()
                                                  << "ms" << std::endl;

                                        timer.start (std::chrono::duration_cast<std::chrono::milliseconds> (d).count ());
                                        goto end;
                                }

                                // Change current name.
                                prevState = currentState;
                                currentState = getState (trans->getStateIndex ());

                                // - run current.entry
                                currentState->runEntryActions (event);
                                eventQueue.clear (); // TODO not quite like that

                                if (prevState) {
                                        prevState->resetExitActions ();
                                }

                                trans->resetActions ();
                                currentState->resetEntryActions ();
                        }
                }
        }

end:;

        // StateProcessResult result
        //         = boost::hana::unpack (states, [this, &currentStateNameCopy, &eventQueue] (auto &... arg) -> StateProcessResult {
        //                   if constexpr (sizeof...(arg)) {
        //                           return processStates (currentStateNameCopy, eventQueue, arg...);
        //                   }

        //                   return {};
        //           });

        // if (result.newStateName) {
        //         currentName = result.newStateName;
        // }
        // else if (result.delay != Delay::zero ()) {
        //         // TODO modify timer, and get rid of the cast.
        //         timer.start (std::chrono::duration_cast<std::chrono::milliseconds> (result.delay).count ());
        // }
}

} // namespace ls
