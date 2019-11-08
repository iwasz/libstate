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
        // ErasedTransitionBase< Ev>

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
 *
 */
template <typename Ev, typename S> class ErasedState : public ErasedStateBase<Ev> {
public:
        explicit ErasedState (S s) : internal (std::move (s)) {}
        Delay runEntryActions (Ev const &ev) override { return internal.runEntryActions (ev); }
        Delay runExitActions (Ev const &ev) override { return internal.runExitActions (ev); }

        size_t getTransitionSizeOf (size_t index) const override { return internal.getTransitionSizeOf (index); }
        ErasedTransitionBase<Ev> *getTransition (size_t index) const override;

        S internal;
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
template <typename Ev, typename S> ErasedTransitionBase<Ev> *ErasedState<Ev, S>::getTransition (size_t index) const
{
        // TODO does not work, why can't I get the size from T3 type directly?
        // if constexpr (boost::hana::length (transitions) != boost::hana::size_c<0>) {
        return boost::hana::unpack (internal.transitions, [index] (auto const &... trans) -> ErasedTransitionBase<Ev> * {
                if constexpr (sizeof...(trans) > 0) {
                        return processGetTransition<Ev> (index, 0, trans...);
                }

                return nullptr;
        });
}

/**
 *
 */
template <typename Ev, typename S> ErasedState<Ev, S> erasedState (S s) { return ErasedState<Ev, S> (std::move (s)); }

} // namespace ls