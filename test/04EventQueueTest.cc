/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "Machine.h"
#include "Utils.h"
#include "catch.hpp"
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace ls;
using Event = std::vector<std::string>;

/**
 * Simple machine where event has the form of a vector.
 */
TEST_CASE ("Instance", "[Event queue]")
{
        auto m = machine (state ("INIT"_STATE, entry (At ("INIT entry")), exit (At ("INIT exit")),
                                 transition (
                                         "B"_STATE, [] (auto const &e) { return e.size () == 1; }, At ("transition to B"))),

                          state ("B"_STATE, entry (At ("B entry")), exit (At ("B exit")),
                                 transition (
                                         "INIT"_STATE, [] (auto const & /* e */) { return false; }, At ("A"), At ("B")))

        );

        /*
         * We run the machine for the first time with two events in the queue. Both
         * events will be checked by the conditions. Because the transition of "INIT"_STATE
         * fires upon event '2', the state is changed.
         */
        m.run (Event{});
        REQUIRE (m.getCurrentStateIndex () == "INIT"_STATE.getIndex ());

        m.run (Event{"Ala"});
        REQUIRE (m.getCurrentStateIndex () == "B"_STATE.getIndex ());
}

/**
 *
 */
TEST_CASE ("Condition", "[Event queue]")
{
        /// Checks if event collection has certain size
        auto size = [] (size_t s) { return [s] (auto const &events) { return events.size () == s; }; };

        /**
         * Mind that there are conversions from const char * to std::string everywhere in following
         * lambdas.
         */

        /// Checks if event collection contains this particular value.
        auto has = [] (const char *val) {
                return [val] (auto const &events) { return std::find (events.cbegin (), events.cend (), val) != events.cend (); };
        };

        // Returns true if all values are present in the event colection, no matter the order
        auto all = [] (auto const &... val) {
                return [val...] (auto const &events) { return ((std::find (events.cbegin (), events.cend (), val) != events.cend ()) && ...); };
        };

        // Returns true if one of the values is present in the event collection.
        auto any = [] (auto const &... val) {
                return [val...] (auto const &events) { return ((std::find (events.cbegin (), events.cend (), val) != events.cend ()) || ...); };
        };

        // Returns true if all values are present in the event colection one after another (without any values in between).
        auto strict = [] (auto const &... val) {
                return [val...] (auto const &events) {
                        std::array a{val...};
                        return (std::search (events.cbegin (), events.cend (), a.cbegin (), a.cend ()) != events.cend ());
                };
        };

        // Returns true if all values are present in the event colection one after another (with possible values in between).
        auto seq = [] (auto const &... val) {
                return [val...] (auto const &events) {
                        std::array args{val...};
                        size_t currentArg{};

                        for (auto const &ev : events) {
                                if (ev == args.at (currentArg)) {
                                        if (++currentArg == args.size ()) {
                                                return true;
                                        }
                                }
                        }

                        return false;
                };
        };

        auto m = machine (
                state ("INIT"_STATE, entry (At ("INIT entry")), exit (At ("INIT exit")),
                       transition ("B"_STATE, size (1), At ("transition to B"))),

                state ("B"_STATE, entry (At ("B entry")), exit (At ("B exit")), transition ("C"_STATE, has ("Janka"), At ("transition to C"))),

                state ("C"_STATE, entry (At ("C entry")), exit (At ("C exit")),
                       transition ("D"_STATE, all ("psa", "ma", "Ala"), At ("transition to D"))),

                state ("D"_STATE, entry (At ("D entry")), exit (At ("D exit")),
                       transition ("E"_STATE, any ("kalosz", "parasol", "płaszcz"), At ("transition to E"))),

                state ("E"_STATE, entry (At ("E entry")), exit (At ("E exit")),
                       transition ("F"_STATE, strict ("kot", "ma", "psa"), At ("transition to F"))),

                state ("F"_STATE, entry (At ("F entry")), exit (At ("F exit")),
                       transition ("G"_STATE, seq ("kot", "ma", "psa"), At ("transition to G"))),

                state ("G"_STATE, entry (At ("G entry")), exit (At ("G exit")),
                       transition (
                               "Z"_STATE, [] (auto /* a */) { return true; }, At ("transition to Z"))),

                state ("Z"_STATE, entry (At ("Z entry")), exit (At ("Z exit")),
                       transition (
                               "INIT"_STATE, [] (auto const & /* e */) { return false; }, At ("transition to INIT"))));

        /*
         * We run the machine for the first time with two events in the queue. Both
         * events will be checked by the conditions. Because the transition of "INIT"_STATE
         * fires upon event '2', the state is changed.
         */
        m.run (Event{});
        REQUIRE (m.getCurrentStateIndex () == "INIT"_STATE.getIndex ());

        m.run (Event{"Ala"});
        REQUIRE (m.getCurrentStateIndex () == "B"_STATE.getIndex ());

        m.run (Event{"Ala", "kocha", "Janka"});
        REQUIRE (m.getCurrentStateIndex () == "C"_STATE.getIndex ());

        m.run (Event{"Ala", "ma", "kota", "i", "psa"});
        REQUIRE (m.getCurrentStateIndex () == "D"_STATE.getIndex ());

        m.run (Event{"Ala", "ma", "parasol", "i", "psa"});
        REQUIRE (m.getCurrentStateIndex () == "E"_STATE.getIndex ());

        /*--------------------------------------------------------------------------*/
        // E -> F
        // Negative check
        m.run (Event{"Stary", "psa", "ma", "parasol", "i", "kot"});
        // We stay in E
        REQUIRE (m.getCurrentStateIndex () == "E"_STATE.getIndex ());

        // Negative check 2
        m.run (Event{"Stary", "kot", "ma", "parasol", "i", "psa"});
        REQUIRE (m.getCurrentStateIndex () == "E"_STATE.getIndex ());

        m.run (Event{"Stary", "kot", "ma", "psa", "i", "parasol"});
        REQUIRE (m.getCurrentStateIndex () == "F"_STATE.getIndex ());

        /*--------------------------------------------------------------------------*/
        // F -> G
        // Negative
        REQUIRE (!m.run (Event{"Stary", "psa", "ma", "parasol", "i", "kot"}));
        REQUIRE (m.getCurrentStateIndex () == "F"_STATE.getIndex ());

        // Positive
        m.run (Event{"Stary", "kot", "ma", "parasol", "i", "psa"});
        REQUIRE (m.getCurrentStateIndex () == "G"_STATE.getIndex ());
}

