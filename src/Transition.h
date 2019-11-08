/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#pragma once
#include "Action.h"
#include <typeindex>
#include <utility>

namespace ls {

/**
 *
 */
template <typename Sn = int, typename C = int, typename T = int> class Transition {
public:
        explicit Transition (Sn sn) : stateName (std::move (sn)) {}
        Transition (Sn sn, C c) : stateName (std::move (sn)), condition (std::move (c)) {}
        Transition (Sn sn, C c, T t) : stateName (std::move (sn)), condition (std::move (c)), actions (std::move (t)) {}

        template <typename Ev> Delay runActions (Ev const &ev) { return actions (ev); }
        void resetActions () { actions.reset (); }

        template <typename Ev> bool checkCondition (Ev const &ev) const
        {
                if constexpr (std::is_invocable_v<C, Ev>) {
                        return condition (ev);
                }
                else {
                        static_assert (
                                std::is_invocable_v<C>,
                                "Incompatibility between an event, and a condition(s) which checks this event. Conditions may have a single "
                                "event argument or no arguments at all.");

                        return condition ();
                }
        }

        std::type_index getStateIndex () const { return std::type_index (typeid (stateName)); }
        const char *getStateName () const { return stateName.c_str (); }

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

/**
 *
 */
template <typename Ev> struct ErasedTransitionBase {

        ErasedTransitionBase () noexcept = default;
        virtual ~ErasedTransitionBase () noexcept = default;
        ErasedTransitionBase (ErasedTransitionBase const &) noexcept = default;
        ErasedTransitionBase &operator= (ErasedTransitionBase const &) noexcept = default;
        ErasedTransitionBase (ErasedTransitionBase &&) noexcept = default;
        ErasedTransitionBase &operator= (ErasedTransitionBase &&) noexcept = default;

        virtual Delay runActions (Ev const &ev) = 0;
        virtual void resetActions () = 0;
        virtual bool checkCondition (Ev const &ev) const = 0;
        virtual std::type_index getStateIndex () const = 0;
        virtual const char *getStateName () const = 0;
};

/**
 *
 */
template <typename Ev, typename Tr> struct ErasedTransition : public ErasedTransitionBase<Ev> {

        explicit ErasedTransition (Tr tr) noexcept : internal (std::move (tr)) {}
        virtual ~ErasedTransition () noexcept = default;
        ErasedTransition (ErasedTransition const &) noexcept = default;
        ErasedTransition &operator= (ErasedTransition const &) noexcept = default;
        ErasedTransition (ErasedTransition &&) noexcept = default;
        ErasedTransition &operator= (ErasedTransition &&) noexcept = default;

        Delay runActions (Ev const &ev) override { return internal.runActions (ev); }
        void resetActions () override { internal.resetActions (); }
        bool checkCondition (Ev const &ev) const override { return internal.checkCondition (ev); }
        std::type_index getStateIndex () const override { return internal.getStateIndex (); }
        const char *getStateName () const override { return internal.getStateName (); }

        Tr internal;
};

template <typename Ev, typename Tr> auto erasedTransition (Tr tr) { return ErasedTransition<Ev, Tr> (std::move (tr)); }

template <typename Ev, typename Tt> auto erasedTransitions (Tt &&tt)
{
        // There's probably easier way of doing that in hana.
        return boost::hana::unpack (tt, [] (auto... transition) { return boost::hana::make_tuple (erasedTransition<Ev> (transition)...); });
}

} // namespace ls