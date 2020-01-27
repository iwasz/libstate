/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include <etl/deque.h>
#include <exception>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#define DEBUG 1
#if DEBUG
#include <iostream>
#endif

template <typename Sn, typename Con, typename TacT> struct Transition {

        using Name = Sn;

        Transition (Con condition, TacT transitionActions) : condition{std::move (condition)}, transitionActions{std::move (transitionActions)}
        {
        }

        template <typename Ev> void runTransitionActions (Ev const &ev);

        Con condition;
        TacT transitionActions;
};

template <typename Sn, typename Con, typename TacT> template <typename Ev> void Transition<Sn, Con, TacT>::runTransitionActions (Ev const &ev)
{
        std::apply ([&ev] (auto &... transitionAction) { (transitionAction (ev), ...); }, transitionActions);
}

// template <typename Sn, typename Con, typename TacT> auto transition (Con &&condition, TacT &&actions)
// {
//         return Transition<Sn, Con, TacT> (std::forward<Con> (condition), std::forward<TacT> (actions));
// }

template <typename Sn, typename Con, typename... Tac> auto transition (Con &&condition, Tac &&... actions)
{
        return Transition<Sn, Con, decltype (std::forward_as_tuple (actions...))> (std::forward<Con> (condition),
                                                                                   std::forward_as_tuple (actions...));
}

/****************************************************************************/

template <int index> struct Name {
        const char *getName () const { return "TODO"; }
        static constexpr int getIndex () { return index; }
};

template <typename Sn, typename EntT, typename TraT, typename ExiT> struct State {

        using Name = Sn;

        State (EntT en, TraT tr, ExiT ex) : entryActions{std::move (en)}, transitions{std::move (tr)}, exitActions{std::move (ex)} {}

        template <typename Ev> void runEntry (Ev const &ev);

        template <typename Ev> void runExit (Ev const &ev);

        EntT entryActions;
        TraT transitions;
        ExiT exitActions;
};

template <typename Sn, typename EntT, typename TraT, typename ExiT>
template <typename Ev>
void State<Sn, EntT, TraT, ExiT>::runEntry (Ev const &ev)
{
        std::apply ([&ev] (auto &... entryAction) { (entryAction (ev), ...); }, entryActions);
}

template <typename Sn, typename EntT, typename TraT, typename ExiT>
template <typename Ev>
void State<Sn, EntT, TraT, ExiT>::runExit (Ev const &ev)
{
        std::apply ([&ev] (auto &... exitAction) { (exitAction (ev), ...); }, exitActions);
}

// template <typename Sn, typename EntT, typename TraT, typename ExiT> auto state (EntT &&en, TraT &&tra, ExiT &&ex)
// {
//         return State<Sn, EntT, TraT, ExiT> (std::forward<EntT> (en), std::forward<TraT> (tra), std::forward<ExiT> (ex));
// }

template <typename Sn, typename EntT, typename... Tra, typename ExiT> auto state (EntT &&en, ExiT &&ex, Tra &&... tra)
{
        return State<Sn, EntT, decltype (std::forward_as_tuple (tra...)), ExiT> (std::forward<EntT> (en), std::forward_as_tuple (tra...),
                                                                                 std::forward<ExiT> (ex));
}

template <typename... Act> constexpr auto entry (Act &&... act) { return std::forward_as_tuple (act...); }
template <typename... Act> constexpr auto exit (Act &&... act) { return std::forward_as_tuple (act...); }

/****************************************************************************/

template <typename StaT> struct Machine {

        Machine (StaT states) : states{std::move (states)} {}

        template <typename Ev> void run (Ev const &ev);

        /// Run functionFunction on currentState.
        template <typename Fun> void forCurrentState (Fun &&function);

        StaT states;
        int currentStateIndex{1};
};

template <typename... Sta> constexpr auto machine (Sta &&... states) { return Machine (std::forward_as_tuple (states...)); }

namespace impl {
template <typename Fun, typename Sta, typename... Rst> void runIfCurrentState2 (int current, Fun &&fun, Sta &state, Rst &... rest)
{
        if (Sta::Name::getIndex () == current) {
                fun (state);
                return;
        }

        if constexpr (sizeof...(rest) > 0) {
                runIfCurrentState2 (current, std::forward<Fun> (fun), rest...);
        }
}

} // namespace impl

