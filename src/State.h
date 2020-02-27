// /****************************************************************************
//  *                                                                          *
//  *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
//  *  ~~~~~~~~                                                                *
//  *  License : see COPYING file for details.                                 *
//  *  ~~~~~~~~~                                                               *
//  ****************************************************************************/

// #pragma once
// #include "Action.h"
// #include "String.h"
// #include "Transition.h"
// #include <tuple>

// namespace ls {

// // TODO This is a GNU extension. Provide macro as an option. Or better still, use hana::string explicitly
// template <typename C, C... c> constexpr auto operator""_STATE () { return ls::string_c<c...>; }

// /**
//  *
//  */
// template <typename Ev> struct ErasedStateBase {

//         ErasedStateBase () = default;
//         virtual ~ErasedStateBase () = default;
//         ErasedStateBase (ErasedStateBase const &e) = default;
//         ErasedStateBase &operator= (ErasedStateBase const &e) = default;
//         ErasedStateBase (ErasedStateBase &&e) noexcept = default;
//         ErasedStateBase &operator= (ErasedStateBase &&e) noexcept = default;

//         virtual const char *getName () const = 0;
//         virtual std::type_index getKey () const = 0;

//         virtual Delay runEntryActions (Ev const &ev) = 0;
//         virtual void resetEntryActions () = 0;

//         virtual Delay runExitActions (Ev const &ev) = 0;
//         virtual void resetExitActions () = 0;

//         // TODO maybe return const and make method const and then change ErasedTransition interface to permit reset on const object and...
//         virtual ErasedTransitionBase<Ev> *getTransition () = 0;
// };

// /**
//  *
//  */
// template <typename Ev, typename Sn, typename Ent = ActionTuple<void>, typename Exi = ActionTuple<void>>
// class ErasedState : public ErasedStateBase<Ev> {
// public:
//         explicit ErasedState (Sn sn) : name (std::move (sn)) {}
//         ErasedState (Sn sn, Ent en) : name (std::move (sn)), entryTuple (std::move (en)) {}
//         template <typename... Tra> ErasedState (Sn sn, Ent en, Tra *... tra) : name (std::move (sn)), entryTuple (std::move (en))
//         {
//                 setTransitions (&transition, tra...);
//         }

//         template <typename... Tra>
//         ErasedState (Sn sn, Ent en, Exi ex, Tra *... tra) : name (std::move (sn)), entryTuple (std::move (en)), exitTuple (std::move (ex))
//         {
//                 setTransitions (&transition, tra...);
//         }

//         ErasedState (ErasedState const &e) = default;
//         ErasedState &operator= (ErasedState const &e) = default;
//         ErasedState (ErasedState &&e) noexcept = default;
//         ErasedState &operator= (ErasedState &&e) noexcept = default;
//         ~ErasedState () = default;

//         const char *getName () const override { return name.c_str (); }
//         virtual std::type_index getKey () const { return std::type_index{typeid (name)}; }

//         Delay runEntryActions (Ev const &ev) override { return entryTuple (ev); }
//         void resetEntryActions () override { entryTuple.reset (); }

//         Delay runExitActions (Ev const &ev) override { return exitTuple (ev); }
//         void resetExitActions () override { exitTuple.reset (); }

//         ErasedTransitionBase<Ev> *getTransition () override { return transition; }

// private:
//         template <typename Tr, typename... Rs> void setTransitions (ErasedTransitionBase<Ev> **current, Tr *tr, Rs *... rs)
//         {
//                 if (*current == nullptr) {
//                         *current = tr;
//                 }
//                 else {
//                         (*current)->next = tr;
//                 }

//                 current = &(*current)->next;
//                 if constexpr (sizeof...(rs)) {
//                         setTransitions (current, rs...);
//                 }
//         }

// private:
//         Sn name;
//         Ent entryTuple;
//         Exi exitTuple;
//         ErasedTransitionBase<Ev> *transition{};
// };

// #if 0
// /**
//  *
//  */
// template <typename Ev, typename T, typename... Rs>
// constexpr ErasedTransitionBase<Ev> *processGetTransition (size_t index, size_t current, T &transition, Rs &... rest)
// {
//         if (index == current) {
//                 return &transition;
//         }

//         if constexpr (sizeof...(rest)) {
//                 return processGetTransition<Ev> (index, current + 1, rest...);
//         }

//         return nullptr;
// }

// /**
//  *
//  */
// template <typename Ev, typename Sn, typename T1, typename T2, typename T3>
// ErasedTransitionBase<Ev> *ErasedState<Ev, Sn, T1, T2, T3>::getTransition (size_t index)
// {
//         return std::apply ([index] (auto &... trans) -> ErasedTransitionBase<Ev> * {
//                 if constexpr (sizeof...(trans) > 0) {
//                         return processGetTransition<Ev> (index, 0, trans...);
//                 }

//                 return nullptr;
//         }, transitions);
// }
// #endif

// /**
//  * A state - typesafe version. This is only a "type container" which is used to make
//  * more specialized "event aware" types. This one is independent of Event type used.
//  */
// // template <typename Sn, typename En = void, typename Ex = void, typename Tt = std::tuple<>> class State {
// // public:
// //         State () = delete;
// //         explicit State (Sn sn) : name (std::move (sn)) {}
// //         State (Sn sn, Entry<En> en) : name (std::move (sn)), entry (std::move (en)) {}
// //         State (Sn sn, Entry<En> en, Exit<Ex> ex) : name (std::move (sn)), entry (std::move (en)), exit (std::move (ex)) {}
// //         State (Sn sn, Entry<En> en, Exit<Ex> ex, Tt ts)
// //             : name (std::move (sn)), entry (std::move (en)), exit (std::move (ex)), transitions (std::move (ts))
// //         {
// //         }

// // private:
// //         Sn name;
// //         Entry<En> entry;
// //         Exit<Ex> exit;
// //         Tt transitions;
// //         // constexpr size_t transitionsNumber;
// // };

// /**
//  *
//  */
// // template <typename T, typename... Rs>
// // constexpr size_t processGetTransitionSizeOf (size_t index, size_t current, T const &transition, Rs const &... rest)
// // {
// //         if (index == current) {
// //                 return sizeof (transition);
// //         }

// //         if constexpr (sizeof...(rest)) {
// //                 return processGetTransitionSizeOf (index, current + 1, rest...);
// //         }

// //         return 0;
// // }

// // /**
// //  *
// //  */
// // template <typename Sn, typename En, typename Ex, typename... Ts> auto state (Sn &&sn, En &&entry, Ex &&exit, Ts &&... trans)
// // {
// //         return State (std::forward<decltype (sn)> (sn), std::forward<decltype (entry)> (entry), std::forward<decltype (exit)> (exit),
// //                       std::tuple (std::forward<decltype (trans)> (trans)...));
// // }

// // /**
// //  *
// //  */
// // template <typename Sn, typename En> auto state (Sn &&sn, En &&entry)
// // {
// //         return State (std::forward<decltype (sn)> (sn), std::forward<decltype (entry)> (entry));
// // }

// } // namespace ls