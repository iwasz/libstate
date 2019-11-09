/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#pragma once
#include "Action.h"
#include "Transition.h"
#include <boost/hana.hpp>

// TODO remove
#include <iostream>

namespace ls {

// TODO This is a GNU extension. Provide macro as an option. Or better still, use hana::string explicitly
template <typename C, C... c> constexpr auto operator""_STATE () { return boost::hana::string_c<c...>; }

/**
 *
 */
template <typename Ev> struct ErasedStateBase {

        ErasedStateBase () = default;
        virtual ~ErasedStateBase () = default;
        ErasedStateBase (ErasedStateBase const &e) = default;
        ErasedStateBase &operator= (ErasedStateBase const &e) = default;
        ErasedStateBase (ErasedStateBase &&e) noexcept = default;
        ErasedStateBase &operator= (ErasedStateBase &&e) noexcept = default;

        virtual const char *getName () const = 0;

        virtual Delay runEntryActions (Ev const &ev) = 0;
        virtual void resetEntryActions () = 0;

        virtual Delay runExitActions (Ev const &ev) = 0;
        virtual void resetExitActions () = 0;

        // TODO maybe return const and make method const and then change ErasedTransition interface to permit reset on const object and...
        virtual ErasedTransitionBase<Ev> *getTransition (size_t index) = 0;
};

/**
 *
 */
template <typename Ev, typename Sn, typename T1 = void, typename T2 = void, typename Tw = boost::hana::tuple<>>
class ErasedState : public ErasedStateBase<Ev> {
public:
        // explicit ErasedState (State<Sn, T1, T2, T3> &s) : name (std::move (s.name)), entry (std::move (s.entry)), exit (std::move (s.exit)) {}
        ErasedState (Sn sn, Entry<T1> en, Exit<T2> ex, Tw ts)
            : name (std::move (sn)), entry (std::move (en)), exit (std::move (ex)), transitions (std::move (ts))
        {
        }

        ErasedState (ErasedState const &e) = default;
        ErasedState &operator= (ErasedState const &e) = default;
        ErasedState (ErasedState &&e) noexcept = default;
        ErasedState &operator= (ErasedState &&e) noexcept = default;

        const char *getName () const override { return name.c_str (); }

        Delay runEntryActions (Ev const &ev) override { return entry (ev); }
        void resetEntryActions () override { entry.reset (); }

        Delay runExitActions (Ev const &ev) override { return exit (ev); }
        void resetExitActions () override { exit.reset (); }

        ErasedTransitionBase<Ev> *getTransition (size_t index) override;

        // private:
        Sn name; // name, entry and exit has the same tyypes and values as in State object
        Entry<T1> entry;
        Exit<T2> exit;
        Tw transitions; // transitions at the other hand are wrapped in ErasedTransition <Ev>
};

/**
 *
 */
template <typename Ev, typename T, typename... Rs>
constexpr ErasedTransitionBase<Ev> *processGetTransition (size_t index, size_t current, T &transition, Rs &... rest)
{
        if (index == current) {
                return &transition;
        }

        if constexpr (sizeof...(rest)) {
                return processGetTransition<Ev> (index, current + 1, rest...);
        }

        return nullptr;
}

/**
 *
 */
template <typename Ev, typename Sn, typename T1, typename T2, typename T3>
ErasedTransitionBase<Ev> *ErasedState<Ev, Sn, T1, T2, T3>::getTransition (size_t index)
{
        // TODO does not work, why can't I get the size from T3 type directly?
        // if constexpr (boost::hana::length (transitions) != boost::hana::size_c<0>) {
        return boost::hana::unpack (transitions, [index] (auto &... trans) -> ErasedTransitionBase<Ev> * {
                if constexpr (sizeof...(trans) > 0) {
                        return processGetTransition<Ev> (index, 0, trans...);
                }

                return nullptr;
        });

        return nullptr; // TODO
}

/**
 *
 */
template <typename Ev, typename Sn, typename T1, typename T2, typename T3>
ErasedState<Ev, Sn, T1, T2, T3> erasedState (Sn sn, Entry<T1> en, Exit<T2> ex, T3 ts)
{
        return ErasedState<Ev, Sn, T1, T2, T3> (std::move (sn), std::move (en), std::move (ex), std::move (ts));
}

/**
 * A state - typesafe version. This is only a "type container" which is used to make
 * more specialized "event aware" types. This one is independent of Event type used.
 */
template <typename Sn, typename En = void, typename Ex = void, typename Tt = boost::hana::tuple<>> class State {
public:
        State () = delete;
        explicit State (Sn sn) : name (std::move (sn)) {}
        State (Sn sn, Entry<En> en) : name (std::move (sn)), entry (std::move (en)) {}
        State (Sn sn, Entry<En> en, Exit<Ex> ex) : name (std::move (sn)), entry (std::move (en)), exit (std::move (ex)) {}
        State (Sn sn, Entry<En> en, Exit<Ex> ex, Tt ts)
            : name (std::move (sn)), entry (std::move (en)), exit (std::move (ex)), transitions (std::move (ts))
        {
        }

        // size_t getTransitionSizeOf (size_t index) const;
        // ErasedTransitionBase< Ev>

        // template <typename Ev>
        // auto makeErasedState () {
        //     //return ErasedState <Ev> (std:move (name), std::move (entry), std::move (exit));
        // }

        // private:
        Sn name;
        Entry<En> entry;
        Exit<Ex> exit;
        Tt transitions;
        // constexpr size_t transitionsNumber;
};

/**
 *
 */
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

/**
 *
 */
// template <typename Sn, typename En, typename Ex, typename Tt> size_t State<Sn, En, Ex, Tt>::getTransitionSizeOf (size_t index) const
// {
//         // TODO does not work, why can't I get the size from T3 type directly?
//         // if constexpr (boost::hana::length (transitions) != boost::hana::size_c<0>) {
//         return boost::hana::unpack (transitions, [index] (auto const &... trans) -> size_t {
//                 if constexpr (sizeof...(trans) > 0) {
//                         return processGetTransitionSizeOf (index, 0, trans...);
//                 }

//                 return 0;
//         });

//         std::cout << std::endl;
//         // }

//         return 0;
// }

/**
 *
 */
template <typename Sn, typename En, typename Ex, typename... Ts> auto state (Sn &&sn, En &&entry, Ex &&exit, Ts &&... trans)
{
        return State (std::forward<decltype (sn)> (sn), std::forward<decltype (entry)> (entry), std::forward<decltype (exit)> (exit),
                      boost::hana::tuple (std::forward<decltype (trans)> (trans)...));
}

/**
 *
 */
template <typename Sn, typename En> auto state (Sn &&sn, En &&entry)
{
        return State (std::forward<decltype (sn)> (sn), std::forward<decltype (entry)> (entry));
}

} // namespace ls