template <typename StaT> template <typename Fun> void Machine<StaT>::forCurrentState (Fun &&function)
{
        std::apply (
                [&function, this] (auto &... state) { impl::runIfCurrentState2 (currentStateIndex, std::forward<Fun> (function), state...); },
                states);
}

namespace impl {
template <typename Fun, typename Ev, typename Tra, typename... Rst>
void runIfMatchingTransition2 (Fun &&fun, Ev const &ev, Tra &transition, Rst &... rest)
{
        if (transition.condition (ev)) {
                fun (transition);
                return;
        }

        if constexpr (sizeof...(rest)) {
                runIfMatchingTransition2 (std::forward<Fun> (fun), ev, rest...);
        }
}
} // namespace impl

template <typename Ev, typename TraT, typename Fun> void forMatchingTransition (Ev const &ev, TraT &transitions, Fun &&function)
{
        std::apply (
                [&ev, &function] (auto &... transition) { impl::runIfMatchingTransition2 (std::forward<Fun> (function), ev, transition...); },
                transitions);
}

template <typename StaT> template <typename Ev> void Machine<StaT>::run (Ev const &ev)
{
        // TODO timer.
        // TODO Currently hardcoded currentState to 1

        forCurrentState ([&ev, machine = this] (auto &state) {
                // TODO For all events {}

                // If not run
                state.runEntry (ev);

                forMatchingTransition (ev, state.transitions, [&ev, machine, &state] (auto &transition) {
#ifndef NDEBUG
                // std::cout << "Transition to : " << trans->getStateName () << std::endl;
#endif
                        // machine->forCurrentState ([&ev] (auto &state) { state.runExit (ev); });
                        state.runExit (ev);
                        transition.runTransitionActions (ev);
                        machine->currentStateIndex = std::remove_reference_t<decltype (transition)>::Name::getIndex ();

                        // ???? Can I replace this with state.runEntry at the beginnig?>?
                        // machine->forCurrentState ([&ev] (auto &state) { state.runEntry (ev); });
                        // eventQueue.clear
                });
        });

#if 0
        if (!timer.isExpired ()) {
                return;
        }

#ifndef NDEBUG
        std::cout << "== Run ==" << std::endl;
#endif

        if (!currentState) {
                currentState = findInitialState ();
        }

        for (ErasedTransitionBase<Ev> *trans = currentState->getTransition (); trans != nullptr; trans = trans->next) {
                for (auto const &event : eventQueue) {
                        if (!trans->checkCondition (event)) {
                                continue;
                        }
#ifndef NDEBUG
                        std::cout << "Transition to : " << trans->getStateName () << std::endl;
#endif
                        if (Delay d = currentState->runExitActions (event); d != DELAY_ZERO) {
                                std::cerr << "Delay requested : " << std::chrono::duration_cast<std::chrono::milliseconds> (d).count () << "ms"
                                          << std::endl;

                                timer.start (std::chrono::duration_cast<std::chrono::milliseconds> (d).count ());
                                goto end;
                        }

                        if (Delay d = trans->runActions (event); d != DELAY_ZERO) {
                                std::cerr << "Delay requested : " << std::chrono::duration_cast<std::chrono::milliseconds> (d).count () << "ms"
                                          << std::endl;

                                timer.start (std::chrono::duration_cast<std::chrono::milliseconds> (d).count ());
                                goto end;
                        }

                        prevState = currentState;
                        currentState = states.at (trans->getStateIndex ());
                        Ensures (currentState);

                        currentState->runEntryActions (event);
                        eventQueue.clear (); // TODO not quite like that

                        if (prevState) {
                                prevState->resetExitActions ();
                        }

                        trans->resetActions ();
                        currentState->resetEntryActions ();
                }
        }

end:;
#endif
}

// template <typename Ev, typename StaT> auto machine (StaT &&s) -> Machine<Ev, StaT> { return Machine<Ev, StaT> (std::forward<StaT> (s)); }

/****************************************************************************/

// struct res {
//         res (std::string s) : message{std::move (s)} {}
//         void operator() (std::string const &) { results.push_back (message); }

// private:
//         std::string message;
// };

