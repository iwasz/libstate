/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#pragma once
#include <boost/hana.hpp>
#include <gsl/gsl>
#include <optional>
// TODO remove
#include <string>
// TODO remove
#include <iostream>

// TODO remove thise statements
namespace hana = boost::hana;
using namespace hana::literals;
using namespace std::string_literals;

/**
 * Copy (in Polish) of TODOs from the previous version of the lib.
 *
 * TODO Dodać globalny timeout. Jeśli w danym timeoucie nie uda się zmienić stanu, to trzeba podjąć jakąć
 * akcję. Ej to chyba jest!
 *
 * TODO dokumentacja z tutorialem jest niezbędna, bo ja sam mam czasem problemy. Jak są ewaluowane warunki,
 * co do nich wpada, w jakiej kolejności i tak dalej. Opisać wszystkie aspekty działania : jak dwie maszyny
 * mogą pracować na jedym input/output i tak dalej.
 *
 * TODO opisać, że zawsze trzeba pamiętyać czy jest odpowiednia ilość czasu na sprawdzenie warunków. Podać taki przykład:
 * Kiedy jest jeden warunek na przejście, który oczekuje jakichś danych, to nie ma problemu. Na przykład :
 *
 *         m->state (INIT, State::INITIAL)->entry (at ("ATE1\r\n"))
 *               ->transition (CHECK_OPERATING_MODE)->when (&ok)->then (&delay);
 *
 * Maszyna będzie tak długo się kręciuć, aż dostanie "OK". Jeżeli jednak mamy warunki zdefiniowane tak, że oczekiwane wejście
 * może nie zdążyć się pojawić, to trzeba dodać opóźnienie, żeby poczekać na te dane wejściowe. Przykład (błędny):
 *
 *         m->state (CHECK_OPERATING_MODE)->entry (at ("AT+CWMODE_DEF?\r\n"))
 *               ->transition (VERIFY_CONNECTED_ACCESS_POINT)->when (eq ("+CWMODE_DEF:1"))->then (&delay)
 *               ->transition (SET_OPERATING_MODE)->when (&alwaysTrue)->then (&delay);
 *
 * W powyższym przykładzie warunki dwóch przejść są sprawdzane jedno po drugim. Najpierw maszyna sprawdzi, czy w kolejce
 * wejściowej jest jakiś element z napisem "+CWMODE_DEF:1", a jeśli nie ma, to natychmiast przejdzie do stanu SET_OPERATING_MODE.
 * Trzeba dać maszynie trochę czasu na oczekiwanie na dane. Powinno być:
 *
 *         m->state (CHECK_OPERATING_MODE)->entry (and_action (at ("AT+CWMODE_DEF?\r\n"), &delay))
 *               ->transition (VERIFY_CONNECTED_ACCESS_POINT)->when (eq ("+CWMODE_DEF:1"))->then (&delay)
 *               ->transition (SET_OPERATING_MODE)->when (&alwaysTrue)->then (&delay);
 *
 * Taki błąd jak wyżej może prowadzić do trudnych do wykrycia konsekwencji. Jeśli jak wyżej mamy dwa przejścia stanowe, to
 * dość łatwo będzie się zorientować, że mimo że modem wysłał odpowiedź "+CWMODE_DEF:1" to jesteśmy w słym stanie (w SET_OPERATING_MODE
 * zamiast VERIFY_CONNECTED_ACCESS_POINT). Ale w przypadku gdy mamy takie przejście (błędne - brakuje delay):
 *
 *         m->state (CLOSE_AND_RECONNECT)->entry (at ("AT+CIPCLOSE=0\r\n"))
 *               ->transition (CONNECT_TO_SERVER)->when(&alwaysTrue)->then (&delay);
 *
 * To w efekcie maszyna przejdzie prawidłowo do stanu CONNECT_TO_SERVER, ale *za szybko* i może złapać jescze dane wejściowe
 * które jej nie interesują i które spowodują dalsze nieprawidłowe przejścia.
 *
 * TODO Ten problem pojawia się częściej niż mi się wydawało rozwiązaniem jest :
 * - Po każdym żądaniu do modemu powinniśmy odczekać chwilkę, bo przecież odpowiedzi
 * może być więcej niż 1 i przychodzą w pewnym okresie czasu (po wysłaniu żądania,
 * ale jest jednak pewne opóźniene). Może akcja at niech się łączy z akcją delay - przydał
 * by się "szablon"!
 * - Wrunki przejść powinniśmy konstruować tak, żeby wymagały wszystkich możliwych
 * odpowiedzi z modemu, czyli trzba oczekiwac na echo, na jakieś tam dane i na OK.
 * - TODO Oczekiwać odpowiedzi w odpowiedniej kolejności - czytaj następne TODO.
 * Błędy niestety są trudne do wykrycia. Przykład z życia : miałem takie przejścia:
 *
 *         m->state (PDP_CONTEXT_CHECK)->entry (at ("AT+QIACT?\r\n"))
 *               ->transition(DNS_CONFIG)->when (like ("+QIACT:%1,%"))->then (&delay)
 *               ->transition(ACTIVATE_PDP_CONTEXT)->when (anded (beginsWith ("AT+QIACT?"), &ok))->then (&delay);
 *
 * Tu mamy odpowiedź z modemu :
 * IN : AT+QIACT?
 * IN : +QIACT: 1,1,1,"37.249.238.180"
 * IN : OK
 *
 * Uwaga! Są 3 odpowiedzi, ale już po drugiej warunek przejścia do DNS_CONFIG jest spełniony i wykonuje się przejście,
 * następnie przychodzi odpowiedź OK co oznacza, że w kolejce zostaje OK i ono trafi jako wejście w następnym stanie
 * (innymi słowy stan ACTIVATE_PDP_CONTEXT będzie myślał, że już ma odpowiedź OK).
 *
 *       m->state (ACTIVATE_PDP_CONTEXT)->entry (at ("AT+QIACT=1\r\n"))
 *               ->transition(DNS_CONFIG)->when (anded (beginsWith ("AT+QIACT="), &ok))->then (&delay);
 *
 * IN : AT+QIACT=1
 * IN : ERROR
 *
 * Czyli otrzymaliśmy błąd! Żadne przejście nie powinno sie wykonać. Ale niestety w kolejce zostało OK, więc się wykonuje
 * przejście do DNS_CONFIG. A następnie przychodzi odpowiedź ERROR.
 *
 *       m->state (DNS_CONFIG)->entry (at ("AT+QIDNSCFG=1,\"8.8.8.8\",\"8.8.4.4\"\r\n"))
 *               ->transition (INIT)->when (&error)->then (&longDelay)
 *               ->transition (NETWORK_GPS_USART_ECHO_OFF)->when (anded (beginsWith ("AT+QIDNSCFG="), &ok))->then (&delay);
 *
 * No tu jest kompletna klapa, bo idzie do init, bo ma error.
 *
 * TODO Powyższy problem jest poważny, bo kolejny raz z nim walczę. Problem by się (częściowo?) rozwiązał gdyby
 * warunki były spełnione, tylko gdy odpowiedzi przyjdą w odpowiedniej kolejności.
 *
 * TODO Zastanowić się, czy dałoby się *łatwo* uniezaleznić tę implementację od StringQueue. Jeśli tak, to
 * implementacja byłaby bardziej uniwersalna.
 *
 * TODO Powstał problem odbierania danych binarnych. Ja zaszyłem w maszynie funkcjonalność operacji na zero-ended
 * strings i przez to nie mam teraz jak obsłużyć danych binarnych. Jeszcze gorzej, że zaszyłem na stałe uzywanie
 * konkretnej kolekcji StringQueue, której elementy mają zawsze 128B W przypadku próby odbierania większej ilości
 * danych to będzie problem (przydał by się CircularBuffer). Natomiast w przypadku próby odbierania całego firmware,
 * to już w ogóle będzie masakra (bo wtedy MySink musiałby kopiowac dane do jakiegoś mega bufora, albo wręcz nagrywać
 * je bezpośrednio na flash).
 *
 * TODO Pousuwać konstruktyory i metody, których nie używam w ogóle.
 *
 * TODO Zależność od libmicro dać jako opcję (ifdef) EDIT : sporo rzeczy nie będzie działać, ale trudno.
 *
 * TODO W walgrindzie jak się uruchomi unitytest to jeden nie przechodzi.
 *
 * TODO Trzeba opisac jak działa NotCondition, bo to nieintuicyjnie działa gdy jest więcej niż 1 odpowiedź (? przykład ?).
 *
 * TODO Easier interface for constructing the machine and for lambdas
 *
 * TODO Event arguments would resolve the problem of passing data between actions in elegant way and
 * would allow more flexible condition checks.
 *
 * TODO It would be fantastic if transitions, entry and exit actions were fired in
 * natural order that is entry , transition, transition action, exit. Now all actions
 * are run all at once.
 *
 * TODO Deferred event implenentation is shitty.
 *
 * TODO strasznie ciężko jest się połapać co się dzieje kiedy maszyna źle działa. Trzebaby poprawić debugi
 * tak, żeby pokazywały wejścia, zamiany stanów, wywołania akcji I WARUNKI we własciwej kolejności!
 *
 * TODO Żeby dało się robić transition bez warunków (co by działo jak alwaysTrue).
 *
 * TODO żeby do eventów moża było bez problemu dodawać dowlną liczbę argumentów o dowolnych typach.

TODO żeby nazwy stanów nie musiałby być definiowane osobno w enumie, i żeby łatwo się je dało
                                                                             * wypisać na debug. Może user-defined literals?
        */

