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

#define DEBUG 0
#if DEBUG
#include <iostream>
#endif

template <typename Ev, typename Sn, typename Con, typename TacT> struct Transition {

        using Name = Sn;

        Transition (Con condition, TacT transitionActions) : condition{std::move (condition)}, transitionActions{std::move (transitionActions)}
        {
        }

        void runTransitionActions (Ev const &ev);

        Con condition;
        TacT transitionActions;
};

template <typename Ev, typename Sn, typename Con, typename TacT> void Transition<Ev, Sn, Con, TacT>::runTransitionActions (Ev const &ev)
{
        std::apply ([&ev] (auto &... transitionAction) { (transitionAction (ev), ...); }, transitionActions);
}

template <typename Ev, typename Sn, typename Con, typename TacT> auto transition (Con &&condition, TacT &&actions)
{
        return Transition<Ev, Sn, Con, TacT> (std::forward<Con> (condition), std::forward<TacT> (actions));
}

/****************************************************************************/

template <int index> struct Name {
        const char *getName () const { return "TODO"; }
        static constexpr int getIndex () { return index; }
};

template <typename Ev, typename Sn, typename EntT, typename TraT, typename ExiT> struct State {

        using Name = Sn;

        State (EntT en, TraT tr, ExiT ex) : entryActions{std::move (en)}, transitions{std::move (tr)}, exitActions{std::move (ex)} {}

        void runEntry (Ev const &ev);
        void runExit (Ev const &ev);

        EntT entryActions;
        TraT transitions;
        ExiT exitActions;
};

template <typename Ev, typename Sn, typename EntT, typename TraT, typename ExiT> void State<Ev, Sn, EntT, TraT, ExiT>::runEntry (Ev const &ev)
{
        std::apply ([&ev] (auto &... entryAction) { (entryAction (ev), ...); }, entryActions);
}

template <typename Ev, typename Sn, typename EntT, typename TraT, typename ExiT> void State<Ev, Sn, EntT, TraT, ExiT>::runExit (Ev const &ev)
{
        std::apply ([&ev] (auto &... exitAction) { (exitAction (ev), ...); }, exitActions);
}

template <typename Ev, typename Sn, typename EntT, typename TraT, typename ExiT> auto state (EntT &&en, TraT &&tra, ExiT &&ex)
{
        return State<Ev, Sn, EntT, TraT, ExiT> (std::forward<EntT> (en), std::forward<TraT> (tra), std::forward<ExiT> (ex));
}

/****************************************************************************/

template </* typename Ev, TODO problem z auto*/ typename StaT> struct Machine {

        Machine (StaT states) : states{std::move (states)} {}

        void run (int Ev);

        /// Run functionFunction on currentState.
        template <typename Fun> void forCurrentState (Fun &&function);

        StaT states;
        int currentStateIndex{1};
};

namespace impl {
template <typename Sta, typename Fun> auto runIfCurrentState (int current, Fun &&fun)
{
        return [&fun, current] (auto &state) {
                if (Sta::Name::getIndex () == current) {
                        fun (state);
                        return true;
                }

                return false;
        };
}
} // namespace impl

template <typename StaT> template <typename Fun> void Machine<StaT>::forCurrentState (Fun &&function)
{
        std::apply (
                [&function, this] (auto &... state) {
                        (impl::runIfCurrentState<std::remove_reference_t<decltype (state)>> (currentStateIndex, function) (state) || ...);
                },
                states);
}

namespace impl {
template <typename Tra, typename Fun> auto runIfMatchingTransition (Tra &transition, Fun &&fun)
{
        return [&fun, &transition] (int ev) {
                if (transition.condition (ev)) {
                        fun (transition);
                        return true;
                }

                return false;
        };
}
} // namespace impl

template <typename TraT, typename Fun> void forMatchingTransition (int ev, TraT &transitions, Fun &&function)
{

        std::apply ([&ev, &function] (
                            auto &... transition) { (impl::runIfMatchingTransition (transition, std::forward<Fun> (function)) (ev) || ...); },
                    transitions);
}