int main ()
{
#if 1
        std::vector<std::string> results;
        using namespace std::string_literals;

        // int results{};

        auto res = [&results] (const char *message) {
                return [&results, message] (auto const &ev) {
                        results.emplace_back (message);
                        // results.push_back (ev);
                };
        };

        auto eq = [] (std::string const &what) { return [what] (auto const &i) -> bool { return i == what; }; };

        auto m = machine (
                state<Name<1>> (entry (res ("INIT entry")), exit (res ("INIT exit")), transition<Name<2>> (eq ("2"), res ("65"), res ("66")))
#define FULL 1
#if FULL

                        ,
                state<Name<2>> (entry (res ("B entry")), exit (res ("B exit")), transition<Name<1>> (eq ("-3"), res ("action"), res ("another")),
                                transition<Name<3>> (eq ("3"), res ("action"), res ("another"))),

                state<Name<3>> (entry (res ("C entry")), exit (res ("C exit")), transition<Name<2>> (eq ("-4"), res ("action"), res ("another")),
                                transition<Name<4>> (eq ("4"), res ("action"), res ("another"))),

                state<Name<4>> (entry (res ("D entry")), exit (res ("D exit")), transition<Name<3>> (eq ("-5"), res ("action"), res ("another")),
                                transition<Name<5>> (eq ("5"), res ("action"), res ("another"))),

                state<Name<5>> (entry (res ("E entry")), exit (res ("E exit")), transition<Name<4>> (eq ("-6"), res ("action"), res ("another")),
                                transition<Name<6>> (eq ("6"), res ("action"), res ("another"))),

                state<Name<6>> (entry (res ("F entry")), exit (res ("F exit")), transition<Name<5>> (eq ("-7"), res ("action"), res ("another")),
                                transition<Name<7>> (eq ("7"), res ("action"), res ("another"))),

                state<Name<7>> (entry (res ("G entry")), exit (res ("G exit")), transition<Name<6>> (eq ("-8"), res ("action"), res ("another")),
                                transition<Name<8>> (eq ("8"), res ("action"), res ("another"))),

                state<Name<8>> (entry (res ("H entry")), exit (res ("H exit")), transition<Name<7>> (eq ("-9"), res ("action"), res ("another")),
                                transition<Name<9>> (eq ("9"), res ("action"), res ("another"))),

                state<Name<9>> (entry (res ("I entry")), exit (res ("I exit")),
                                transition<Name<8>> (eq ("-10"), res ("action"), res ("another")),
                                transition<Name<10>> (eq ("10"), res ("action"), res ("another"))),

                state<Name<10>> (entry (res ("J entry")), exit (res ("J exit")),
                                 transition<Name<9>> (eq ("-11"), res ("action"), res ("another")),
                                 transition<Name<11>> (eq ("11"), res ("action"), res ("another"))),

                state<Name<11>> (entry (res ("K entry")), exit (res ("K exit")),
                                 transition<Name<10>> (eq ("-12"), res ("action"), res ("another")),
                                 transition<Name<12>> (eq ("12"), res ("action"), res ("another"))),

                state<Name<12>> (entry (res ("L entry")), exit (res ("L exit")),
                                 transition<Name<11>> (eq ("-13"), res ("action"), res ("another")),
                                 transition<Name<13>> (eq ("13"), res ("action"), res ("another"))),

                state<Name<13>> (entry (res ("M entry")), exit (res ("M exit")),
                                 transition<Name<12>> (eq ("-14"), res ("action"), res ("another")),
                                 transition<Name<14>> (eq ("14"), res ("action"), res ("another"))),

                state<Name<14>> (entry (res ("N entry")), exit (res ("N exit")),
                                 transition<Name<13>> (eq ("-15"), res ("action"), res ("another")),
                                 transition<Name<15>> (eq ("15"), res ("action"), res ("another"))),

                state<Name<15>> (entry (res ("O entry")), exit (res ("O exit")),
                                 transition<Name<14>> (eq ("-16"), res ("action"), res ("another")),
                                 transition<Name<16>> (eq ("16"), res ("action"), res ("another"))),

                state<Name<16>> (entry (res ("FINAL entry")), exit (res ("")),
                                 transition<Name<15>> (eq ("-17"), res ("action"), res ("another")),
                                 transition<Name<16>> (eq ("17"), res ("action"), res ("another")))
#endif
        );

        m.run ("1"s); // TODO already in init state
        assert (m.currentStateIndex == 1);

        for (int i = 0; i < 10000; ++i) {
                m.run ("2"s); // 1->2
                m.run ("3"s); // B->C
                m.run ("4"s);
                m.run ("5"s);
                m.run ("6"s);
                m.run ("7"s);
                m.run ("8"s);
                m.run ("9"s);
                m.run ("10"s);
                m.run ("11"s);
                m.run ("12"s);
                m.run ("13"s);
                m.run ("14"s);
                m.run ("15"s);

                m.run ("-16"s);
                m.run ("-15"s);
                m.run ("-14"s);
                m.run ("-13"s);
                m.run ("-12"s);
                m.run ("-11"s);
                m.run ("-10"s);
                m.run ("-9"s);
                m.run ("-8"s);
                m.run ("-7"s);
                m.run ("-6"s);
                m.run ("-5"s);
                m.run ("-4"s);
                m.run ("-3"s); // B->INIT
        }

        // Ensures (results.size () == 1110001); This true when entering INIT state triggers entry actions when FSM is run for the first time
        // Ensures (results.size () == 1110000);
#if DEBUG
        std::cout << results.size () << std::endl;
#endif

        // TODO Check if this number is really correct
        if (results.size () != 1120001) {
                std::terminate ();
        }
#endif

#if 0
        int results{};

        // auto res = [&results] (int message) { return [&results, message] (auto) { results += message; }; };
        auto res = [&results] (int message) { return [&results, message] (auto) { ++results; }; };
        auto eq = [] (int what) { return [what] (auto const &i) { return i == what; }; };

        auto m = Machine (
                state<Name<1>> (res (1), res (2),
                                                 transition<Name<1>> (eq (-2), res (65), res (66)),
                                                                  transition<Name<2>> (eq (2), res (65), res (66))),
                                                 res (3), res (4))
#define FULL 1
#if FULL
                                         ,
                                 state<Name<2>> (res (5), res (6),
                                                 transition<Name<1>> (eq (-3), res (65), res (66)),
                                                                  transition<Name<3>> (eq (3), res (65), res (66))),
                                                 res (7), res (8)),

                                 state<Name<3>> (res (9), res (10),
                                                 transition<Name<2>> (eq (-4), res (65), res (66)),
                                                                  transition<Name<4>> (eq (4), res (65), res (66))),
                                                 res (11), res (12)),

                                 state<Name<4>> (res (13), res (14),
                                                 transition<Name<3>> (eq (-5), res (65), res (66)),
                                                                  transition<Name<5>> (eq (5), res (65), res (66))),
                                                 res (15), res (16)),

                                 state<Name<5>> (res (17), res (18),
                                                 transition<Name<4>> (eq (-6), res (65), res (66)),
                                                                  transition<Name<6>> (eq (6), res (65), res (66))),
                                                 res (19), res (20)),

                                 state<Name<6>> (res (21), res (22),
                                                 transition<Name<5>> (eq (-7), res (65), res (66)),
                                                                  transition<Name<7>> (eq (7), res (65), res (66))),
                                                 res (23), res (24)),

                                 state<Name<7>> (res (25), res (26),
                                                 transition<Name<6>> (eq (-8), res (65), res (66)),
                                                                  transition<Name<8>> (eq (8), res (65), res (66))),
                                                 res (27), res (28)),

                                 state<Name<8>> (res (29), res (30),
                                                 transition<Name<7>> (eq (-9), res (65), res (66)),
                                                                  transition<Name<9>> (eq (9), res (65), res (66))),
                                                 res (31), res (32)),

                                 state<Name<9>> (res (33), res (34),
                                                 transition<Name<8>> (eq (-10), res (65), res (66)),
                                                                  transition<Name<10>> (eq (10), res (65), res (66))),
                                                 res (35), res (36)),

                                 state<Name<10>> (res (37), res (38),
                                                  transition<Name<9>> (eq (-11), res (65), res (66)),
                                                                   transition<Name<11>> (eq (11), res (65), res (66))),
                                                  res (39), res (40)),

                                 state<Name<11>> (res (41), res (42),
                                                  transition<Name<10>> (eq (-12), res (65), res (66)),
                                                                   transition<Name<12>> (eq (12), res (65), res (66))),
                                                  res (43), res (44)),

                                 state<Name<12>> (res (45), res (46),
                                                  transition<Name<11>> (eq (-13), res (65), res (66)),
                                                                   transition<Name<13>> (eq (13), res (65), res (66))),
                                                  res (47), res (48)),

                                 state<Name<13>> (res (49), res (50),
                                                  transition<Name<12>> (eq (-14), res (65), res (66)),
                                                                   transition<Name<14>> (eq (14), res (65), res (66))),
                                                  res (51), res (52)),

                                 state<Name<14>> (res (53), res (54),
                                                  transition<Name<13>> (eq (-15), res (65), res (66)),
                                                                   transition<Name<15>> (eq (15), res (65), res (66))),
                                                  res (55), res (56)),

                                 state<Name<15>> (res (57), res (58),
                                                  transition<Name<14>> (eq (-16), res (65), res (66)),
                                                                   transition<Name<16>> (eq (16), res (65), res (66))),
                                                  res (59), res (60)),

                                 state<Name<16>> (res (61), res (62),
                                                  transition<Name<15>> (eq (-17), res (65), res (66)),
                                                                   transition<Name<16>> (eq (17), res (65), res (66))),
                                                  res (63), res (64))