template <typename ConA, typename ConB> auto operator&& (ConA conditionA, ConB conditionB)
{
        return [cA = std::move (conditionA), cB = std::move (conditionB)] (auto const &events) { return cA (events) && cB (events); };
}

template <typename ConA, typename ConB> auto operator|| (ConA conditionA, ConB conditionB)
{
        return [cA = std::move (conditionA), cB = std::move (conditionB)] (auto const &events) { return cA (events) || cB (events); };
}

template <typename ConA> auto operator! (ConA conditionA)
{
        return [cA = std::move (conditionA)] (auto const &events) { return !(cA (events)); };
}

/**
 *
 */
TEST_CASE ("AndOr", "[Event queue]")
{
        /// Checks if event collection has certain size
        auto size = [] (size_t s) { return [s] (auto const &events) { return events.size () == s; }; };

        /**
         * Mind that there are conversions from const char * to std::string everywhere in following
         * lambdas.
         */

        /// Checks if event collection contains this particular value.
        auto has = [] (const char *val) {
                return [val] (auto const &events) { return std::find (events.cbegin (), events.cend (), val) != events.cend (); };
        };

        auto m = machine (state ("INIT"_STATE, entry (At ("INIT entry")), exit (At ("INIT exit")),
                                 transition ("B"_STATE, size (1) && has ("Janek"), At ("transition to B"))),

                          state ("B"_STATE, entry (At ("B entry")), exit (At ("B exit")),
                                 transition ("C"_STATE, size (3) && has ("Ala") && has ("kota"), At ("transition to C"))),

                          state ("C"_STATE, entry (At ("C entry")), exit (At ("C exit")),
                                 transition ("D"_STATE, size (3) || has ("Ala"), At ("transition to C")))

        );

        /*
         * We run the machine for the first time with two events in the queue. Both
         * events will be checked by the conditions. Because the transition of "INIT"_STATE
         * fires upon event '2', the state is changed.
         */
        m.run (Event{});
        REQUIRE (m.getCurrentStateIndex () == "INIT"_STATE.getIndex ());

        /*--------------------------------------------------------------------------*/

        // Negative
        m.run (Event{"Ala"});
        REQUIRE (m.getCurrentStateIndex () != "B"_STATE.getIndex ());
        // Positive test
        m.run (Event{"Janek"});
        REQUIRE (m.getCurrentStateIndex () == "B"_STATE.getIndex ());

        /*--------------------------------------------------------------------------*/

        // Negative
        m.run (Event{"Ala", "ma", "pas"});
        REQUIRE (m.getCurrentStateIndex () != "C"_STATE.getIndex ());
        // Negative
        m.run (Event{"Ala", "ma", "kota", "psa"});
        REQUIRE (m.getCurrentStateIndex () != "C"_STATE.getIndex ());
        // Positive test
        m.run (Event{"Ala", "ma", "kota"});
        REQUIRE (m.getCurrentStateIndex () == "C"_STATE.getIndex ());

        /*--------------------------------------------------------------------------*/

        // Positive test
        m.run (Event{"Aaa", "Bbb", "Ccc"});
        REQUIRE (m.getCurrentStateIndex () == "D"_STATE.getIndex ());
}

