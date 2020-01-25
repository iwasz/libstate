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

template <typename Sn, typename Con, typename TacT> auto transition (Con &&condition, TacT &&actions)
{
        return Transition<Sn, Con, TacT> (std::forward<Con> (condition), std::forward<TacT> (actions));
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

template <typename Sn, typename EntT, typename TraT, typename ExiT> auto state (EntT &&en, TraT &&tra, ExiT &&ex)
{
        return State<Sn, EntT, TraT, ExiT> (std::forward<EntT> (en), std::forward<TraT> (tra), std::forward<ExiT> (ex));
}

/****************************************************************************/

template <typename StaT> struct Machine {

        Machine (StaT states) : states{std::move (states)} {}

        template <typename Ev> void run (Ev const &ev);

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
        return [&fun, &transition] (auto const &ev) {
                if (transition.condition (ev)) {
                        fun (transition);
                        return true;
                }

                return false;
        };
}
} // namespace impl

template <typename Ev, typename TraT, typename Fun> void forMatchingTransition (Ev const &ev, TraT &transitions, Fun &&function)
{
        std::apply ([&ev, &function] (
                            auto &... transition) { (impl::runIfMatchingTransition (transition, std::forward<Fun> (function)) (ev) || ...); },
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

int main ()
{
        // int results{};
        std::vector<std::string> results;

        // auto res = [&results] (int message) { return [&results, message] (auto) { results += message; }; };
        // auto res = [&results] (auto const &message) { return [&results, message] (auto) { results; }; };
        auto res
                = [&results] (std::string const &message) { return [&results, message] (std::string const &) { results.push_back (message); }; };
        auto eq = [] (std::string const &what) { return [what] (std::string const &i) -> bool { return i == what; }; };

        auto m = Machine (std::make_tuple (
                state<Name<1>> (std::make_tuple (res ("INIT entry")),
                                std::make_tuple (transition<Name<2>> (eq ("2"), std::make_tuple (res ("65"), res ("66")))),
                                std::make_tuple (res ("INIT exit")))
#define FULL 1
#if FULL
                        ,
                state<Name<2>> (std::make_tuple (res ("B entry")),
                                std::make_tuple (transition<Name<1>> (eq ("-3"), std::make_tuple (res ("action"), res ("another"))),
                                                 transition<Name<3>> (eq ("3"), std::make_tuple (res ("action"), res ("another")))),
                                std::make_tuple (res ("B exit"))),

                state<Name<3>> (std::make_tuple (res ("C entry")),
                                std::make_tuple (transition<Name<2>> (eq ("-4"), std::make_tuple (res ("action"), res ("another"))),
                                                 transition<Name<4>> (eq ("4"), std::make_tuple (res ("action"), res ("another")))),
                                std::make_tuple (res ("C exit"))),

                state<Name<4>> (std::make_tuple (res ("D entry")),
                                std::make_tuple (transition<Name<3>> (eq ("-5"), std::make_tuple (res ("action"), res ("another"))),
                                                 transition<Name<5>> (eq ("5"), std::make_tuple (res ("action"), res ("another")))),
                                std::make_tuple (res ("D exit"))),

                state<Name<5>> (std::make_tuple (res ("E entry")),
                                std::make_tuple (transition<Name<4>> (eq ("-6"), std::make_tuple (res ("action"), res ("another"))),
                                                 transition<Name<6>> (eq ("6"), std::make_tuple (res ("action"), res ("another")))),
                                std::make_tuple (res ("E exit"))),

                state<Name<6>> (std::make_tuple (res ("F entry")),
                                std::make_tuple (transition<Name<5>> (eq ("-7"), std::make_tuple (res ("action"), res ("another"))),
                                                 transition<Name<7>> (eq ("7"), std::make_tuple (res ("action"), res ("another")))),
                                std::make_tuple (res ("F exit"))),

                state<Name<7>> (std::make_tuple (res ("G entry")),
                                std::make_tuple (transition<Name<6>> (eq ("-8"), std::make_tuple (res ("action"), res ("another"))),
                                                 transition<Name<8>> (eq ("8"), std::make_tuple (res ("action"), res ("another")))),
                                std::make_tuple (res ("G exit"))),

                state<Name<8>> (std::make_tuple (res ("H entry")),
                                std::make_tuple (transition<Name<7>> (eq ("-9"), std::make_tuple (res ("action"), res ("another"))),
                                                 transition<Name<9>> (eq ("9"), std::make_tuple (res ("action"), res ("another")))),
                                std::make_tuple (res ("H exit"))),

                state<Name<9>> (std::make_tuple (res ("I entry")),
                                std::make_tuple (transition<Name<8>> (eq ("-10"), std::make_tuple (res ("action"), res ("another"))),
                                                 transition<Name<10>> (eq ("10"), std::make_tuple (res ("action"), res ("another")))),
                                std::make_tuple (res ("I exit"))),

                state<Name<10>> (std::make_tuple (res ("J entry")),
                                 std::make_tuple (transition<Name<9>> (eq ("-11"), std::make_tuple (res ("action"), res ("another"))),
                                                  transition<Name<11>> (eq ("11"), std::make_tuple (res ("action"), res ("another")))),
                                 std::make_tuple (res ("J exit"))),

                state<Name<11>> (std::make_tuple (res ("K entry")),
                                 std::make_tuple (transition<Name<10>> (eq ("-12"), std::make_tuple (res ("action"), res ("another"))),
                                                  transition<Name<12>> (eq ("12"), std::make_tuple (res ("action"), res ("another")))),
                                 std::make_tuple (res ("K exit"))),

                state<Name<12>> (std::make_tuple (res ("L entry")),
                                 std::make_tuple (transition<Name<11>> (eq ("-13"), std::make_tuple (res ("action"), res ("another"))),
                                                  transition<Name<13>> (eq ("13"), std::make_tuple (res ("action"), res ("another")))),
                                 std::make_tuple (res ("L exit"))),

                state<Name<13>> (std::make_tuple (res ("M entry")),
                                 std::make_tuple (transition<Name<12>> (eq ("-14"), std::make_tuple (res ("action"), res ("another"))),
                                                  transition<Name<14>> (eq ("14"), std::make_tuple (res ("action"), res ("another")))),
                                 std::make_tuple (res ("M exit"))),

                state<Name<14>> (std::make_tuple (res ("N entry")),
                                 std::make_tuple (transition<Name<13>> (eq ("-15"), std::make_tuple (res ("action"), res ("another"))),
                                                  transition<Name<15>> (eq ("15"), std::make_tuple (res ("action"), res ("another")))),
                                 std::make_tuple (res ("N exit"))),

                state<Name<15>> (std::make_tuple (res ("O entry")),
                                 std::make_tuple (transition<Name<14>> (eq ("-16"), std::make_tuple (res ("action"), res ("another"))),
                                                  transition<Name<16>> (eq ("16"), std::make_tuple (res ("action"), res ("another")))),
                                 std::make_tuple (res ("O exit"))),

                state<Name<16>> (std::make_tuple (res ("FINAL entry")),
                                 std::make_tuple (transition<Name<15>> (eq ("-17"), std::make_tuple (res ("action"), res ("another"))),
                                                  transition<Name<16>> (eq ("17"), std::make_tuple (res ("action"), res ("another")))),
                                 std::make_tuple (res ("")))
#endif
                        ));

        m.run (std::string ("1")); // TODO already in init state
        assert (m.currentStateIndex == 1);

        for (int i = 0; i < 10000; ++i) {
                m.run ("2"); // 1->2
                m.run ("3"); // B->C
                m.run ("4");
                m.run ("5");
                m.run ("6");
                m.run ("7");
                m.run ("8");
                m.run ("9");
                m.run ("10");
                m.run ("11");
                m.run ("12");
                m.run ("13");
                m.run ("14");
                m.run ("15");

                m.run ("-16");
                m.run ("-15");
                m.run ("-14");
                m.run ("-13");
                m.run ("-12");
                m.run ("-11");
                m.run ("-10");
                m.run ("-9");
                m.run ("-8");
                m.run ("-7");
                m.run ("-6");
                m.run ("-5");
                m.run ("-4");
                m.run ("-3"); // B->INIT
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
}

/*
    FILE SIZE        VM SIZE
 --------------  --------------
  42.9%   100Ki  83.9%   100Ki    .text
  39.5%  92.5Ki   0.0%       0    .strtab
   4.8%  11.2Ki   0.0%       0    .symtab
   3.6%  8.52Ki   0.0%       0    [Unmapped]
   3.5%  8.13Ki   6.7%  8.07Ki    .eh_frame
   2.4%  5.71Ki   4.7%  5.65Ki    .gcc_except_table
   0.7%  1.57Ki   1.3%  1.50Ki    .eh_frame_hdr
   0.3%     685   0.6%     685    [LOAD #2 [R]]
   0.2%     592   0.4%     528    .dynamic
   0.2%     573   0.4%     509    .dynstr
   0.2%     568   0.4%     504    .dynsym
   0.2%     441   0.3%     377    .rodata
   0.2%     400   0.3%     336    .rela.plt
   0.1%     354   0.0%       0    .shstrtab
   0.1%     352   0.2%     288    .rela.dyn
   0.1%     304   0.2%     240    .plt
   0.1%     224   0.1%     160    .gnu.version_r
   0.1%     200   0.1%     136    .got.plt
   0.1%     128   0.0%       0    [ELF Headers]
   0.0%     106   0.0%      42    .gnu.version
   0.0%     104   0.0%      40    .got
   0.0%     100   0.0%      36    .note.gnu.build-id
   0.0%      96   0.0%      32    .note.ABI-tag
   0.0%      92   0.0%      28    .gnu.hash
   0.0%      92   0.0%      28    .interp
   0.0%      91   0.0%      27    .init
   0.0%      88   0.0%      24    .data.rel.ro
   0.0%      81   0.0%       0    .comment
   0.0%      80   0.0%      16    .data
   0.0%      77   0.0%      13    .fini
   0.0%      72   0.0%       8    .fini_array
   0.0%      72   0.0%       8    .init_array
   0.0%       0   0.0%       8    .bss
   0.0%       8   0.0%       8    [LOAD #3 [RX]]
   0.0%       3   0.0%       3    [LOAD #4 [R]]
 100.0%   233Ki 100.0%   119Ki    TOTAL


    FILE SIZE        VM SIZE
 --------------  --------------
  64.3%   297Ki  65.7%   297Ki    .text
  24.1%   111Ki  24.6%   111Ki    .eh_frame
   5.7%  26.5Ki   5.8%  26.4Ki    .eh_frame_hdr
   2.4%  11.2Ki   2.5%  11.2Ki    .gcc_except_table
   1.7%  7.85Ki   0.0%       0    [Unmapped]
   0.3%  1.56Ki   0.3%  1.50Ki    .dynstr
   0.2%     976   0.2%     912    .dynsym
   0.2%     784   0.2%     720    .rela.plt
   0.1%     688   0.1%     688    [LOAD #2 [R]]
   0.1%     592   0.1%     528    .dynamic
   0.1%     560   0.1%     496    .plt
   0.1%     554   0.1%     490    .rodata
   0.1%     328   0.1%     264    .got.plt
   0.1%     325   0.0%       0    .shstrtab
   0.1%     280   0.0%     216    .rela.dyn
   0.0%     208   0.0%     144    .gnu.version_r
   0.0%     140   0.0%      76    .gnu.version
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
   0.0%       2   0.0%       2    [LOAD #4 [R]]
 100.0%   462Ki 100.0%   452Ki    TOTAL

    FILE SIZE        VM SIZE
 --------------  --------------
  76.8%  9.98Mi   0.0%       0    .debug_str
  12.9%  1.68Mi   0.0%       0    .strtab
   4.4%   579Ki   0.0%       0    .debug_info
   2.2%   297Ki  65.7%   297Ki    .text
   1.0%   138Ki   0.0%       0    .debug_line
   0.8%   111Ki  24.6%   111Ki    .eh_frame
   0.6%  85.5Ki   0.0%       0    .symtab
   0.4%  52.8Ki   0.0%       0    .debug_aranges
   0.4%  52.8Ki   0.0%       0    .debug_ranges
   0.2%  26.5Ki   5.8%  26.4Ki    .eh_frame_hdr
   0.1%  11.2Ki   2.5%  11.2Ki    .gcc_except_table
   0.1%  7.85Ki   0.0%       0    [Unmapped]
   0.0%  4.31Ki   0.0%       0    .debug_abbrev
   0.0%  1.56Ki   0.3%  1.50Ki    .dynstr
   0.0%     976   0.2%     912    .dynsym
   0.0%     784   0.2%     720    .rela.plt
   0.0%     688   0.1%     688    [LOAD #2 [R]]
   0.0%     592   0.1%     528    .dynamic
   0.0%     560   0.1%     496    .plt
   0.0%     554   0.1%     490    .rodata
   0.0%     419   0.0%       0    .shstrtab
   0.0%     328   0.1%     264    .got.plt
   0.0%     280   0.0%     216    .rela.dyn
   0.0%     208   0.0%     144    .gnu.version_r
   0.0%     140   0.0%      76    .gnu.version
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
   0.0%       2   0.0%       2    [LOAD #4 [R]]
 100.0%  13.0Mi 100.0%   452Ki    TOTAL
*/