/*
 * NEW todos for the new version
 * TODO Allow raising an event from inside of actions (somehow).
 * TODO The code is so generic, that it sometimes accepts wring type of arguments,
 * like you can pass transition instead of a state. It should be convinient if
 * the compiler reported shuch a misuse on early stage and with meaningful messages.
 */

namespace ls {

class StateName;
std::ostream &operator<< (std::ostream &o, StateName const &s) noexcept;

class StateName {
public:
        explicit StateName (gsl::not_null<gsl::czstring<>> s, std::size_t len) : name (s, len) { Expects (len > 0); }
        friend std::ostream &operator<< (std::ostream &o, StateName const &s) noexcept;

private:
        std::string_view name;
};

std::ostream &operator<< (std::ostream &o, StateName const &s) noexcept
{
        o << s.name;
        return o;
}

/// TODO optimize! Use template variant, and comute some unique number (hash) of the string, and use this hash to distinguish between the
/// StateNames
StateName operator"" _STATE (const char *s, std::size_t len) { return StateName (s, len); }

/// Do zwracania z akcji.
enum class Done { NO, YES };

/**
 * It's a wrapper for an action which can run it conditionally. It works like that : if 'active' flag is true, the operator()
 * method will run the 'action'. Then, if the return value from that 'action' equals Done::YES, 'active' is flipped to
 * false, and next time the action will not be run.
 */
template <typename T> class ActionRunner {
public:
        explicit ActionRunner (T t) : action (std::move (t)) {}

        /**
         * It does not have to use paramater pack. I think actions API will always have
         * only one parameter, but let leave it for now.
         */
        template <typename... Arg> void operator() (Arg &&... a)
        {
                if (!active) {
                        return;
                }

                Done done = Done::YES;

                if constexpr (std::is_invocable<T, Arg...>::value) {
                        if constexpr (std::is_same<typename std::result_of<T (Arg...)>::type, Done>::value) {
                                done = action (std::forward<Arg> (a)...);
                        }
                        else {
                                action (std::forward<Arg> (a)...);
                        }
                }
                else {
                        if constexpr (std::is_same<typename std::result_of<T ()>::type, Done>::value) {
                                done = action ();
                        }
                        else {
                                action ();
                        }
                }

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
        explicit ActionTuple (T a) : actions{std::move (a)} {}

        template <typename Ev> void operator() (Ev const &event)
        {
                hana::for_each (actions, [&event](auto &f) { f (event); });
        }

private:
        T actions;
};

template <> class ActionTuple<void> {
public:
        ActionTuple () = default;
        template <typename Ev> void operator() (Ev const & /*event*/) {}
};

template <typename T> struct Entry : public ActionTuple<T> {
        using ActionTuple<T>::ActionTuple;
};

template <typename T> struct Exit : public ActionTuple<T> {
        using ActionTuple<T>::ActionTuple;
};

template <template <typename T> class Tu, typename... Ar> auto actionTuple (Ar &&... args)
{
        auto tuple = hana::make_tuple (ActionRunner (std::forward<Ar> (args))...);
        auto at = Tu<decltype (tuple)> (std::move (tuple));
        return at;
}

template <typename... Ar> auto entry (Ar &&... args) { return actionTuple<Entry, Ar...> (std::forward<Ar> (args)...); }
template <typename... Ar> auto exit (Ar &&... args) { return actionTuple<Exit, Ar...> (std::forward<Ar> (args)...); }

template <typename C = int, typename T = int> class Transition {
public:
        explicit Transition (StateName sn) : stateName (sn) {}
        Transition (StateName sn, C c) : stateName (sn), condition (std::move (c)) {}
        Transition (StateName sn, C c, T t) : stateName (sn), condition (std::move (c)), actions (std::move (t)) {}

private:
        StateName stateName;
        C condition;
        T actions;
};

auto transition (StateName sn, auto cond, auto &&... acts)
{
        return Transition (sn, std::move (cond), hana::tuple (std::forward<decltype (acts)> (acts)...));
};

// template <typename... Ts> auto transitions (Ts &&... ts) { return hana::make_tuple (std::forward<Ts> (ts)...); }

template <typename T1 = void, typename T2 = void, typename T3 = int> class State {
public:
        State () = delete;
        explicit State (StateName sn) : name (sn) {}
        State (StateName sn, Entry<T1> en) : name (sn), entry (std::move (en)) {}
        State (StateName sn, Entry<T1> en, Exit<T2> ex) : name (sn), entry (std::move (en)), exit (std::move (ex)) {}
        State (StateName sn, Entry<T1> en, Exit<T2> ex, T3 ts)
            : name (sn), entry (std::move (en)), exit (std::move (ex)), transitions (std::move (ts))
        {
        }

        // private:
        StateName name;
        Entry<T1> entry;
        Exit<T2> exit;
        T3 transitions;
};

auto state (StateName sn, auto &&entry, auto &&exit, auto &&... trans)
{
        return State (sn, std::forward<decltype (entry)> (entry), std::forward<decltype (exit)> (exit),
                      hana::tuple (std::forward<decltype (trans)> (trans)...));
}

// TODO not here
class At {
public:
        At (gsl::czstring<> c) : cmd (c) {}
        void operator() () { std::cout << "usart <- " << cmd << std::endl; }

private:
        gsl::czstring<> cmd;
};

/// Event : string with operator ==
// TODO move from here
struct Eq {
        Eq (std::string const &t) : t (t) {}
        bool operator() (auto const &ev) const { return ev == t; }
        std::string t;
};

/**
 *
 */
template <typename S> class Machine {
public:
        explicit Machine (S s) : states{std::move (s)} {}

        template <typename Q> void run (Q &&queue);

private:
        S states;                               /// hana::tuple of States.
        std::optional<StateName> currentName{}; /// Current state name.
};

template <typename... Sts> auto machine (Sts &&... states) { return Machine (hana::make_tuple (std::forward<Sts> (states)...)); }

template <typename S> template <typename Q> void Machine<S>::run (Q && /*queue*/)
{
        hana::for_each (states, [](auto const &state) { std::cout << state.name << std::endl; });

        // Look for initial state if current is empty (std::optional is used).
        // If it was empty, run initial state's entry action

        // find transition

        // performTransition
        // - run curent.exit
        // - run transition.action
        // - change current name.
        // - run current.entry
}

} // namespace ls
