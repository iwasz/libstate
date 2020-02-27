// /****************************************************************************
//  *                                                                          *
//  *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
//  *  ~~~~~~~~                                                                *
//  *  License : see COPYING file for details.                                 *
//  *  ~~~~~~~~~                                                               *
//  ****************************************************************************/

// #pragma once
// #include "Action.h"
// #include "Misc.h"
// #include "State.h"
// #include "Transition.h"
// #include "gsl/pointers"
// #include <chrono>
// #include <cstddef>
// #include <etl/map.h>
// #include <gsl/gsl>
// #include <optional>
// #include <pthread.h>
// #include <tuple>
// #include <type_traits>
// #include <typeindex>

// // TODO remove
// #include <string>
// // TODO remove
// #include <iostream>
// #include <utility>

// namespace ls {

// /**
//  *
//  */
// struct HeapAllocator {

//         // TODO try to understand why I had to overload state instead of perfect forwarding all parameters at once
//         template <typename Ev, typename St> gsl::owner<ErasedStateBase<Ev> *> state (St &&st)
//         {
//                 return new ErasedState<Ev, St> (std::forward<St> (st));
//         }

//         template <typename Ev, typename St, typename Ent, typename = std::enable_if_t<is_entry_v<Ent>>>
//         gsl::owner<ErasedStateBase<Ev> *> state (St &&st, Ent &&en)
//         {
//                 return new ErasedState<Ev, St, Ent> (std::forward<St> (st), std::forward<Ent> (en));
//         }

//         template <typename Ev, typename St, typename Ent, typename... Tra,
//                   typename = std::enable_if_t<std::conjunction_v<is_entry<Ent>, is_transition<Tra>...>>>
//         gsl::owner<ErasedStateBase<Ev> *> state (St &&st, Ent &&en, Tra *... tra)
//         {
//                 return new ErasedState<Ev, St, Ent> (std::forward<St> (st), std::forward<Ent> (en), tra...);
//         }

//         template <typename Ev, typename St, typename Ent, typename Exi, typename... Tra,
//                   typename = std::enable_if_t<std::conjunction_v<is_entry<Ent>, is_exit<Exi>>>>
//         gsl::owner<ErasedStateBase<Ev> *> state (St &&st, Ent &&en, Exi &&ex, Tra *... tra)
//         {
//                 return new ErasedState<Ev, St, Ent, Exi> (std::forward<St> (st), std::forward<Ent> (en), std::forward<Exi> (ex), tra...);
//         }

//         /*--------------------------------------------------------------------------*/

//         template <typename Ev, typename St> gsl::owner<ErasedTransitionBase<Ev> *> transition (St &&st)
//         {
//                 return new Transition<Ev, St> (std::forward<St> (st));
//         }

//         template <typename Ev, typename St, typename Con> gsl::owner<ErasedTransitionBase<Ev> *> transition (St &&st, Con &&con)
//         {
//                 return new Transition<Ev, St, Con> (std::forward<St> (st), std::forward<Con> (con));
//         }

//         template <typename Ev, typename St, typename Con, typename... Tts>
//         gsl::owner<ErasedTransitionBase<Ev> *> transition (St &&st, Con &&con, Tts &&... tts)
//         {
//                 return new Transition<Ev, St, Con, decltype (transitionAction (std::forward<Tts> (tts)...))> (
//                         std::forward<St> (st), std::forward<Con> (con), transitionAction (std::forward<Tts> (tts)...));
//         }

//         /*--------------------------------------------------------------------------*/

//         template <typename Ev> void destroy (gsl::owner<ErasedStateBase<Ev> *> state) { delete state; }
// };

// /**
//  *
//  */
// template <size_t SZ> class StackAllocator {
// public:
//         template <typename Ev, typename... Arg> gsl::owner<ErasedStateBase<Ev> *> allocateState (Arg &&... arg)
//         {
//                 using ThisState = ErasedState<Ev, Arg...>;
//                 auto sz = sizeof (ThisState);
//                 Expects (std::next (end, sz) <= block.end ());
//                 gsl::owner<ThisState *> p = new ((void *)(end)) ThisState (std::forward<Arg> (arg)...);
//                 end += sz;
//                 return p;
//         }

