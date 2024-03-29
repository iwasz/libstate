/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#pragma once
#include <array>
#include <tuple>
#include <utility>

namespace ls {

/****************************************************************************/

static auto False = [] (auto const & /* ev */) { return false; };
static auto True = [] (auto const & /* ev */) { return true; };

auto Not (auto const &cond)
{
        return [cond]<typename Evt> (Evt &&evt) { return !cond (std::forward<Evt> (evt)); };
};

auto And (auto const &...cond)
{
        return [cond...]<typename Evt> (Evt &&evt) { return (cond (std::forward<Evt> (evt)) && ...); };
}

auto Or (auto const &...cond)
{
        return [cond...]<typename Evt> (Evt &&evt) { return (cond (std::forward<Evt> (evt)) || ...); };
}

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

template <unsigned int CRC_VALUE, char... Chars> struct Crc32Impl {};

template <unsigned int CRC_VALUE, char Head, char... Tail> struct Crc32Impl<CRC_VALUE, Head, Tail...> {
        static constexpr unsigned int value
                = Crc32Impl<crc32_table[static_cast<unsigned char> (CRC_VALUE) ^ static_cast<unsigned char> (Head)] ^ (CRC_VALUE >> 8),
                            Tail...>::value;
};

template <unsigned int CRC_VALUE> struct Crc32Impl<CRC_VALUE> {
        static constexpr unsigned int value = CRC_VALUE ^ 0xFFFFFFFF;
};

template <char... Chars> using Crc32 = Crc32Impl<0xFFFFFFFF, Chars...>;

template <char... s> struct Name2 {

        constexpr static const char *c_str () { return name.data (); }
        constexpr static std::array<char, sizeof...(s) + 1> name{s..., '\0'};

        static constexpr unsigned int getIndex () { return Crc32<s...>::value; }
};

template <char... s> constexpr Name2<s...> Name2_c{};

// TODO This is a GNU extension. Provide macro as an option.
template <typename C, C... c> constexpr auto operator""_ST () { return Name2_c<c...>; }

/****************************************************************************/

template <typename Sn, typename Con, typename TacT> struct Transition {

        using Name = Sn;

        explicit Transition (Con condition, TacT transitionActions)
            : condition{std::move (condition)}, transitionActions{std::move (transitionActions)}
        {
        }

        template <typename Ev> void runTransitionActions (Ev const &ev);

        Con condition;
        TacT transitionActions;
};

template <typename Sn, typename Con, typename TacT> template <typename Ev> void Transition<Sn, Con, TacT>::runTransitionActions (Ev const &ev)
{
        std::apply ([&ev] (auto &...transitionAction) { (transitionAction (ev), ...); }, transitionActions);
}

template <typename Sn, typename Con, typename... Tac> auto transition (Sn /* where */, Con &&condition, Tac &&...actions)
{
        return Transition<Sn, std::decay_t<Con>, decltype (std::make_tuple (actions...))> (std::forward<Con> (condition),
                                                                                           std::make_tuple (actions...));
}

/****************************************************************************/

template <typename Sn, typename EntT, typename TraT, typename ExiT> struct State {
        using Name = Sn;

        State (EntT en, TraT tr, ExiT ex) : entryActions{std::move (en)}, transitions{std::move (tr)}, exitActions{std::move (ex)} {}

        EntT entryActions;
        TraT transitions;
        ExiT exitActions;
};

template <typename T> struct is_state : public std::bool_constant<false> {};
template <typename Sn, typename EntT, typename TraT, typename ExiT>
struct is_state<State<Sn, EntT, TraT, ExiT>> : public std::bool_constant<true> {};

/// For a callable (single action)
template <typename Ev, typename Act> inline void runActions (Ev const &ev, Act &action)
{
        if constexpr (std::is_invocable_v<decltype (action), Ev const &>) {
                action (ev);
        }
        else if constexpr (std::is_invocable_v<decltype (action)>) {
                action ();
        }

        // Any other type will result with an empty operation
}

/// For a tuple of callables
template <typename Ev, typename... Acts> inline void runActions (Ev const &ev, std::tuple<Acts...> &actions)
{
        std::apply ([&ev] (auto &...action) { (runActions (ev, action), ...); }, actions);
}

template <typename Ev, typename Act> void runSomeActions (Ev const &ev, Act &actions) { runActions (ev, actions); }

/*--------------------------------------------------------------------------*/

template <typename... Acts> inline void runActions (std::tuple<Acts...> &actions)
{
        // Works with std::refs because std::reference_wrapper has operator()
        std::apply ([] (auto &...action) { (action (), ...); }, actions);
}

template <typename Act> inline void runActions (Act &action) { action (); }

template <typename Act> void runSomeActions (Act &actions) { runActions (actions); }

template <typename Act> struct EntryActions {
        explicit EntryActions (Act a) : act{std::move (a)} {}
        Act act; /// std::tuple or a single callable
};

template <typename... Acts> constexpr auto entry (Acts &&...act) { return EntryActions{std::make_tuple (act...)}; }
template <typename Act> constexpr auto entry (Act &&act) { return EntryActions{std::forward<Act> (act)}; }

template <typename T> struct is_entry_action : public std::bool_constant<false> {};
template <typename T> struct is_entry_action<EntryActions<T>> : public std::bool_constant<true> {};

/*--------------------------------------------------------------------------*/

template <typename Act> struct ExitActions {
        explicit ExitActions (Act a) : act{std::move (a)} {}
        Act act; /// std::tuple or a single callable
};

template <typename... Act> constexpr auto exit (Act &&...act) { return ExitActions{std::make_tuple (act...)}; }
template <typename Act> constexpr auto exit (Act &&act) { return ExitActions{std::forward<Act> (act)}; }

template <typename T> struct is_exit_action : public std::bool_constant<false> {};
template <typename T> struct is_exit_action<ExitActions<T>> : public std::bool_constant<true> {};

/*--------------------------------------------------------------------------*/

template <typename Sn, typename EntT, typename ExiT, typename... Snn, typename... Con, typename... TacT>
auto state (Sn /* stateName */, EntryActions<EntT> &&en, ExitActions<ExiT> &&ex, Transition<Snn, Con, TacT> &&...tra)
{
        return State<Sn, EntryActions<EntT>, decltype (std::make_tuple (tra...)), ExitActions<ExiT>> (
                std::forward<EntryActions<EntT>> (en), std::make_tuple (tra...), std::forward<ExitActions<ExiT>> (ex));
}

template <typename Sn, typename EntT, typename... Snn, typename... Con, typename... TacT>
auto state (Sn /* stateName */, EntryActions<EntT> &&en, Transition<Snn, Con, TacT> &&...tra)
{
        return State<Sn, EntryActions<EntT>, decltype (std::make_tuple (tra...)), ExitActions<int>> (
                std::forward<EntryActions<EntT>> (en), std::make_tuple (tra...), ExitActions<int>{0});
}

template <typename Sn, typename... Snn, typename... Con, typename... TacT> auto state (Sn /* stateName */, Transition<Snn, Con, TacT> &&...tra)
{
        return State<Sn, EntryActions<int>, decltype (std::make_tuple (tra...)), ExitActions<int>> (
                EntryActions<int>{0}, std::make_tuple (tra...), ExitActions<int>{0});
}

template <typename Sn, typename EntT> auto state (Sn /* stateName */, EntryActions<EntT> &&en)
{
        return State<Sn, EntryActions<EntT>, decltype (std::make_tuple ()), ExitActions<int>> (std::forward<EntryActions<EntT>> (en),
                                                                                               std::make_tuple (), ExitActions<int>{0});
}

/****************************************************************************/

struct EmptyGlobal {
        std::tuple<> transitions{};
};

template <typename TraT> struct Global {
        explicit Global (TraT tr) : transitions{std::move (tr)} {}
        TraT transitions;
};

template <typename... Snn, typename... Con, typename... TacT> auto global (Transition<Snn, Con, TacT> &&...tra)
{
        return Global (std::make_tuple (std::forward<Transition<Snn, Con, TacT>> (tra)...));
}

template <typename T> struct is_global : public std::bool_constant<false> {};
template <typename TraT> struct is_global<Global<TraT>> : public std::bool_constant<true> {};

/****************************************************************************/

//  this is not implemented. Rethink.
struct Instrumentation {
        // void onEntry (const char *currentStateName, unsigned int currentStateIndex) {}
        // void onExit (const char *currentStateName, unsigned int currentStateIndex, int acceptedTransNumber) {}
};

template <typename StaT, typename Ins, typename Glob = EmptyGlobal> class Machine {
public:
        Machine (StaT states, Ins const &ins, Glob const &glob = {}) : states{std::move (states)}, instrumentation{ins}, global{glob} {}

        /// returns whether the state was changed
        template <typename Ev> bool run (Ev const &ev);

        /// Run functionFunction on currentState.
        template <typename Fun> void forCurrentState (Fun &&function);

        /// States are distinguished at runtime by unique IDS.
        auto getCurrentStateIndex () const { return currentStateIndex; }
        /// For debugging purposes
        const char *getCurrentStateName () const { return currentStateName; }

        StaT states;

private:
        bool entryRun{};
        unsigned int currentStateIndex{std::tuple_element<0, StaT>::type::Name::getIndex ()};
        const char *currentStateName{std::tuple_element<0, StaT>::type::Name::c_str ()};
        Ins instrumentation;
        Glob global;
};

template <typename... Sta, typename Ins = Instrumentation>
        requires std::conjunction_v<is_state<Sta>...>
constexpr auto machine (Sta &&...states)
{
        return Machine<decltype (std::make_tuple (std::forward<Sta> (states)...)), Ins> (std::make_tuple (std::forward<Sta> (states)...), {});
}

template <typename... Sta, typename Ins = Instrumentation, typename Glob>
        requires std::conjunction_v<is_state<Sta>...> && is_global<Glob>::value
constexpr auto machine (Glob &&glob, Sta &&...states)
{
        return Machine<decltype (std::make_tuple (std::forward<Sta> (states)...)), Ins, std::unwrap_ref_decay_t<Glob>> (
                std::make_tuple (std::forward<Sta> (states)...), {}, std::forward<Glob> (glob));
}

template <typename... Sta, typename Ins>
        requires (!is_state<Ins>::value) && (!is_global<Ins>::value) && std::conjunction_v<is_state<Sta>...>
constexpr auto machine (Ins &&instrumentation, Sta &&...states)
{
        return Machine<decltype (std::make_tuple (std::forward<Sta> (states)...)), std::unwrap_ref_decay_t<Ins>> (
                std::make_tuple (std::forward<Sta> (states)...), std::forward<Ins> (instrumentation));
}

template <typename... Sta, typename Ins, typename Glob>
        requires (!is_state<Ins>::value) && (!is_global<Ins>::value) && std::conjunction_v<is_state<Sta>...> && is_global<Glob>::value
constexpr auto machine (Ins &&instrumentation, Glob &&glob, Sta &&...states)
{
        return Machine<decltype (std::make_tuple (std::forward<Sta> (states)...)), std::unwrap_ref_decay_t<Ins>, std::unwrap_ref_decay_t<Glob>> (
                std::make_tuple (std::forward<Sta> (states)...), std::forward<Ins> (instrumentation), std::forward<Glob> (glob));
}

namespace detail {
        template <typename Fun, typename Sta, typename... Rst> void runIfCurrentState (unsigned int current, Fun &&fun, Sta &state, Rst &...rest)
        {
                if (Sta::Name::getIndex () == current) {
                        fun (state);
                        return;
                }

                if constexpr (sizeof...(rest) > 0) {
                        runIfCurrentState (current, std::forward<Fun> (fun), rest...);
                }
        }

} // namespace detail

template <typename StaT, typename Ins, typename Glob> template <typename Fun> void Machine<StaT, Ins, Glob>::forCurrentState (Fun &&function)
{
        std::apply (
                [&function, this] (auto &...state) { detail::runIfCurrentState (currentStateIndex, std::forward<Fun> (function), state...); },
                states);
}

namespace detail {
        template <typename Fun, typename Ev, typename Tra, typename... Rst>
        void runIfMatchingTransition (Fun &&fun, Ev const &ev, int acceptedTransNumber, Tra &transition, Rst &...rest)
        {
                if (transition.condition (ev)) {
                        fun (transition, acceptedTransNumber);
                        return;
                }

                if constexpr (sizeof...(rest)) {
                        runIfMatchingTransition (std::forward<Fun> (fun), ev, acceptedTransNumber + 1, rest...);
                }
        }
} // namespace detail

template <typename Ev, typename TraT, typename Fun> void forMatchingTransition (Ev const &ev, TraT &transitions, Fun &&function)
{
        // If there are any transitions at all
        if constexpr (std::tuple_size<TraT>::value > 0) {
                std::apply (
                        [&ev, &function] (auto &...transition) {
                                detail::runIfMatchingTransition (std::forward<Fun> (function), ev, 0, transition...);
                        },
                        transitions);
        }
}

template <typename StaT, typename Ins, typename Glob> template <typename Ev> bool Machine<StaT, Ins, Glob>::run (Ev const &ev)
{
        bool stateChangedAtLeastOnce{};

        while (true) {
                bool stateChanged{};

                forCurrentState ([&ev, &stateChanged, &stateChangedAtLeastOnce, machine = this] (auto &state) {
                        if (!machine->entryRun) {
                                machine->entryRun = true;

                                if constexpr (requires {
                                                      machine->instrumentation.onEntry (
                                                              std::remove_reference_t<decltype (state)>::Name::c_str (),
                                                              std::remove_reference_t<decltype (state)>::Name::getIndex ());
                                              }) {
                                        machine->instrumentation.onEntry (std::remove_reference_t<decltype (state)>::Name::c_str (),
                                                                          std::remove_reference_t<decltype (state)>::Name::getIndex ());
                                }

                                runActions (ev, state.entryActions.act);
                                return;
                        }

                        auto onTransition = [&ev, &stateChanged, &stateChangedAtLeastOnce, machine, &state] (auto &transition,
                                                                                                             int acceptedTransNumber) {
                                runActions (ev, state.exitActions.act);

                                if constexpr (requires {
                                                      machine->instrumentation.onExit (
                                                              std::remove_reference_t<decltype (state)>::Name::c_str (),
                                                              std::remove_reference_t<decltype (state)>::Name::getIndex (), acceptedTransNumber);
                                              }) {
                                        machine->instrumentation.onExit (std::remove_reference_t<decltype (state)>::Name::c_str (),
                                                                         std::remove_reference_t<decltype (state)>::Name::getIndex (),
                                                                         acceptedTransNumber);
                                }

                                transition.runTransitionActions (ev);
                                machine->currentStateIndex = std::remove_reference_t<decltype (transition)>::Name::getIndex ();
                                machine->currentStateName = std::remove_reference_t<decltype (transition)>::Name::c_str ();
                                stateChangedAtLeastOnce = stateChanged = true;
                                machine->entryRun = false; // TODO what should happen in case of an exception thworwn from exit or
                                                           // transition action?
                        };

                        // TODO global should go after speciffic. This way we could override!
                        forMatchingTransition (ev, machine->global.transitions, onTransition);
                        forMatchingTransition (ev, state.transitions, onTransition);
                });

                if (!stateChanged || entryRun) {
                        break;
                }
        }

        return stateChangedAtLeastOnce;
}

} // namespace ls