template </* typename Ev, TODO problem z auto*/ typename StaT> void Machine<StaT>::run (int ev)
{
        // TODO timer.
        // TODO Currently hardcoded currentState to 1

        forCurrentState ([ev, machine = this] (auto &state) {
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

int main ()
{
        int results{};

        // auto res = [&results] (int message) { return [&results, message] (auto) { results += message; }; };
        auto res = [&results] (int message) { return [&results, message] (auto) { ++results; }; };
        auto eq = [] (int what) { return [what] (auto const &i) { return i == what; }; };

        auto m = Machine (std::make_tuple (
                state<int, Name<1>> (std::make_tuple (res (1), res (2)),
                                     std::make_tuple (transition<int, Name<1>> (eq (-2), std::make_tuple (res (65), res (66))),
                                                      transition<int, Name<2>> (eq (2), std::make_tuple (res (65), res (66)))),
                                     std::make_tuple (res (3), res (4)))
#define FULL 1
#if FULL
                        ,
                state<int, Name<2>> (std::make_tuple (res (5), res (6)),
                                     std::make_tuple (transition<int, Name<1>> (eq (-3), std::make_tuple (res (65), res (66))),
                                                      transition<int, Name<3>> (eq (3), std::make_tuple (res (65), res (66)))),
                                     std::make_tuple (res (7), res (8))),

                state<int, Name<3>> (std::make_tuple (res (9), res (10)),
                                     std::make_tuple (transition<int, Name<2>> (eq (-4), std::make_tuple (res (65), res (66))),
                                                      transition<int, Name<4>> (eq (4), std::make_tuple (res (65), res (66)))),
                                     std::make_tuple (res (11), res (12))),

                state<int, Name<4>> (std::make_tuple (res (13), res (14)),
                                     std::make_tuple (transition<int, Name<3>> (eq (-5), std::make_tuple (res (65), res (66))),
                                                      transition<int, Name<5>> (eq (5), std::make_tuple (res (65), res (66)))),
                                     std::make_tuple (res (15), res (16))),

                state<int, Name<5>> (std::make_tuple (res (17), res (18)),
                                     std::make_tuple (transition<int, Name<4>> (eq (-6), std::make_tuple (res (65), res (66))),
                                                      transition<int, Name<6>> (eq (6), std::make_tuple (res (65), res (66)))),
                                     std::make_tuple (res (19), res (20))),

                state<int, Name<6>> (std::make_tuple (res (21), res (22)),
                                     std::make_tuple (transition<int, Name<5>> (eq (-7), std::make_tuple (res (65), res (66))),
                                                      transition<int, Name<7>> (eq (7), std::make_tuple (res (65), res (66)))),
                                     std::make_tuple (res (23), res (24))),

                state<int, Name<7>> (std::make_tuple (res (25), res (26)),
                                     std::make_tuple (transition<int, Name<6>> (eq (-8), std::make_tuple (res (65), res (66))),
                                                      transition<int, Name<8>> (eq (8), std::make_tuple (res (65), res (66)))),
                                     std::make_tuple (res (27), res (28))),

                state<int, Name<8>> (std::make_tuple (res (29), res (30)),
                                     std::make_tuple (transition<int, Name<7>> (eq (-9), std::make_tuple (res (65), res (66))),
                                                      transition<int, Name<9>> (eq (9), std::make_tuple (res (65), res (66)))),
                                     std::make_tuple (res (31), res (32))),

                state<int, Name<9>> (std::make_tuple (res (33), res (34)),
                                     std::make_tuple (transition<int, Name<8>> (eq (-10), std::make_tuple (res (65), res (66))),
                                                      transition<int, Name<10>> (eq (10), std::make_tuple (res (65), res (66)))),
                                     std::make_tuple (res (35), res (36))),

                state<int, Name<10>> (std::make_tuple (res (37), res (38)),
                                      std::make_tuple (transition<int, Name<9>> (eq (-11), std::make_tuple (res (65), res (66))),
                                                       transition<int, Name<11>> (eq (11), std::make_tuple (res (65), res (66)))),
                                      std::make_tuple (res (39), res (40))),

                state<int, Name<11>> (std::make_tuple (res (41), res (42)),
                                      std::make_tuple (transition<int, Name<10>> (eq (-12), std::make_tuple (res (65), res (66))),
                                                       transition<int, Name<12>> (eq (12), std::make_tuple (res (65), res (66)))),
                                      std::make_tuple (res (43), res (44))),

                state<int, Name<12>> (std::make_tuple (res (45), res (46)),
                                      std::make_tuple (transition<int, Name<11>> (eq (-13), std::make_tuple (res (65), res (66))),
                                                       transition<int, Name<13>> (eq (13), std::make_tuple (res (65), res (66)))),
                                      std::make_tuple (res (47), res (48))),

                state<int, Name<13>> (std::make_tuple (res (49), res (50)),
                                      std::make_tuple (transition<int, Name<12>> (eq (-14), std::make_tuple (res (65), res (66))),
                                                       transition<int, Name<14>> (eq (14), std::make_tuple (res (65), res (66)))),
                                      std::make_tuple (res (51), res (52))),

                state<int, Name<14>> (std::make_tuple (res (53), res (54)),
                                      std::make_tuple (transition<int, Name<13>> (eq (-15), std::make_tuple (res (65), res (66))),
                                                       transition<int, Name<15>> (eq (15), std::make_tuple (res (65), res (66)))),
                                      std::make_tuple (res (55), res (56))),

                state<int, Name<15>> (std::make_tuple (res (57), res (58)),
                                      std::make_tuple (transition<int, Name<14>> (eq (-16), std::make_tuple (res (65), res (66))),
                                                       transition<int, Name<16>> (eq (16), std::make_tuple (res (65), res (66)))),
                                      std::make_tuple (res (59), res (60))),

                state<int, Name<16>> (std::make_tuple (res (61), res (62)),
                                      std::make_tuple (transition<int, Name<15>> (eq (-17), std::make_tuple (res (65), res (66))),
                                                       transition<int, Name<16>> (eq (17), std::make_tuple (res (65), res (66)))),
                                      std::make_tuple (res (63), res (64)))
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
}

/*
  35.2%  7.43Ki  74.4%  7.36Ki    .text
  29.2%  6.16Ki   0.0%       0    [Unmapped]
   7.7%  1.63Ki   0.0%       0    .strtab
   7.2%  1.52Ki   0.0%       0    .symtab
   3.2%     689   6.8%     689    [LOAD #2 [R]]
   2.7%     592   5.2%     528    .dynamic
   1.5%     315   0.0%       0    .shstrtab
   1.4%     304   2.4%     240    .eh_frame
   1.3%     283   2.2%     219    .dynstr
   1.2%     256   1.9%     192    .dynsym
   1.2%     256   1.9%     192    .rela.dyn
   0.7%     144   0.8%      80    .gnu.version_r
   0.6%     128   0.0%       0    [ELF Headers]
   0.6%     124   0.6%      60    .eh_frame_hdr
   0.5%     112   0.5%      48    .plt
   0.5%     112   0.5%      48    .rela.plt
   0.5%     104   0.4%      40    .got
   0.5%     104   0.4%      40    .got.plt
   0.5%     100   0.4%      36    .note.gnu.build-id
   0.4%      96   0.3%      32    .note.ABI-tag
   0.4%      92   0.3%      28    .gnu.hash
   0.4%      92   0.3%      28    .interp
   0.4%      91   0.3%      27    .init
   0.4%      81   0.0%       0    .comment
   0.4%      80   0.2%      16    .gnu.version
   0.4%      77   0.1%      13    .fini
   0.3%      72   0.1%       8    .data
   0.3%      72   0.1%       8    .fini_array
   0.3%      72   0.1%       8    .init_array
   0.0%       0   0.1%       8    .bss
   0.0%       8   0.1%       8    [LOAD #3 [RX]]
   0.0%       4   0.0%       4    [LOAD #4 [R]]
 100.0%  21.1Ki 100.0%  9.90Ki    TOTAL

    FILE SIZE        VM SIZE
 --------------  --------------
  58.9%  93.1Ki  62.2%  93.1Ki    .text
  27.2%  42.9Ki  28.7%  42.9Ki    .eh_frame
   6.6%  10.4Ki   6.9%  10.3Ki    .eh_frame_hdr
   4.1%  6.51Ki   0.0%       0    [Unmapped]
   0.5%     796   0.5%     732    .gcc_except_table
   0.4%     686   0.4%     686    [LOAD #2 [R]]
   0.4%     592   0.3%     528    .dynamic
   0.2%     352   0.2%     288    .dynstr
   0.2%     328   0.2%     264    .dynsym
   0.2%     325   0.0%       0    .shstrtab
   0.2%     280   0.1%     216    .rela.dyn
   0.1%     192   0.1%     128    .gnu.version_r
   0.1%     160   0.1%      96    .rela.plt
   0.1%     144   0.1%      80    .plt
   0.1%     128   0.0%       0    [ELF Headers]
   0.1%     121   0.0%      57    .rodata
   0.1%     120   0.0%      56    .got.plt
   0.1%     104   0.0%      40    .got
   0.1%     100   0.0%      36    .note.gnu.build-id
   0.1%      96   0.0%      32    .note.ABI-tag
   0.1%      92   0.0%      28    .gnu.hash
   0.1%      92   0.0%      28    .interp
   0.1%      91   0.0%      27    .init
   0.1%      86   0.0%      22    .gnu.version
   0.1%      81   0.0%       0    .comment
   0.0%      80   0.0%      16    .data
   0.0%      77   0.0%      13    .fini
   0.0%      72   0.0%       8    .fini_array
   0.0%      72   0.0%       8    .init_array
   0.0%       0   0.0%       8    .bss
   0.0%       8   0.0%       8    [LOAD #3 [RX]]
   0.0%       3   0.0%       3    [LOAD #4 [R]]
 100.0%   158Ki 100.0%   149Ki    TOTAL

    FILE SIZE        VM SIZE
 --------------  --------------
  75.4%  3.15Mi   0.0%       0    .debug_str
  11.8%   502Ki   0.0%       0    .strtab
   6.1%   262Ki   0.0%       0    .debug_info
   2.2%  93.1Ki  62.2%  93.1Ki    .text
   1.1%  48.5Ki   0.0%       0    .debug_line
   1.0%  42.9Ki  28.7%  42.9Ki    .eh_frame
   0.8%  34.9Ki   0.0%       0    .symtab
   0.5%  20.6Ki   0.0%       0    .debug_aranges
   0.5%  20.6Ki   0.0%       0    .debug_ranges
   0.2%  10.4Ki   6.9%  10.3Ki    .eh_frame_hdr
   0.2%  6.51Ki   0.0%       0    [Unmapped]
   0.1%  3.59Ki   0.0%       0    .debug_abbrev
   0.0%     796   0.5%     732    .gcc_except_table
   0.0%     686   0.4%     686    [LOAD #2 [R]]
   0.0%     592   0.3%     528    .dynamic
   0.0%     419   0.0%       0    .shstrtab
   0.0%     352   0.2%     288    .dynstr
   0.0%     328   0.2%     264    .dynsym
   0.0%     280   0.1%     216    .rela.dyn
   0.0%     192   0.1%     128    .gnu.version_r
   0.0%     160   0.1%      96    .rela.plt
   0.0%     144   0.1%      80    .plt
   0.0%     128   0.0%       0    [ELF Headers]
   0.0%     121   0.0%      57    .rodata
   0.0%     120   0.0%      56    .got.plt
   0.0%     104   0.0%      40    .got
   0.0%     100   0.0%      36    .note.gnu.build-id
   0.0%      96   0.0%      32    .note.ABI-tag
   0.0%      92   0.0%      28    .gnu.hash
   0.0%      92   0.0%      28    .interp
   0.0%      91   0.0%      27    .init
   0.0%      86   0.0%      22    .gnu.version
   0.0%      81   0.0%       0    .comment
   0.0%      80   0.0%      16    .data
   0.0%      77   0.0%      13    .fini
   0.0%      72   0.0%       8    .fini_array
   0.0%      72   0.0%       8    .init_array
   0.0%       0   0.0%       8    .bss
   0.0%       8   0.0%       8    [LOAD #3 [RX]]
   0.0%       3   0.0%       3    [LOAD #4 [R]]
 100.0%  4.17Mi 100.0%   149Ki    TOTAL

*/