// private:
//         using Block = std::array<uint8_t, SZ>;
//         Block block{};
//         typename Block::iterator end{block.begin ()};
// };

// template <typename Ev, typename Allocator = HeapAllocator, size_t MAX_STATES = 10> class Machine2 {
// public:
//         template <typename Q> void run (Q &&queue);
//         template <typename Q> void waitAndRun (Q &&queue)
//         {
//                 while (isWaiting ()) {
//                 }
//                 run (std::forward<Q> (queue));
//         }
//         bool isWaiting () const { return !timer.isExpired (); }

//         /**
//          * Adds a state.
//          */
//         template <typename... Arg> auto state (Arg &&... args)
//         {
//                 Expects (!states.full ());
//                 auto s = allocator.template state<Ev> (std::forward<Arg> (args)...);
//                 states[s->getKey ()] = s;

//                 if (!initialState) {
//                         initialState = s;
//                 }

//                 return s;
//         }

//         template <typename... Arg> auto transition (Arg &&... arg) { return allocator.template transition<Ev> (std::forward<Arg> (arg)...); }

// private:
//         auto findInitialState ()
//         {
//                 Ensures (initialState);
//                 return initialState;
//         }

// private:
//         Allocator allocator;
//         using StateMap = etl::map<std::type_index, ErasedStateBase<Ev> *, MAX_STATES>;
//         StateMap states;
//         Timer timer{};
//         ErasedStateBase<Ev> *initialState{};
//         ErasedStateBase<Ev> *currentState{};
//         ErasedStateBase<Ev> *prevState{};
// };

// /****************************************************************************/

// template <typename Ev, typename Allocator, size_t MAX_STATES>
// template <typename Q>
// void Machine2<Ev, Allocator, MAX_STATES>::run (Q &&eventQueue)
// {

//         if (!timer.isExpired ()) {
//                 return;
//         }

// #ifndef NDEBUG
//         std::cout << "== Run ==" << std::endl;
// #endif

//         if (!currentState) {
//                 currentState = findInitialState ();
//         }

// #if 1
//         for (ErasedTransitionBase<Ev> *trans = currentState->getTransition (); trans != nullptr; trans = trans->next) {
//                 for (auto const &event : eventQueue) {
//                         if (!trans->checkCondition (event)) {
//                                 continue;
//                         }
// #ifndef NDEBUG
//                         std::cout << "Transition to : " << trans->getStateName () << std::endl;
// #endif
//                         if (Delay d = currentState->runExitActions (event); d != DELAY_ZERO) {
//                                 std::cerr << "Delay requested : " << std::chrono::duration_cast<std::chrono::milliseconds> (d).count () <<
//                                 "ms"
//                                           << std::endl;

//                                 timer.start (std::chrono::duration_cast<std::chrono::milliseconds> (d).count ());
//                                 goto end;
//                         }

//                         if (Delay d = trans->runActions (event); d != DELAY_ZERO) {
//                                 std::cerr << "Delay requested : " << std::chrono::duration_cast<std::chrono::milliseconds> (d).count () <<
//                                 "ms"
//                                           << std::endl;

//                                 timer.start (std::chrono::duration_cast<std::chrono::milliseconds> (d).count ());
//                                 goto end;
//                         }

//                         prevState = currentState;
//                         currentState = states.at (trans->getStateIndex ());
//                         Ensures (currentState);

//                         currentState->runEntryActions (event);
//                         eventQueue.clear (); // TODO not quite like that

//                         if (prevState) {
//                                 prevState->resetExitActions ();
//                         }

//                         trans->resetActions ();
//                         currentState->resetEntryActions ();
//                 }
//         }

// end:;
// #endif
// }

// } // namespace ls
