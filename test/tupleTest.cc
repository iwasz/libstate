/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include <exception>
#include <tuple>
#include <utility>
#include <vector>

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
        // Hardcoded currentState to 1

        forCurrentState ([ev, machine = this] (auto &state) {
                // TODO For all events {}

                forMatchingTransition (ev, state.transitions, [&ev, machine] (auto &transition) {
#ifndef NDEBUG
                // std::cout << "Transition to : " << trans->getStateName () << std::endl;
#endif
                        machine->forCurrentState ([&ev] (auto &state) { state.runEntry (ev); });
                        transition.runTransitionActions (ev);
                        machine->currentStateIndex = std::remove_reference_t<decltype (transition)>::Name::getIndex ();
                        machine->forCurrentState ([&ev] (auto &state) { state.runExit (ev); });
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

        auto res = [&results] (int message) { return [&results, message] (auto) { results += message; }; };
        auto eq = [] (int what) { return [what] (auto const &i) { return i == what; }; };

        auto machine = Machine (std::make_tuple (
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

        std::apply ([] (auto &... state) { (state.runEntry (1), ...); }, machine.states);
        std::apply ([] (auto &... state) { (state.runExit (2), ...); }, machine.states);

        machine.run (1);

        if (results != 300) {
                std::terminate ();
        }
}

/*
    FILE SIZE        VM SIZE
 --------------  --------------
  69.9%  67.4Ki   0.0%       0    .strtab
  16.2%  15.6Ki  76.3%  15.6Ki    .text
   3.8%  3.64Ki   0.0%       0    [Unmapped]
   3.1%  3.02Ki   0.0%       0    .symtab
   2.7%  2.59Ki  12.4%  2.52Ki    .eh_frame
   0.7%     689   3.3%     689    [LOAD #2 [R]]
   0.6%     592   2.5%     528    .dynamic
   0.3%     315   0.0%       0    .shstrtab
   0.3%     283   1.0%     219    .dynstr
   0.3%     256   0.9%     192    .dynsym
   0.3%     256   0.9%     192    .rela.dyn
   0.1%     144   0.4%      80    .gnu.version_r
   0.1%     140   0.4%      76    .eh_frame_hdr
   0.1%     128   0.0%       0    [ELF Headers]
   0.1%     112   0.2%      48    .plt
   0.1%     112   0.2%      48    .rela.plt
   0.1%     104   0.2%      40    .got
   0.1%     104   0.2%      40    .got.plt
   0.1%     100   0.2%      36    .note.gnu.build-id
   0.1%      96   0.2%      32    .note.ABI-tag
   0.1%      92   0.1%      28    .gnu.hash
   0.1%      92   0.1%      28    .interp
   0.1%      91   0.1%      27    .init
   0.1%      81   0.0%       0    .comment
   0.1%      80   0.1%      16    .gnu.version
   0.1%      77   0.1%      13    .fini
   0.1%      72   0.0%       8    .data
   0.1%      72   0.0%       8    .fini_array
   0.1%      72   0.0%       8    .init_array
   0.0%       0   0.0%       8    .bss
   0.0%       8   0.0%       8    [LOAD #3 [RX]]
   0.0%       4   0.0%       4    [LOAD #4 [R]]
 100.0%  96.3Ki 100.0%  20.4Ki    TOTAL


    FILE SIZE        VM SIZE
 --------------  --------------
  80.6%  23.7Mi   0.0%       0    .debug_str
  12.9%  3.79Mi   0.0%       0    .strtab
   2.8%   839Ki   0.0%       0    .debug_info
   1.5%   445Ki  69.2%   445Ki    .text
   0.7%   195Ki   0.0%       0    .debug_line
   0.5%   155Ki  24.2%   155Ki    .eh_frame
   0.4%   117Ki   0.0%       0    .symtab
   0.3%  75.8Ki   0.0%       0    .debug_aranges
   0.3%  75.8Ki   0.0%       0    .debug_ranges
   0.1%  38.0Ki   5.9%  37.9Ki    .eh_frame_hdr
   0.0%  8.48Ki   0.0%       0    [Unmapped]
   0.0%  3.21Ki   0.0%       0    .debug_abbrev
   0.0%  1.78Ki   0.3%  1.71Ki    .gcc_except_table
   0.0%     686   0.1%     686    [LOAD #2 [R]]
   0.0%     592   0.1%     528    .dynamic
   0.0%     411   0.0%       0    .shstrtab
   0.0%     338   0.0%     274    .dynstr
   0.0%     304   0.0%     240    .dynsym
   0.0%     280   0.0%     216    .rela.dyn
   0.0%     192   0.0%     128    .gnu.version_r
   0.0%     136   0.0%      72    .rela.plt
   0.0%     128   0.0%      64    .plt
   0.0%     128   0.0%       0    [ELF Headers]
   0.0%     112   0.0%      48    .got.plt
   0.0%     104   0.0%      40    .got
   0.0%     100   0.0%      36    .note.gnu.build-id
   0.0%      96   0.0%      32    .note.ABI-tag
   0.0%      92   0.0%      28    .gnu.hash
   0.0%      92   0.0%      28    .interp
   0.0%      91   0.0%      27    .init
   0.0%      84   0.0%      20    .gnu.version
   0.0%      81   0.0%       0    .comment
   0.0%      80   0.0%      16    .data
   0.0%      77   0.0%      13    .fini
   0.0%      72   0.0%       8    .fini_array
   0.0%      72   0.0%       8    .init_array
   0.0%       0   0.0%       8    .bss
   0.0%       8   0.0%       8    [LOAD #3 [RX]]
   0.0%       4   0.0%       4    [LOAD #4 [R]]
 100.0%  29.4Mi 100.0%   643Ki    TOTAL




 STRIPPED
    FILE SIZE        VM SIZE
 --------------  --------------
  68.2%   445Ki  69.2%   445Ki    .text
  23.8%   155Ki  24.2%   155Ki    .eh_frame
   5.8%  38.0Ki   5.9%  37.9Ki    .eh_frame_hdr
   1.3%  8.48Ki   0.0%       0    [Unmapped]
   0.3%  1.78Ki   0.3%  1.71Ki    .gcc_except_table
   0.1%     686   0.1%     686    [LOAD #2 [R]]
   0.1%     592   0.1%     528    .dynamic
   0.1%     338   0.0%     274    .dynstr
   0.0%     317   0.0%       0    .shstrtab
   0.0%     304   0.0%     240    .dynsym
   0.0%     280   0.0%     216    .rela.dyn
   0.0%     192   0.0%     128    .gnu.version_r
   0.0%     136   0.0%      72    .rela.plt
   0.0%     128   0.0%      64    .plt
   0.0%     128   0.0%       0    [ELF Headers]
   0.0%     112   0.0%      48    .got.plt
   0.0%     104   0.0%      40    .got
   0.0%     100   0.0%      36    .note.gnu.build-id
   0.0%      96   0.0%      32    .note.ABI-tag
   0.0%      92   0.0%      28    .gnu.hash
   0.0%      92   0.0%      28    .interp
   0.0%      91   0.0%      27    .init
   0.0%      84   0.0%      20    .gnu.version
   0.0%      81   0.0%       0    .comment
   0.0%      80   0.0%      16    .data
   0.0%      77   0.0%      13    .fini
   0.0%      72   0.0%       8    .fini_array
   0.0%      72   0.0%       8    .init_array
   0.0%       0   0.0%       8    .bss
   0.0%       8   0.0%       8    [LOAD #3 [RX]]
   0.0%       4   0.0%       4    [LOAD #4 [R]]
 100.0%   654Ki 100.0%   643Ki    TOTAL
*/
