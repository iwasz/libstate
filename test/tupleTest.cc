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

template <size_t index> struct Name {
        const char *getName () const { return "TODO"; }
        static constexpr size_t getIndex () { return index; }
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
template <typename Sta, typename Fun> auto runIfCurrent (int current, Fun &&fun)
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
                        (impl::runIfCurrent<std::remove_reference_t<decltype (state)>> (currentStateIndex, function) (state) || ...);
                },
                states);
}

template <typename TraT, typename Fun> void forMatchingTransition (TraT &transitions, Fun &&function) {}

template </* typename Ev, TODO problem z auto*/ typename StaT> void Machine<StaT>::run (int ev)
{
        // Hardcoded currentState to 1

        forCurrentState ([ev] (auto &state) {
                // For all events {}
                // For all transitions
                forMatchingTransition (state.transitions, [&ev] (auto &transition) {
#ifndef NDEBUG
                        std::cout << "Transition to : " << trans->getStateName () << std::endl;
#endif
                        // forCurrentState ([&ev] (auto &state) { state.runEntry (ev); });
                        // transition.runTransitionActions (ev);
                        // currentStateIndex = decltype(transition)::Name::getIndex  ();
                        // forCurrentState ([&ev] (auto &state) { state.runExit (ev); });
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
#define FULL 0
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
58.2%  9.30Ki   0.0%       0    [Unmapped]
9.2%  1.47Ki   0.0%       0    .symtab
4.2%     686  24.2%     686    [LOAD #2 [R]]
3.6%     592  18.7%     528    .dynamic
3.5%     574   0.0%       0    .strtab
2.8%     453  13.8%     389    .text
1.9%     315   0.0%       0    .shstrtab
1.7%     272   7.4%     208    .eh_frame
1.6%     256   6.8%     192    .dynstr
1.6%     256   6.8%     192    .rela.dyn
1.4%     232   5.9%     168    .dynsym
0.8%     128   2.3%      64    .gnu.version_r
0.8%     128   0.0%       0    [ELF Headers]
0.7%     116   1.8%      52    .eh_frame_hdr
0.6%     104   1.4%      40    .got
0.6%     100   1.3%      36    .note.gnu.build-id
0.6%      96   1.1%      32    .got.plt
0.6%      96   1.1%      32    .note.ABI-tag
0.6%      96   1.1%      32    .plt
0.6%      92   1.0%      28    .gnu.hash
0.6%      92   1.0%      28    .interp
0.6%      91   1.0%      27    .init
0.5%      88   0.8%      24    .rela.plt
0.5%      81   0.0%       0    .comment
0.5%      78   0.5%      14    .gnu.version
0.5%      77   0.5%      13    .fini
0.4%      72   0.3%       8    .data
0.4%      72   0.3%       8    .fini_array
0.4%      72   0.3%       8    .init_array
0.0%       0   0.3%       8    .bss
0.0%       8   0.3%       8    [LOAD #3 [RX]]
0.0%       4   0.1%       4    [LOAD #4 [R]]
100.0%  16.0Ki 100.0%  2.76Ki    TOTAL
*/