#endif
                                         ));

        m.run (1); // TODO already in init state
        assert (m.currentStateIndex == 1);

        for (int i = 0; i < 10000; ++i) {
                m.run (2); // 1->2
                m.run (3); // B->C
                m.run (4);
                m.run (5);
                m.run (6);
                m.run (7);
                m.run (8);
                m.run (9);
                m.run (10);
                m.run (11);
                m.run (12);
                m.run (13);
                m.run (14);
                m.run (15);

                m.run (-16);
                m.run (-15);
                m.run (-14);
                m.run (-13);
                m.run (-12);
                m.run (-11);
                m.run (-10);
                m.run (-9);
                m.run (-8);
                m.run (-7);
                m.run (-6);
                m.run (-5);
                m.run (-4);
                m.run (-3); // B->INIT
        }

        // Ensures (results.size () == 1110001); This true when entering INIT state triggers entry actions when FSM is run for the first time
        // Ensures (results.size () == 1110000);
#if DEBUG
        std::cout << results << std::endl;
#endif

        // TODO Check if this number is really correct
        if (results != 1680002) {
                std::terminate ();
        }
#endif
}

/*
-O3
    FILE SIZE        VM SIZE
 --------------  --------------
  76.9%  75.5Ki  80.5%  75.4Ki    .text
   7.7%  7.60Ki   8.0%  7.54Ki    .eh_frame
   5.6%  5.46Ki   5.8%  5.40Ki    .gcc_except_table
   2.5%  2.45Ki   0.0%       0    [Unmapped]
   1.5%  1.50Ki   1.5%  1.43Ki    .eh_frame_hdr
   0.7%     685   0.7%     685    [LOAD #2 [R]]
   0.6%     592   0.6%     528    .dynamic
   0.6%     573   0.5%     509    .dynstr
   0.6%     568   0.5%     504    .dynsym
   0.4%     441   0.4%     377    .rodata
   0.4%     400   0.4%     336    .rela.plt
   0.3%     325   0.0%       0    .shstrtab
   0.3%     304   0.3%     240    .plt
   0.3%     280   0.2%     216    .rela.dyn
   0.2%     224   0.2%     160    .gnu.version_r
   0.2%     200   0.1%     136    .got.plt
   0.1%     128   0.0%       0    [ELF Headers]
   0.1%     106   0.0%      42    .gnu.version
   0.1%     104   0.0%      40    .got
   0.1%     100   0.0%      36    .note.gnu.build-id
   0.1%      96   0.0%      32    .note.ABI-tag
   0.1%      92   0.0%      28    .gnu.hash
   0.1%      92   0.0%      28    .interp
   0.1%      91   0.0%      27    .init
   0.1%      81   0.0%       0    .comment
   0.1%      80   0.0%      16    .data
   0.1%      77   0.0%      13    .fini
   0.1%      72   0.0%       8    .fini_array
   0.1%      72   0.0%       8    .init_array
   0.0%       0   0.0%       8    .bss
   0.0%       8   0.0%       8    [LOAD #3 [RX]]
   0.0%       3   0.0%       3    [LOAD #4 [R]]
 100.0%  98.2Ki 100.0%  93.7Ki    TOTAL

-O0 stripped
    FILE SIZE        VM SIZE
 --------------  --------------
  61.0%   130Ki  63.5%   130Ki    .text
  23.6%  50.5Ki  24.5%  50.4Ki    .eh_frame
   5.7%  12.1Ki   5.9%  12.1Ki    .eh_frame_hdr
   3.0%  6.50Ki   3.1%  6.43Ki    .gcc_except_table
   3.0%  6.35Ki   0.0%       0    [Unmapped]
   0.8%  1.63Ki   0.8%  1.56Ki    .dynstr
   0.5%    1000   0.4%     936    .dynsym
   0.4%     808   0.4%     744    .rela.plt
   0.3%     684   0.3%     684    [LOAD #2 [R]]
   0.3%     592   0.3%     528    .dynamic
   0.3%     576   0.2%     512    .plt
   0.3%     556   0.2%     492    .rodata
   0.2%     336   0.1%     272    .got.plt
   0.1%     325   0.0%       0    .shstrtab
   0.1%     280   0.1%     216    .rela.dyn
   0.1%     208   0.1%     144    .gnu.version_r
   0.1%     142   0.0%      78    .gnu.version
   0.1%     128   0.0%       0    [ELF Headers]
   0.0%     104   0.0%      40    .got
   0.0%     100   0.0%      36    .gnu.hash
   0.0%     100   0.0%      36    .note.gnu.build-id
   0.0%      96   0.0%      32    .note.ABI-tag
   0.0%      92   0.0%      28    .interp
   0.0%      91   0.0%      27    .init
   0.0%      81   0.0%       0    .comment
   0.0%      80   0.0%      16    .data
   0.0%      77   0.0%      13    .fini
   0.0%      72   0.0%       8    .fini_array
   0.0%      72   0.0%       8    .init_array
   0.0%       0   0.0%       8    .bss
   0.0%       8   0.0%       8    [LOAD #3 [RX]]
 100.0%   214Ki 100.0%   205Ki    TOTAL


   FILE SIZE        VM SIZE
 --------------  --------------
  72.5%  3.34Mi   0.0%       0    .debug_str
  13.4%   632Ki   0.0%       0    .strtab
   6.3%   298Ki   0.0%       0    .debug_info
   2.8%   130Ki  63.5%   130Ki    .text
   1.3%  59.4Ki   0.0%       0    .debug_line
   1.1%  50.5Ki  24.5%  50.4Ki    .eh_frame
   0.9%  42.5Ki   0.0%       0    .symtab
   0.5%  24.2Ki   0.0%       0    .debug_ranges
   0.5%  24.1Ki   0.0%       0    .debug_aranges
   0.3%  12.1Ki   5.9%  12.1Ki    .eh_frame_hdr
   0.1%  6.50Ki   3.1%  6.43Ki    .gcc_except_table
   0.1%  6.36Ki   0.0%       0    [Unmapped]
   0.1%  4.34Ki   0.0%       0    .debug_abbrev
   0.0%  1.63Ki   0.8%  1.56Ki    .dynstr
   0.0%    1000   0.4%     936    .dynsym
   0.0%     808   0.4%     744    .rela.plt
   0.0%     684   0.3%     684    [LOAD #2 [R]]
   0.0%     592   0.3%     528    .dynamic
   0.0%     576   0.2%     512    .plt
   0.0%     556   0.2%     492    .rodata
   0.0%     419   0.0%       0    .shstrtab
   0.0%     336   0.1%     272    .got.plt
   0.0%     280   0.1%     216    .rela.dyn
   0.0%     208   0.1%     144    .gnu.version_r
   0.0%     142   0.0%      78    .gnu.version
   0.0%     128   0.0%       0    [ELF Headers]
   0.0%     104   0.0%      40    .got
   0.0%     100   0.0%      36    .gnu.hash
   0.0%     100   0.0%      36    .note.gnu.build-id
   0.0%      96   0.0%      32    .note.ABI-tag
   0.0%      92   0.0%      28    .interp
   0.0%      91   0.0%      27    .init
   0.0%      81   0.0%       0    .comment
   0.0%      80   0.0%      16    .data
   0.0%      77   0.0%      13    .fini
   0.0%      72   0.0%       8    .fini_array
   0.0%      72   0.0%       8    .init_array
   0.0%       0   0.0%       8    .bss
   0.0%       8   0.0%       8    [LOAD #3 [RX]]
 100.0%  4.61Mi 100.0%   205Ki    TOTAL

*/
