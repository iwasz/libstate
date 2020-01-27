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

#include "Misc.h"

#define DEBUG 0
#if DEBUG
#include <iostream>
#endif

using namespace ls;

/**
 * Timeouts
 * Queue (?)
 */

template <unsigned int index> struct Name {
        const char *getName () const { return "TODO"; }
        static constexpr auto getIndex () { return index; }
};

/****************************************************************************/

/// CRC32 implementation by GitHub user oktal. https://gist.github.com/oktal/5573082
constexpr unsigned int crc32_table[]
        = {0,          0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E,
           0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB,
           0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8,
           0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
           0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599,
           0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106,
           0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB,
           0x086D3D2D, 0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
           0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 0x4DB26158, 0x3AB551CE, 0xA3BC0074,
           0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5,
           0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E,
           0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
           0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27,
           0x7D079EB1, 0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0,
           0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1,
           0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
           0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92,
           0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D, 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
           0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4,
           0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
           0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D,
           0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9, 0xBDBDF21C, 0xCABAC28A,
           0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37,
           0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D};

template <unsigned int CRC, char... Chars> struct Crc32Impl {
};

template <unsigned int CRC, char Head, char... Tail> struct Crc32Impl<CRC, Head, Tail...> {
        static constexpr unsigned int value
                = Crc32Impl<crc32_table[static_cast<unsigned char> (CRC) ^ static_cast<unsigned char> (Head)] ^ (CRC >> 8), Tail...>::value;
};

template <unsigned int CRC> struct Crc32Impl<CRC> {
        static constexpr unsigned int value = CRC ^ 0xFFFFFFFF;
};

template <char... Chars> using Crc32 = Crc32Impl<0xFFFFFFFF, Chars...>;

// constexpr unsigned int crc32_rec (unsigned int crc, const char *s)
// {
//         return *s == 0 ? crc ^ 0xFFFFFFFF
//                        : crc32_rec (crc32_table[static_cast<unsigned char> (crc) ^ static_cast<unsigned char> (*s)] ^ (crc >> 8), s + 1);
// }

// constexpr unsigned int operator"" _crc32 (const char *s, size_t len) { return crc32_rec (0xFFFFFFFF, s); }

template <char... s> struct Name2 {
        static constexpr char const *c_str () { return "TODO"; }
        static constexpr unsigned int getIndex () { return Crc32<s...>::value; }
};

template <char... s> constexpr Name2<s...> Name2_c{};

// TODO This is a GNU extension. Provide macro as an option. Or better still, use hana::string explicitly
template <typename C, C... c> constexpr auto operator""_STATE () { return Name2_c<c...>; }

/****************************************************************************/

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
        return Transition<Sn, Con, decltype (std::make_tuple (actions...))> (std::forward<Con> (condition), std::make_tuple (actions...));
}

template <typename Sn, typename Con, typename... Tac> auto transition (Sn, Con &&condition, Tac &&... actions)
{
        return Transition<Sn, Con, decltype (std::make_tuple (actions...))> (std::forward<Con> (condition), std::make_tuple (actions...));
}

/****************************************************************************/

template <typename Sn, typename EntT, typename TraT, typename ExiT> struct State {
        using Name = Sn;

        State (EntT en, TraT tr, ExiT ex) : entryActions{std::move (en)}, transitions{std::move (tr)}, exitActions{std::move (ex)} {}

        EntT entryActions;
        TraT transitions;
        ExiT exitActions;
};

template <typename Ev, typename... Acts> inline void runActions (Ev const &ev, std::tuple<Acts...> &actions)
{
        std::apply ([&ev] (auto &... action) { (action (ev), ...); }, actions);
}

template <typename Ev, typename Act> inline void runActions (Ev const &ev, Act &action) { action (ev); }

template <typename Ev, typename Act> void runSomeActions (Ev const &ev, Act &actions) { runActions (ev, actions); }

/*--------------------------------------------------------------------------*/

template <typename... Acts> inline void runActions (std::tuple<Acts...> &actions)
{
        std::apply ([] (auto &... action) { (action (), ...); }, actions);
}

template <typename Act> inline void runActions (Act &action) { action (); }

template <typename Act> void runSomeActions (Act &actions) { runActions (actions); }

// template <typename Sn, typename EntT, typename TraT, typename ExiT>
// template <typename Ev>
// void State<Sn, EntT, TraT, ExiT>::runExit (Ev const &ev)
// {
//         runActions (ev, exitActions);
// }

