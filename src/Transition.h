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

        ErasedTransitionBase *next{};
};

struct Condition {
        bool operator() () const { return false; }
};

/**
 *
 */
template <typename Ev, typename Sn, typename Con = Condition, typename Tac = ActionTuple<void>>
class Transition : public ErasedTransitionBase<Ev> {
public:
        explicit Transition (Sn sn) : stateName (std::move (sn)) {}
        Transition (Sn sn, Con c) : stateName (std::move (sn)), condition (std::move (c)) {}
        Transition (Sn sn, Con c, Tac tac) : stateName (std::move (sn)), condition (std::move (c)), transitionActionsTuple (std::move (tac)) {}

        Transition (Transition const &t) : stateName (t.stateName), condition (t.condition), transitionActionsTuple (t.transitionActions) {}
        Transition &operator= (Transition const &t) = default;
        Transition (Transition &&t) noexcept = default;
        Transition &operator= (Transition &&t) noexcept = default;
        ~Transition () = default;

        Delay runActions (Ev const &ev) override { return transitionActionsTuple (ev); }
        void resetActions () override { transitionActionsTuple.reset (); }

        bool checkCondition (Ev const &ev) const override
        {
                if constexpr (std::is_invocable_v<Con, Ev>) {
                        return condition (ev);
                }
                else {
                        static_assert (
                                std::is_invocable_v<Con>,
                                "Incompatibility between an event, and a condition(s) which checks this event. Conditions may have a single "
                                "event argument or no arguments at all.");

                        return condition ();
                }
        }

        std::type_index getStateIndex () const override { return std::type_index (typeid (stateName)); }
        const char *getStateName () const override { return stateName.c_str (); }

        // private:
        Sn stateName;
        Con condition;
        Tac transitionActionsTuple;
};

template <typename Sn, typename Cond, typename... Acts> auto transition (Sn &&sn, Cond &&cond, Acts &&... acts)
{
        return Transition (std::forward<decltype (sn)> (sn), std::forward<decltype (cond)> (cond),
                           transitionAction (std::forward<decltype (acts)> (acts)...));
};

/**
 *
 */
// template <typename Ev, typename Tr> struct ErasedTransition : public ErasedTransitionBase<Ev> {

//         explicit ErasedTransition (Tr tr) noexcept : internal (std::move (tr)) {}
//         virtual ~ErasedTransition () noexcept = default;
//         ErasedTransition (ErasedTransition const &t) noexcept : internal (t.internal) {}
//         ErasedTransition &operator= (ErasedTransition const &) noexcept = default;
//         ErasedTransition (ErasedTransition &&) noexcept = default;
//         ErasedTransition &operator= (ErasedTransition &&) noexcept = default;

//         Delay runActions (Ev const &ev) override { return internal.runActions (ev); }
//         void resetActions () override { internal.resetActions (); }
//         bool checkCondition (Ev const &ev) const override { return internal.checkCondition (ev); }
//         std::type_index getStateIndex () const override { return internal.getStateIndex (); }
//         const char *getStateName () const override { return internal.getStateName (); }
//         size_t getStateIdx () const override { return stateIdx; }

//         Tr internal;
//         size_t stateIdx{};
// };

// template <typename Ev, typename Tr> auto erasedTransition (Tr tr) { return ErasedTransition<Ev, Tr> (std::move (tr)); }

// template <typename Ev, typename Tt> auto erasedTransitions (Tt &&tt)
// {
//         // There's probably easier way of doing that in hana.
//         return boost::hana::unpack (tt, [] (auto... transition) { return boost::hana::make_tuple (erasedTransition<Ev> (transition)...); });
// }

/****************************************************************************/

template <typename T> struct is_transition : public std::false_type {
};

template <typename In> struct is_transition<ErasedTransitionBase<In>> : public std::true_type {
};

template <typename T> constexpr bool is_transition_v = is_transition<T>::value;

} // namespace ls