enum class StripInput { DONT_STRIP, STRIP };

/**
 * Like condition (like in SQL), but without '_'. Only '%' works.
 */
template <typename EventT> bool like (EventT const &event, gsl::czstring<> conditionStr, StripInput stripInput = StripInput::STRIP)
{
        std::string_view condition{conditionStr};

        size_t ei = 0;
        size_t ci = 0;

        // Stripuj początek.
        if (stripInput == StripInput::STRIP) {
                while (ei < event.size () && std::isspace (event.at (ei))) {
                        ++ei;
                }
        }

        bool resume;
        do {
                resume = false;

                // Normalny tekst, zgadzają się.
                while (ei < event.size () && ci < condition.size () && (event.at (ei) == condition.at (ci))) {
                        ++ei;
                        ++ci;
                }

                // Wtem w "condition natrafiamy na '%'
                if (ci < condition.size () && condition.at (ci) == '%') {
                        resume = true;
                        ++ci;

                        if (ci >= condition.size ()) {
                                return true;
                        }

                        do {
                                while (ei < event.size () && ci < condition.size () && (event.at (ei) != condition.at (ci))) {
                                        ++ei;
                                }

                                // Skończyło się wejście.
                                if (ei >= event.size ()) {
                                        return false;
                                }

                                size_t ci1 = ci;
                                while (ei < event.size () && ci1 < condition.size () && condition.at (ci1) != '%' && condition.at (ci1) != '_'
                                       && (event.at (ei) == condition.at (ci1))) {
                                        ++ei;
                                        ++ci1;
                                }

                                if (ci1 < condition.size () && condition.at (ci1) != '%' && condition.at (ci1) != '_') {
                                        continue;
                                }

                                ci = ci1;
                                break;

                        } while (true);
                }

        } while (resume);

        return (ci == condition.size ()) && (ei == event.size ());
}

/**
 * For modems and GNSSes
 */