// template <typename Sn, typename EntT, typename TraT, typename ExiT> auto state (EntT &&en, TraT &&tra, ExiT &&ex)
// {
//         return State<Sn, EntT, TraT, ExiT> (std::forward<EntT> (en), std::forward<TraT> (tra), std::forward<ExiT> (ex));
// }

template <typename Sn, typename EntT, typename... Tra, typename ExiT> auto state (EntT &&en, ExiT &&ex, Tra &&... tra)
{
        return State<Sn, EntT, decltype (std::make_tuple (tra...)), ExiT> (std::forward<EntT> (en), std::make_tuple (tra...),
                                                                           std::forward<ExiT> (ex));
}

template <typename Sn, typename EntT, typename... Tra, typename ExiT> auto state (Sn, EntT &&en, ExiT &&ex, Tra &&... tra)
{
        return State<Sn, EntT, decltype (std::make_tuple (tra...)), ExiT> (std::forward<EntT> (en), std::make_tuple (tra...),
                                                                           std::forward<ExiT> (ex));
}

template <typename... Acts> constexpr auto entry (Acts &&... act) { return std::make_tuple (act...); }
template <typename Act> constexpr auto entry (Act &&act) { return std::forward<Act> (act); }

template <typename... Act> constexpr auto exit (Act &&... act) { return std::make_tuple (act...); }
template <typename Act> constexpr auto exit (Act &&act) { return std::forward<Act> (act); }

/****************************************************************************/

template <typename StaT> struct Machine {

        Machine (StaT states) : states{std::move (states)} {}

        template <typename Ev> void run (Ev const &ev);

        /// Run functionFunction on currentState.
        template <typename Fun> void forCurrentState (Fun &&function);

        StaT states;
        unsigned int currentStateIndex{Crc32<'I', 'N', 'I', 'T'>::value};
        Timer timer;
};

template <typename... Sta> constexpr auto machine (Sta &&... states) { return Machine (std::make_tuple (states...)); }

namespace impl {
template <typename Fun, typename Sta, typename... Rst> void runIfCurrentState (unsigned int current, Fun &&fun, Sta &state, Rst &... rest)
{
        if (Sta::Name::getIndex () == current) {
                fun (state);
                return;
        }

        if constexpr (sizeof...(rest) > 0) {
                runIfCurrentState (current, std::forward<Fun> (fun), rest...);
        }
}

} // namespace impl

template <typename StaT> template <typename Fun> void Machine<StaT>::forCurrentState (Fun &&function)
{
        std::apply ([&function, this] (auto &... state) { impl::runIfCurrentState (currentStateIndex, std::forward<Fun> (function), state...); },
                    states);
}

namespace impl {
template <typename Fun, typename Ev, typename Tra, typename... Rst>
void runIfMatchingTransition (Fun &&fun, Ev const &ev, Tra &transition, Rst &... rest)
{
        if (transition.condition (ev)) {
                fun (transition);
                return;
        }

        if constexpr (sizeof...(rest)) {
                runIfMatchingTransition (std::forward<Fun> (fun), ev, rest...);
        }
}
} // namespace impl

template <typename Ev, typename TraT, typename Fun> void forMatchingTransition (Ev const &ev, TraT &transitions, Fun &&function)
{
        std::apply ([&ev, &function] (auto &... transition) { impl::runIfMatchingTransition (std::forward<Fun> (function), ev, transition...); },
                    transitions);
}

// template <typename T, typename = std::void_t<>> struct is_state_entry_no_arg : public std::false_type {
// };
// template <typename T> struct is_state_entry_no_arg<T, std::void_t<decltype (std::declval<T &> ().runEntry ())>> : public std::true_type {
// };

