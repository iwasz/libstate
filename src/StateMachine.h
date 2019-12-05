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
#include "gsl/pointers"
#include <boost/hana.hpp>
#include <boost/hana/fwd/for_each.hpp>
#include <chrono>
#include <cstddef>
#include <etl/map.h>
#include <gsl/gsl>
#include <optional>
#include <pthread.h>
#include <type_traits>
#include <typeindex>

// TODO remove
#include <string>
// TODO remove
#include <iostream>
#include <utility>

namespace ls {

/**
 *
 */
struct HeapAllocator {

        // TODO try to understand why I had to overload state instead of perfect forwarding all parameters at once
        template <typename Ev, typename St> gsl::owner<ErasedStateBase<Ev> *> state (St &&st)
        {
                return new ErasedState<Ev, St> (std::forward<St> (st));
        }

        template <typename Ev, typename St, typename Ent, typename = std::enable_if_t<is_entry_v<Ent>>>
        gsl::owner<ErasedStateBase<Ev> *> state (St &&st, Ent &&en)
        {
                return new ErasedState<Ev, St, Ent> (std::forward<St> (st), std::forward<Ent> (en));
        }

        template <typename Ev, typename St, typename Ent, typename... Tra,
                  typename = std::enable_if_t<std::conjunction_v<is_entry<Ent>, is_transition<Tra>...>>>
        gsl::owner<ErasedStateBase<Ev> *> state (St &&st, Ent &&en, Tra *... tra)
        {
                return new ErasedState<Ev, St, Ent> (std::forward<St> (st), std::forward<Ent> (en), tra...);
        }

        template <typename Ev, typename St, typename Ent, typename Exi, typename... Tra,
                  typename = std::enable_if_t<std::conjunction_v<is_entry<Ent>, is_exit<Exi>>>>
        gsl::owner<ErasedStateBase<Ev> *> state (St &&st, Ent &&en, Exi &&ex, Tra *... tra)
        {
                return new ErasedState<Ev, St, Ent, Exi> (std::forward<St> (st), std::forward<Ent> (en), std::forward<Exi> (ex), tra...);
        }

        /*--------------------------------------------------------------------------*/

        template <typename Ev, typename St> gsl::owner<ErasedTransitionBase<Ev> *> transition (St &&st)
        {
                return new Transition<Ev, St> (std::forward<St> (st));
        }

        template <typename Ev, typename St, typename Con> gsl::owner<ErasedTransitionBase<Ev> *> transition (St &&st, Con &&con)
        {
                return new Transition<Ev, St, Con> (std::forward<St> (st), std::forward<Con> (con));
        }

        template <typename Ev, typename St, typename Con, typename... Tts>
        gsl::owner<ErasedTransitionBase<Ev> *> transition (St &&st, Con &&con, Tts &&... tts)
        {
                return new Transition<Ev, St, Con, decltype (transitionAction (std::forward<Tts> (tts)...))> (
                        std::forward<St> (st), std::forward<Con> (con), transitionAction (std::forward<Tts> (tts)...));
        }

        /*--------------------------------------------------------------------------*/

