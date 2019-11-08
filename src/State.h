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

        virtual Delay runEntryActions (Ev const &ev) = 0;
        virtual Delay runExitActions (Ev const &ev) = 0;

        // Transitions - but how?
        virtual size_t getTransitionSizeOf (size_t index) const = 0;
        virtual ErasedTransitionBase<Ev> *getTransition (size_t index) const = 0;
};

/**
 * TODO change T3 to Tw to denote difference. T3 is tuple of Transitions, and Tw would be tuple of Erased Transitions
 */
template <typename Ev, typename Sn, typename T1 = void, typename T2 = void, typename T3 = boost::hana::tuple<>>
class ErasedState : public ErasedStateBase<Ev> {
public:
        // explicit ErasedState (State<Sn, T1, T2, T3> &s) : name (std::move (s.name)), entry (std::move (s.entry)), exit (std::move (s.exit)) {}
        ErasedState (Sn sn, Entry<T1> en, Exit<T2> ex, T3 ts)
            : name (std::move (sn)), entry (std::move (en)), exit (std::move (ex)), transitions (std::move (ts))
        {
        }

        Delay runEntryActions (Ev const &ev) override
        { /* return internal.runEntryActions (ev); */
        }
        Delay runExitActions (Ev const &ev) override
        { /*  return internal.runExitActions (ev); */
        }

        size_t getTransitionSizeOf (size_t index) const override
        { /* return internal.getTransitionSizeOf (index);  */
        }
        ErasedTransitionBase<Ev> *getTransition (size_t index) const override;

        // S internal;

        Sn name; // name, entry and exit has the same tyypes and values as in State object
        Entry<T1> entry;
        Exit<T2> exit;
        T3 transitions; // transitions at the other hand are wrapped in ErasedTransition <Ev>
};

/**
 *
 */
template <typename Ev, typename T, typename... Rs>
constexpr ErasedTransitionBase<Ev> *processGetTransition (size_t index, size_t current, T const &transition, Rs const &... rest)
{
        if (index == current) {
                return nullptr; // &transition;
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
ErasedTransitionBase<Ev> *ErasedState<Ev, Sn, T1, T2, T3>::getTransition (size_t index) const
{
        // TODO does not work, why can't I get the size from T3 type directly?
        // if constexpr (boost::hana::length (transitions) != boost::hana::size_c<0>) {
        // return boost::hana::unpack (internal.transitions, [index] (auto const &... trans) -> ErasedTransitionBase<Ev> * {
        //         if constexpr (sizeof...(trans) > 0) {
        //                 return processGetTransition<Ev> (index, 0, trans...);
        //         }

        //         return nullptr;
        // });

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
        // ErasedTransitionBase< Ev>

        // template <typename Ev>
        // auto makeErasedState () {
        //     //return ErasedState <Ev> (std:move (name), std::move (entry), std::move (exit));
        // }

        // private:
        Sn name;
        Entry<T1> entry;
        Exit<T2> exit;
        T3 transitions;
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

/**
 *
 */
template <typename Sn, typename Entry, typename Exit, typename... Trans> auto state (Sn &&sn, Entry &&entry, Exit &&exit, Trans &&... trans)
{
        return State (std::forward<decltype (sn)> (sn), std::forward<decltype (entry)> (entry), std::forward<decltype (exit)> (exit),
                      boost::hana::tuple (std::forward<decltype (trans)> (trans)...));
}

/**
 *
 */
template <typename Sn, typename Entry> auto state (Sn &&sn, Entry &&entry)
{
        return State (std::forward<decltype (sn)> (sn), std::forward<decltype (entry)> (entry));
}

} // namespace ls