template <typename StaT> template <typename Ev> void Machine<StaT>::run (Ev const &ev)
{
        if (!timer.isExpired ()) {
                return;
        }

        // TODO Currently hardcoded currentState to 1

        forCurrentState ([&ev, machine = this] (auto &state) {
                // TODO For all events {}

                // If not run
                if constexpr (std::is_invocable_v<decltype (state.entryActions), Ev const &>) {
                        runSomeActions (ev, state.entryActions);
                }
                else {
                        runSomeActions (state.entryActions);
                }

                forMatchingTransition (ev, state.transitions, [&ev, machine, &state] (auto &transition) {
#ifndef NDEBUG
                // std::cout << "Transition to : " << trans->getStateName () << std::endl;
#endif
                        // machine->forCurrentState ([&ev] (auto &state) { state.runExit (ev); });
                        if constexpr (std::is_invocable_v<decltype (state.exitActions), Ev const &>) {
                                runSomeActions (ev, state.exitActions);
                        }
                        else {
                                runSomeActions (state.exitActions);
                        }

                        transition.runTransitionActions (ev);

                        // if constexpr (std::is_invocable_v<decltype (transition.transitionActions), Ev const &>) {
                        //         runSomeActions (ev, transition.transitionActions);
                        // }
                        // else {
                        //         runSomeActions (transition.transitionActions);
                        // }

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

/****************************************************************************/

int main ()
{
#if 1
        std::vector<std::string> results;
        using namespace std::string_literals;

        auto res = [&results] (const char *const message) { return [&results, message] (auto const &ev) { results.emplace_back (message); }; };
        auto cmd = [&results] (const char *const message) { return [&results, message] { results.emplace_back (message); }; };
        auto eq = [] (const char *what) { return [what] (auto const &i) -> bool { return i == what; }; };

        auto m = machine (state ("INIT"_STATE, entry (cmd ("INIT entry")), exit (cmd ("INIT exit")),
                                 transition ("A"_STATE, eq ("2"), res ("65"), res ("66")))
#define FULL 1
#if FULL
                                  ,
                          state ("A"_STATE, entry (cmd ("A entry")), exit (cmd ("A exit")),
                                 transition ("INIT"_STATE, eq ("-3"), res ("action"), res ("another")),
                                 transition ("B"_STATE, eq ("3"), res ("action"), res ("another"))),

                          state ("B"_STATE, entry (cmd ("B entry")), exit (cmd ("B exit")),
                                 transition ("A"_STATE, eq ("-4"), res ("action"), res ("another")),
                                 transition ("C"_STATE, eq ("4"), res ("action"), res ("another"))),

                          state ("C"_STATE, entry (cmd ("D entry")), exit (cmd ("D exit")),
                                 transition ("B"_STATE, eq ("-5"), res ("action"), res ("another")),
                                 transition ("D"_STATE, eq ("5"), res ("action"), res ("another"))),

                          state ("D"_STATE, entry (cmd ("E entry")), exit (cmd ("E exit")),
                                 transition ("C"_STATE, eq ("-6"), res ("action"), res ("another")),
                                 transition ("E"_STATE, eq ("6"), res ("action"), res ("another"))),

                          state ("E"_STATE, entry (cmd ("F entry")), exit (cmd ("F exit")),
                                 transition ("D"_STATE, eq ("-7"), res ("action"), res ("another")),
                                 transition ("F"_STATE, eq ("7"), res ("action"), res ("another"))),

                          state ("F"_STATE, entry (cmd ("G entry")), exit (cmd ("G exit")),
                                 transition ("E"_STATE, eq ("-8"), res ("action"), res ("another")),
                                 transition ("G"_STATE, eq ("8"), res ("action"), res ("another"))),

                          state ("G"_STATE, entry (cmd ("H entry")), exit (cmd ("H exit")),
                                 transition ("F"_STATE, eq ("-9"), res ("action"), res ("another")),
                                 transition ("H"_STATE, eq ("9"), res ("action"), res ("another"))),

                          state ("H"_STATE, entry (cmd ("I entry")), exit (cmd ("I exit")),
                                 transition ("G"_STATE, eq ("-10"), res ("action"), res ("another")),
                                 transition ("I"_STATE, eq ("10"), res ("action"), res ("another"))),

                          state ("I"_STATE, entry (cmd ("J entry")), exit (cmd ("J exit")),
                                 transition ("H"_STATE, eq ("-11"), res ("action"), res ("another")),
                                 transition ("J"_STATE, eq ("11"), res ("action"), res ("another"))),

                          state ("J"_STATE, entry (cmd ("K entry")), exit (cmd ("K exit")),
                                 transition ("I"_STATE, eq ("-12"), res ("action"), res ("another")),
                                 transition ("K"_STATE, eq ("12"), res ("action"), res ("another"))),

                          state ("K"_STATE, entry (cmd ("L entry")), exit (cmd ("L exit")),
                                 transition ("J"_STATE, eq ("-13"), res ("action"), res ("another")),
                                 transition ("L"_STATE, eq ("13"), res ("action"), res ("another"))),

                          state ("L"_STATE, entry (cmd ("M entry")), exit (cmd ("M exit")),
                                 transition ("K"_STATE, eq ("-14"), res ("action"), res ("another")),
                                 transition ("M"_STATE, eq ("14"), res ("action"), res ("another"))),

                          state ("M"_STATE, entry (cmd ("N entry")), exit (cmd ("N exit")),
                                 transition ("L"_STATE, eq ("-15"), res ("action"), res ("another")),
                                 transition ("N"_STATE, eq ("15"), res ("action"), res ("another"))),

                          state ("N"_STATE, entry (cmd ("O entry")), exit (cmd ("O exit")),
                                 transition ("M"_STATE, eq ("-16"), res ("action"), res ("another")),
                                 transition ("FINAL"_STATE, eq ("16"), res ("action"), res ("another"))),

                          state ("FINAL"_STATE, entry (cmd ("FINAL entry")), exit (cmd ("")), transition ("FINAL"_STATE, eq ("-17")))
#endif
        );

        m.run ("1"s); // TODO already in init state

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
                                                 transition(eq (-2), res (65), res (66)),
                                                                  transition(eq (2), res (65), res (66))),
                                                 res (3), res (4))
#define FULL 1
#if FULL
                                         ,
                                 state<Name<2>> (res (5), res (6),
                                                 transition(eq (-3), res (65), res (66)),
                                                                  transition(eq (3), res (65), res (66))),
                                                 res (7), res (8)),

                                 state<Name<3>> (res (9), res (10),
                                                 transition(eq (-4), res (65), res (66)),
                                                                  transition(eq (4), res (65), res (66))),
                                                 res (11), res (12)),

                                 state<Name<4>> (res (13), res (14),
                                                 transition(eq (-5), res (65), res (66)),
                                                                  transition(eq (5), res (65), res (66))),
                                                 res (15), res (16)),

                                 state<Name<5>> (res (17), res (18),
                                                 transition(eq (-6), res (65), res (66)),
                                                                  transition(eq (6), res (65), res (66))),
                                                 res (19), res (20)),

                                 state<Name<6>> (res (21), res (22),
                                                 transition(eq (-7), res (65), res (66)),
                                                                  transition(eq (7), res (65), res (66))),
                                                 res (23), res (24)),

                                 state<Name<7>> (res (25), res (26),
                                                 transition(eq (-8), res (65), res (66)),
                                                                  transition(eq (8), res (65), res (66))),
                                                 res (27), res (28)),

                                 state<Name<8>> (res (29), res (30),
                                                 transition(eq (-9), res (65), res (66)),
                                                                  transition(eq (9), res (65), res (66))),
                                                 res (31), res (32)),

                                 state<Name<9>> (res (33), res (34),
                                                 transition(eq (-10), res (65), res (66)),
                                                                  transition ("A"_STATE, eq (10), res (65), res (66))),
                                                 res (35), res (36)),

                                 state<Name<10>> (res (37), res (38),
                                                  transition(eq (-11), res (65), res (66)),
                                                                   transition ("A"_STATE, eq (11), res (65), res (66))),
                                                  res (39), res (40)),

                                 state<Name<11>> (res (41), res (42),
                                                  transition ("A"_STATE, eq (-12), res (65), res (66)),
                                                                   transition ("A"_STATE, eq (12), res (65), res (66))),
                                                  res (43), res (44)),

                                 state<Name<12>> (res (45), res (46),
                                                  transition ("A"_STATE, eq (-13), res (65), res (66)),
                                                                   transition ("A"_STATE, eq (13), res (65), res (66))),
                                                  res (47), res (48)),

                                 state<Name<13>> (res (49), res (50),
                                                  transition ("A"_STATE, eq (-14), res (65), res (66)),
                                                                   transition ("A"_STATE, eq (14), res (65), res (66))),
                                                  res (51), res (52)),

                                 state<Name<14>> (res (53), res (54),
                                                  transition ("A"_STATE, eq (-15), res (65), res (66)),
                                                                   transition ("A"_STATE, eq (15), res (65), res (66))),
                                                  res (55), res (56)),

                                 state<Name<15>> (res (57), res (58),
                                                  transition ("A"_STATE, eq (-16), res (65), res (66)),
                                                                   transition ("A"_STATE, eq (16), res (65), res (66))),
                                                  res (59), res (60)),

                                 state<Name<16>> (res (61), res (62),
                                                  transition ("A"_STATE, eq (-17), res (65), res (66)),
                                                                   transition ("A"_STATE, eq (17), res (65), res (66))),
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
  46.0%  14.1Ki  73.3%  14.1Ki    .text
  16.1%  4.97Ki   0.0%       0    [Unmapped]
   8.0%  2.45Ki   0.0%       0    .strtab
   6.7%  2.05Ki   0.0%       0    .symtab
   2.3%     720   3.3%     656    .eh_frame
   2.2%     691   3.5%     691    [LOAD #2 [R]]
   2.2%     685   3.2%     621    .dynstr
   1.9%     592   2.7%     528    .dynamic
   1.9%     592   2.7%     528    .dynsym
   1.4%     447   1.9%     383    .rodata
   1.3%     424   1.8%     360    .rela.plt
   1.1%     354   0.0%       0    .shstrtab
   1.0%     328   1.3%     264    .rela.dyn
   1.0%     320   1.3%     256    .plt
   0.9%     291   1.2%     227    .gcc_except_table
   0.8%     240   0.9%     176    .gnu.version_r
   0.7%     208   0.7%     144    .got.plt
   0.5%     156   0.5%      92    .eh_frame_hdr
   0.4%     128   0.0%       0    [ELF Headers]
   0.3%     108   0.2%      44    .gnu.version
   0.3%     104   0.2%      40    .got
   0.3%     100   0.2%      36    .note.gnu.build-id
   0.3%      96   0.2%      32    .note.ABI-tag
   0.3%      92   0.1%      28    .gnu.hash
   0.3%      92   0.1%      28    .interp
   0.3%      91   0.1%      27    .init
   0.3%      81   0.0%       0    .comment
   0.3%      80   0.1%      16    .data
   0.3%      80   0.1%      16    .data.rel.ro
   0.2%      77   0.1%      13    .fini
   0.2%      72   0.0%       8    .fini_array
   0.2%      72   0.0%       8    .init_array
   0.0%       0   0.0%       8    .bss
   0.0%       8   0.0%       8    [LOAD #3 [RX]]
   0.0%       5   0.0%       5    [LOAD #4 [R]]
 100.0%  30.8Ki 100.0%  19.2Ki    TOTAL


-O0 stripped
    FILE SIZE        VM SIZE
 --------------  --------------
  56.6%  82.8Ki  60.4%  82.8Ki    .text
  26.3%  38.4Ki  28.0%  38.3Ki    .eh_frame
   6.3%  9.26Ki   6.7%  9.20Ki    .eh_frame_hdr
   4.9%  7.22Ki   0.0%       0    [Unmapped]
   1.0%  1.50Ki   1.1%  1.44Ki    .dynstr
   0.6%     928   0.6%     864    .dynsym
   0.6%     839   0.6%     775    .gcc_except_table
   0.5%     736   0.5%     672    .rela.plt
   0.5%     688   0.5%     688    [LOAD #2 [R]]
   0.4%     592   0.4%     528    .dynamic
   0.4%     556   0.4%     492    .rodata
   0.4%     528   0.3%     464    .plt
   0.2%     325   0.0%       0    .shstrtab
   0.2%     312   0.2%     248    .got.plt
   0.2%     280   0.2%     216    .rela.dyn
   0.1%     208   0.1%     144    .gnu.version_r
   0.1%     136   0.1%      72    .gnu.version
   0.1%     128   0.0%       0    [ELF Headers]
   0.1%     104   0.0%      40    .got
   0.1%     100   0.0%      36    .gnu.hash
   0.1%     100   0.0%      36    .note.gnu.build-id
   0.1%      96   0.0%      32    .note.ABI-tag
   0.1%      92   0.0%      28    .interp
   0.1%      91   0.0%      27    .init
   0.1%      81   0.0%       0    .comment
   0.1%      80   0.0%      16    .data
   0.1%      77   0.0%      13    .fini
   0.0%      72   0.0%       8    .fini_array
   0.0%      72   0.0%       8    .init_array
   0.0%       0   0.0%       8    .bss
   0.0%       8   0.0%       8    [LOAD #3 [RX]]
 100.0%   146Ki 100.0%   137Ki    TOTAL


*/
