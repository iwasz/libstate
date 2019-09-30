#pragma once
#include <boost/hana.hpp>
#include <gsl/gsl>
// TODO remove
#include <string>
// TODO remove
#include <iostream>

// TODO remove thise statements
namespace hana = boost::hana;
using namespace hana::literals;
using namespace std::string_literals;

/// Do zwracania z akcji.
enum class Done { NO, YES };

// machine = Machine {State {RESET_STATE, Entry {UsartAction {u, UsartAction::INTERRUPT_ON}}, Exit {}, Transition {PIN_STATUS_STATE, beginsWith
// ("RDY")}},
//                   State {}};

/**
 * It's a wrapper for an action which can run it conditionally. It works like that : if 'active' flag is true, the 'call'
 * method will run the 'action'. Then, if the return value from that 'action' equals Done::YES, 'active' is flipped to
 * false, and next time the action will not be run.
 */
template <typename T> class ActionRunner {
public:
        explicit ActionRunner (T t) : action (std::move (t)) {}

        template <typename Ret, typename... Arg> Ret call (Arg &&... a)
        {
                if (!active) {
                        return {};
                }
                Done done = action (std::forward<Arg...> (a)...);

                if (done == Done::YES) {
                        active = false;
                }
        }

        void reset () noexcept { active = true; }
        bool isActive () const noexcept { return active; }
        void setActive (bool b) noexcept { active = b; }

private:
        bool active = true;
        T action;
};

/**
 * Tuple of actions.
 */
template <typename T> class ActionTuple {
public:
        template <typename Ev> void invoke (Ev const &event)
        {
                hana::for_each (actions, [&event](auto &f) { f (event); });
        }

        // protected:
        explicit ActionTuple (T &&a) : actions{std::move (a)} {}

private:
        T actions;
};

template <> class ActionTuple<void> {
public:
        ActionTuple () = default;
        template <typename Ev> void invoke (Ev const &event) {}
};

template <typename T> struct Entry : public ActionTuple<T> {
        using ActionTuple<T>::ActionTuple;
};

template <typename T> struct Exit : public ActionTuple<T> {
        using ActionTuple<T>::ActionTuple;
};

/// Transition action.
template <typename T> struct Action : public ActionTuple<T> {
        using ActionTuple<T>::ActionTuple;
};

template <typename... Ar> auto actionTuple (Ar &&... args)
{
        auto tuple = hana::make_tuple (std::forward<Ar> (args)...);
        auto at = ActionTuple (std::move (tuple));
        return at;
}

template <template <typename T> class Tu, typename... Ar> auto actionTuple2 (Ar &&... args)
{
        auto tuple = hana::make_tuple (std::forward<Ar> (args)...);
        auto at = Tu<decltype (tuple)> (std::move (tuple));
        return at;
}

template <typename... Ar> auto entry (Ar &&... args) { return actionTuple2<Entry, Ar...> (std::forward<Ar> (args)...); }
template <typename... Ar> auto exit (Ar &&... args) { return actionTuple2<Exit, Ar...> (std::forward<Ar> (args)...); }
template <typename... Ar> auto action (Ar &&... args) { return actionTuple2<Action, Ar...> (std::forward<Ar> (args)...); }

template </*typename Tu,*/ typename... Ar> struct Actions {

        auto operator() (Ar &&... args)
        {
                auto tuple = hana::make_tuple (std::forward<Ar> (args)...);
                auto at = /*Tu*/ ActionTuple (std::move (tuple));
                return at;
        }
};

// template <typename... Ar> auto entry (Ar &&... args) { return actionTuple<Entry, Ar> (std::forward<Ar> (args)...); }

// template <typename... Ar> struct Actions {

//        explicit Actions (Ar &&... args)
//        {
//                auto tuple = hana::make_tuple (std::forward<Ar> (args)...);
//                auto at = ActionTuple (std::move (tuple));
//        }
//};

template <typename T1 = void, typename T2 = void> struct State {
        State () = default;
        explicit State (Entry<T1> en) : entry (std::move (en)) {}
        State (Entry<T1> en, Exit<T2> ex) : entry (std::move (en)), exit (std::move (ex)) {}

private:
        Entry<T1> entry;
        Exit<T2> exit;
};

struct Transition {
};

class At {
public:
        At (gsl::czstring<> c) : cmd (c) {}
        void operator() (auto /*event*/) { std::cout << "usart <- " << cmd << std::endl; }

private:
        gsl::czstring<> cmd;
};