        template <typename Ev> void destroy (gsl::owner<ErasedStateBase<Ev> *> state) { delete state; }
};

/**
 *
 */
template <size_t SZ> class StackAllocator {
public:
        template <typename Ev, typename... Arg> gsl::owner<ErasedStateBase<Ev> *> allocateState (Arg &&... arg)
        {
                using ThisState = ErasedState<Ev, Arg...>;
                auto sz = sizeof (ThisState);
                Expects (std::next (end, sz) <= block.end ());
                gsl::owner<ThisState *> p = new ((void *)(end)) ThisState (std::forward<Arg> (arg)...);
                end += sz;
                return p;
        }

private:
        using Block = std::array<uint8_t, SZ>;
        Block block{};
        typename Block::iterator end{block.begin ()};
};

template <typename Ev, typename Allocator = HeapAllocator, size_t MAX_STATES = 10> class Machine2 {
public:
        /**
         * Adds a state.
         */
        template <typename... Arg> void state (Arg &&... args)
        {
                Expects (!states.full ());
                auto s = allocator.template state<Ev> (std::forward<Arg> (args)...);
                states[s->getKey ()] = s;
        }

        template <typename... Arg> auto transition (Arg &&... arg) { return allocator.template transition<Ev> (std::forward<Arg> (arg)...); }

private:
        Allocator allocator;

        using StateMap = etl::map<std::type_index, ErasedStateBase<Ev> *, MAX_STATES>;
        StateMap states;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/**
 *
 */
template <typename Ev, typename S> class Machine {
public:
        explicit Machine (S s) : states{std::move (s)}
        {
                // Look for initial state if current is empty (std::optional is used).
                // auto initialState = findInitialState ();
                // currentName = std::type_index (typeid (initialState.name));

#ifndef NDEBUG
                // std::cout << "Initial : " << initialState.name.c_str () << std::endl;
#endif
                Machine *that = this;

                boost::hana::for_each (states, [that] (auto &s) {                // ErasedState <xyz>
                        boost::hana::for_each (s.transitions, [that] (auto &t) { // ErasedTransition <xyz>
                                t.stateIdx = that->stateNameToIndex (t.internal.stateName);
                        });
                });
        }

        template <typename Q> void run (Q &&queue);
        template <typename Q> void waitAndRun (Q &&queue)
        {
                while (isWaiting ()) {
                }
                run (std::forward<Q> (queue));
        }

        auto getCurrentStateName () const { return currentName; } // TODO remove
        const char *getCurrentStateName2 () const { return currentState->getName (); }
        bool isWaiting () const { return !timer.isExpired (); }

        template <typename Sn> constexpr size_t stateNameToIndex (Sn &sn);
        constexpr ErasedStateBase<Ev> *indexToState (size_t i);

private:
        auto findInitialState () const;
        // auto getState (std::type_index const & /* ti*/) -> ErasedStateBase<Ev> *;

        ErasedStateBase<Ev> *findState (std::type_index const &stateIndex) {}

private:
        S states;
        std::optional<std::type_index> currentName{};
        ErasedStateBase<Ev> *currentState{};
        ErasedStateBase<Ev> *prevState{};
        Timer timer{};
};

/*--------------------------------------------------------------------------*/

/// Helper for creating a machine.
// template <typename Ev, typename... Sts> auto machine (Sts &&... states)
// {
//         auto s = boost::hana::make_tuple (
//                 erasedState<Ev> ("_"_STATE, entry ([] {}), exit ([] {}),
//                                  erasedTransitions<Ev> (boost::hana::make_tuple (transition ("INIT"_STATE, [] (auto /*ev*/) { return true;
//                                  })))),
//                 erasedState<Ev> (states.name, states.entry, states.exit, erasedTransitions<Ev> (states.transitions))...);

//         return Machine<Ev, decltype (s)> (std::move (s));
// }

/****************************************************************************/

template <typename Ev, typename S> auto Machine<Ev, S>::findInitialState () const
{
        auto initialState
                = boost::hana::find_if (states, [] (auto const &state) { return state.name == boost::hana::string_c<'I', 'N', 'I', 'T'>; });

        static_assert (initialState != boost::hana::nothing, "Initial state has to be named \"INIT\"_STATE and it must be defined.");

        auto baseState = boost::hana::find_if (states, [] (auto const &state) { return state.name == boost::hana::string_c<'_'>; });

        static_assert (baseState != boost::hana::nothing, "No base state (internal error).");

        return *baseState;
}

// template <typename Ev, typename St, typename... Rs> ErasedStateBase<Ev> *processGetState (std::type_index const &ti, St &st, Rs &... rest)
// {
//         if (ti == std::type_index (typeid (st.name))) {
//                 return &st;
//         }

//         if constexpr (sizeof...(rest)) {
//                 return processGetState<Ev> (ti, rest...);
//         }

//         return nullptr;
// }

// template <typename Ev, typename S> auto Machine<Ev, S>::getState (std::type_index const &ti) -> ErasedStateBase<Ev> *
// {
//         return boost::hana::unpack (states, [&ti] (auto &... sts) -> ErasedStateBase<Ev> * {
//                 if constexpr (sizeof...(sts)) {
//                         return processGetState<Ev> (ti, sts...);
//                 }

//                 return nullptr;
//         });
// }

template <typename Ev, typename St, typename... Rs>
constexpr ErasedStateBase<Ev> *processIndexToState (size_t i, size_t currentIdx, St &st, Rs &... rest)
{
        if (i == currentIdx) {
                return &st;
        }

        if constexpr (sizeof...(rest)) {
                return processIndexToState<Ev> (i, currentIdx + 1, rest...);
        }

        return nullptr;
}

template <typename Ev, typename S> constexpr ErasedStateBase<Ev> *Machine<Ev, S>::indexToState (size_t i)
{
        return boost::hana::unpack (states, [&i] (auto &... sts) -> ErasedStateBase<Ev> * {
                if constexpr (sizeof...(sts)) {
                        return processIndexToState<Ev> (i, 0, sts...);
                }

                return nullptr;
        });
}

/****************************************************************************/

template <typename Ev, typename Sn, typename St, typename... Rs>
constexpr size_t processStateNameToIndex (Sn &sn, size_t currentIdx, St &st, Rs &... rest)
{
        if (std::is_same_v<Sn, decltype (st.name)>) {
                return currentIdx;
        }

        if constexpr (sizeof...(rest)) {
                return processStateNameToIndex<Ev> (sn, currentIdx + 1, rest...);
        }

        return size_t (-1);
}

template <typename Ev, typename S> template <typename Sn> constexpr size_t Machine<Ev, S>::stateNameToIndex (Sn &sn)
{
        return boost::hana::unpack (states, [&sn] (auto &... sts) -> size_t {
                if constexpr (sizeof...(sts)) {
                        return processStateNameToIndex<Ev> (sn, 0, sts...);
                }

                return size_t (-1);
        });
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
        // Expects (currentName);
        // std::type_index &currentStateNameCopy = *currentName;

        if (!currentState) {
                // auto initialState = findInitialState ();

                auto initialState = findInitialState ();

                // currentName = std::type_index (typeid (initialState.name));
                // currentState = getState (*currentName);

                currentState = indexToState (0);
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
                                if (Delay d = currentState->runExitActions (event); d != DELAY_ZERO) {
                                        std::cerr << "Delay requested : " << std::chrono::duration_cast<std::chrono::milliseconds> (d).count ()
                                                  << "ms" << std::endl;

                                        timer.start (std::chrono::duration_cast<std::chrono::milliseconds> (d).count ());
                                        goto end;
                                }

                                if (Delay d = trans->runActions (event); d != DELAY_ZERO) {
                                        std::cerr << "Delay requested : " << std::chrono::duration_cast<std::chrono::milliseconds> (d).count ()
                                                  << "ms" << std::endl;

                                        timer.start (std::chrono::duration_cast<std::chrono::milliseconds> (d).count ());
                                        goto end;
                                }

                                // Change current name.
                                prevState = currentState;
                                // currentState = getState (trans->getStateIndex ());
                                currentState = indexToState (trans->getStateIdx ());

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
}

} // namespace ls
