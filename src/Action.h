// /****************************************************************************
//  *                                                                          *
//  *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
//  *  ~~~~~~~~                                                                *
//  *  License : see COPYING file for details.                                 *
//  *  ~~~~~~~~~                                                               *
//  ****************************************************************************/

// #pragma once
// #include <chrono>
// #include <gsl/gsl>
// #include <optional>
// #include <tuple>
// #include <type_traits>
// #include <typeindex>

// namespace ls {

// /// Common return type for simplicity.
// using Delay = std::chrono::nanoseconds;

// static const auto DELAY_ZERO = Delay::zero ();

// template <typename> struct IsDuration : public std::false_type {
// };

// template <typename Rep, typename Period> struct IsDuration<std::chrono::duration<Rep, Period>> : public std::true_type {
// };

// /**
//  * It's a wrapper for an action which runs the wrapped type only unless
//  * it is reset.
//  */
// template <typename T> class ActionRunner {
// public:
//         explicit ActionRunner (T t) : action (std::move (t)) {}

//         /**
//          *
//          */
//         template <typename... Arg> Delay operator() (Arg &&... a)
//         {
//                 if (!active) {
//                         return DELAY_ZERO;
//                 }

//                 active = false;

//                 if constexpr (std::is_invocable_v<T, Arg...>) { // Action accepts arguments

//                         if constexpr (IsDuration<std::invoke_result_t<T, Arg...>>::value) {
//                                 return std::chrono::duration_cast<Delay> (action (std::forward<Arg> (a)...));
//                         }
//                         else {
//                                 static_assert (std::is_same_v<std::invoke_result_t<T, Arg...>, void>,
//                                                "Wrong action return value. Use either std::chrono::duration <R, P> or void.");
//                                 action (std::forward<Arg> (a)...);
//                                 return DELAY_ZERO;
//                         }
//                 }
//                 else { // Action does not accept arguments
//                         static_assert (
//                                 std::is_invocable_v<T>,
//                                 "Wrong action argument type(s). Action argument has to be compatible with the event type or there should be no
//                                 " "arguments at all.");

//                         if constexpr (IsDuration<std::invoke_result_t<T>>::value) { // But it returns Done.
//                                 return std::chrono::duration_cast<Delay> (action ());
//                         }
//                         else {
//                                 static_assert (std::is_same_v<std::invoke_result_t<T>, void>,
//                                                "Wrong action return value. Use either std::chrono::duration <R, P> or void.");
//                                 action (); // Doesn't either accept an arg or return.
//                                 return DELAY_ZERO;
//                         }
//                 }
//         }

//         void reset () { active = true; }
//         bool isActive () const { return active; }

// private:
//         T action;
//         bool active{true};
// };

// /****************************************************************************/

// template <typename Ev, typename R, typename... Rr> Delay processActionRunners (Ev const &event, R &runner, Rr &... rest)
// {
//         if (Delay d = runner (event); d != DELAY_ZERO) {
//                 return d;
//         }

//         if constexpr (sizeof...(rest) > 0) {
//                 return processActionRunners (event, rest...);
//         }

//         return {};
// }

// /**
//  * Tuple of actions.
//  */
// template <typename T> class ActionTuple {
// public:
//         explicit ActionTuple (T a) : actions{std::move (a)} {}

//         template <typename Ev> Delay operator() (Ev const &event)
//         {
//                 return std::apply (
//                         [&event, this] (auto &... args) {
//                                 if constexpr (sizeof...(args) > 0) {
//                                         return processActionRunners (event, args...);
//                                 }

//                                 reset ();
//                                 return Delay{};
//                         },
//                         actions);
//         }

//         void reset ()
//         {
//                 std::apply ([] (auto &... args) { (args.reset (), ...); }, actions);
//         }

// private:
//         T actions;
// };

// /**
//  * This is for creating "default"
//  */
// template <> class ActionTuple<void> {
// public:
//         ActionTuple () = default;
//         template <typename Ev> Delay operator() (Ev const & /*event*/) { return Delay{}; }
//         void reset () {}
// };

// template <typename T> struct Entry : public ActionTuple<T> {
//         using ActionTuple<T>::ActionTuple;
// };

// template <typename T> struct Exit : public ActionTuple<T> {
//         using ActionTuple<T>::ActionTuple;
// };

// template <typename T> struct TransitionAction : public ActionTuple<T> {
//         using ActionTuple<T>::ActionTuple;
// };

// template <template <typename T> class Tu, typename... Ar> auto actionTuple (Ar &&... args)
// {
//         auto tuple = std::make_tuple (ActionRunner (std::forward<Ar> (args))...);
//         return Tu<decltype (tuple)> (std::move (tuple));
// }

// template <typename... Ar> auto entry (Ar &&... args) { return actionTuple<Entry, Ar...> (std::forward<Ar> (args)...); }
// template <typename... Ar> auto exit (Ar &&... args) { return actionTuple<Exit, Ar...> (std::forward<Ar> (args)...); }
// template <typename... Ar> auto transitionAction (Ar &&... args) { return actionTuple<TransitionAction, Ar...> (std::forward<Ar> (args)...); }

// /****************************************************************************/

// template <typename T> struct is_entry : public std::false_type {
// };

// template <typename In> struct is_entry<Entry<In>> : public std::true_type {
// };

// template <typename T> constexpr bool is_entry_v = is_entry<T>::value;

// template <typename T> struct is_exit : public std::false_type {
// };

// template <typename In> struct is_exit<Exit<In>> : public std::true_type {
// };

// template <typename T> constexpr bool is_exit_v = is_exit<T>::value;

// } // namespace ls