TEST_CASE ("LikeCondition", "[Event queue]")
{
        using namespace std::string_literals;
        REQUIRE (like ("Ala"s, "%la"));
        REQUIRE (like (" Ala"s, "%la"));

        /// Checks if event collection has certain size
        auto size = [] (size_t s) { return [s] (auto const &events) { return events.size () == s; }; };

        /**
         * Mind that there are conversions from const char * to std::string everywhere in following
         * lambdas.
         */

        /// Checks if event collection contains this particular value.
        auto has = [] (const char *val) {
                return [val] (auto const &events) {
                        return std::find_if (events.cbegin (), events.cend (), [val] (auto const &event) { return like (event, val); })
                                != events.cend ();
                };
        };

        // Returns true if all values are present in the event colection, no matter the order
        auto all = [] (auto const &... val) {
                return [val...] (auto const &events) {
                        return ((std::find_if (events.cbegin (), events.cend (), [val] (auto const &event) { return like (event, val); })
                                 != events.cend ())
                                && ...);
                };
        };

        // Returns true if one of the values is present in the event collection.
        auto any = [] (auto const &... val) {
                return [val...] (auto const &events) {
                        return ((std::find_if (events.cbegin (), events.cend (), [val] (auto const &event) { return like (event, val); })
                                 != events.cend ())
                                || ...);
                };
        };

        // Returns true if all values are present in the event colection one after another (with possible values in between).
        auto seq = [] (auto const &... val) {
                return [val...] (auto const &events) {
                        std::array args{val...};
                        size_t currentArg{};

                        for (auto const &ev : events) {
                                if (like (ev, args.at (currentArg))) {
                                        if (++currentArg == args.size ()) {
                                                return true;
                                        }
                                }
                        }

                        return false;
                };
        };

        auto m = machine (
                state ("INIT"_STATE, entry (At ("INIT entry")), exit (At ("INIT exit")),
                       transition ("B"_STATE, size (1), At ("transition to B"))),

                state ("B"_STATE, entry (At ("B entry")), exit (At ("B exit")), transition ("C"_STATE, has ("%och%"), At ("transition to C"))),

                state ("C"_STATE, entry (At ("C entry")), exit (At ("C exit")),
                       transition ("D"_STATE, all ("%sa", "ma", "A%a"), At ("transition to D"))),

                state ("D"_STATE, entry (At ("D entry")), exit (At ("D exit")),
                       transition ("E"_STATE, any ("ka%sz", "parasol", "p%cz"), At ("transition to E"))),

                state ("E"_STATE, entry (At ("E entry")), exit (At ("E exit")),
                       transition ("F"_STATE, seq ("k%", "ma", "psa"), At ("transition to F"))),

                state ("F"_STATE, entry (At ("F entry")), exit (At ("F exit")),
                       transition ("G"_STATE, seq ("kot", "ma", "p%"), At ("transition to G"))),

                state ("G"_STATE, entry (At ("G entry")), exit (At ("G exit")),
                       transition (
                               "Z"_STATE, [] (auto /* a */) { return true; }, At ("transition to Z"))),

                state ("Z"_STATE, entry (At ("Z entry")), exit (At ("Z exit")),
                       transition (
                               "INIT"_STATE, [] (auto const & /* e */) { return false; }, At ("transition to INIT"))));

        /*
         * We run the machine for the first time with two events in the queue. Both
         * events will be checked by the conditions. Because the transition of "INIT"_STATE
         * fires upon event '2', the state is changed.
         */
        m.run (Event{});
        REQUIRE (m.getCurrentStateIndex () == "INIT"_STATE.getIndex ());

        m.run (Event{"Ala"});
        REQUIRE (m.getCurrentStateIndex () == "B"_STATE.getIndex ());

        m.run (Event{"Ala", "kocha", "Janka"});
        REQUIRE (m.getCurrentStateIndex () == "C"_STATE.getIndex ());

        m.run (Event{"Ala", "ma", "kota", "i", "psa"});
        REQUIRE (m.getCurrentStateIndex () == "D"_STATE.getIndex ());

        m.run (Event{"Ala", "ma", "parasol", "i", "psa"});
        REQUIRE (m.getCurrentStateIndex () == "E"_STATE.getIndex ());

        /*--------------------------------------------------------------------------*/
        // E -> F
        // Negative check
        m.run (Event{"Stary", "psa", "ma", "parasol", "i", "kot"});
        // We stay in E
        REQUIRE (m.getCurrentStateIndex () == "E"_STATE.getIndex ());

        m.run (Event{"Stary", "kot", "ma", "psa", "i", "parasol"});
        REQUIRE (m.getCurrentStateIndex () == "F"_STATE.getIndex ());

        /*--------------------------------------------------------------------------*/
        // F -> G
        // Negative
        REQUIRE (!m.run (Event{"Stary", "psa", "ma", "parasol", "i", "kot"}));
        REQUIRE (m.getCurrentStateIndex () == "F"_STATE.getIndex ());

        REQUIRE (!m.run (Event{"kot"}));
        REQUIRE (m.getCurrentStateIndex () == "F"_STATE.getIndex ());

        // Positive
        m.run (Event{"Stary", "kot", "ma", "parasol", "i", "psa"});
        REQUIRE (m.getCurrentStateIndex () == "G"_STATE.getIndex ());
}