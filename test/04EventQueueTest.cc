/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "Machine.h"
#include "Utils.h"
#include <array>
#include <catch2/catch.hpp>
#include <cstdlib>
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
        auto m = machine (state ("INIT"_ST, entry (At ("INIT entry")), exit (At ("INIT exit")),
                                 transition (
                                         "B"_ST, [] (auto const &e) { return e.size () == 1; }, At ("transition to B"))),

                          state ("B"_ST, entry (At ("B entry")), exit (At ("B exit")),
                                 transition (
                                         "INIT"_ST, [] (auto const & /* e */) { return false; }, At ("A"), At ("B")))

        );

        /*
         * We run the machine for the first time with two events in the queue. Both
         * events will be checked by the conditions. Because the transition of "INIT"_ST
         * fires upon event '2', the state is changed.
         */
        m.run (Event{});
        REQUIRE (m.getCurrentStateIndex () == "INIT"_ST.getIndex ());

        m.run (Event{"Ala"});
        REQUIRE (m.getCurrentStateIndex () == "B"_ST.getIndex ());
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
        auto all = [] (auto const &...val) {
                return [val...] (auto const &events) { return ((std::find (events.cbegin (), events.cend (), val) != events.cend ()) && ...); };
        };

        // Returns true if one of the values is present in the event collection.
        auto any = [] (auto const &...val) {
                return [val...] (auto const &events) { return ((std::find (events.cbegin (), events.cend (), val) != events.cend ()) || ...); };
        };

        // Returns true if all values are present in the event colection one after another (without any values in between).
        auto strict = [] (auto const &...val) {
                return [val...] (auto const &events) {
                        std::array a{val...};
                        return (std::search (events.cbegin (), events.cend (), a.cbegin (), a.cend ()) != events.cend ());
                };
        };

        // Returns true if all values are present in the event colection one after another (with possible values in between).
        auto seq = [] (auto const &...val) {
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
                state ("INIT"_ST, entry (At ("INIT entry")), exit (At ("INIT exit")), transition ("B"_ST, size (1), At ("transition to B"))),

                state ("B"_ST, entry (At ("B entry")), exit (At ("B exit")), transition ("C"_ST, has ("Janka"), At ("transition to C"))),

                state ("C"_ST, entry (At ("C entry")), exit (At ("C exit")),
                       transition ("D"_ST, all ("psa", "ma", "Ala"), At ("transition to D"))),

                state ("D"_ST, entry (At ("D entry")), exit (At ("D exit")),
                       transition ("E"_ST, any ("kalosz", "parasol", "płaszcz"), At ("transition to E"))),

                state ("E"_ST, entry (At ("E entry")), exit (At ("E exit")),
                       transition ("F"_ST, strict ("kot", "ma", "psa"), At ("transition to F"))),

                state ("F"_ST, entry (At ("F entry")), exit (At ("F exit")),
                       transition ("G"_ST, seq ("idzie", "Grześ", "wieś"), At ("transition to G"))),

                state ("G"_ST, entry (At ("G entry")), exit (At ("G exit")),
                       transition (
                               "Z"_ST, [] (auto /* a */) { return false; }, At ("transition to Z")))

        );

        /*
         * We run the machine for the first time with two events in the queue. Both
         * events will be checked by the conditions. Because the transition of "INIT"_ST
         * fires upon event '2', the state is changed.
         */
        m.run (Event{});
        REQUIRE (m.getCurrentStateIndex () == "INIT"_ST.getIndex ());

        m.run (Event{"Ala"});
        REQUIRE (m.getCurrentStateIndex () == "B"_ST.getIndex ());

        m.run (Event{"Ala", "kocha", "Janka"});
        REQUIRE (m.getCurrentStateIndex () == "C"_ST.getIndex ());

        m.run (Event{"Ala", "ma", "kota", "i", "psa"});
        REQUIRE (m.getCurrentStateIndex () == "D"_ST.getIndex ());

        m.run (Event{"Ala", "ma", "parasol", "i", "psa"});
        REQUIRE (m.getCurrentStateIndex () == "E"_ST.getIndex ());

        /*--------------------------------------------------------------------------*/
        // E -> F
        // Negative check
        m.run (Event{"Stary", "psa", "ma", "parasol", "i", "kot"});
        // We stay in E
        REQUIRE (m.getCurrentStateIndex () == "E"_ST.getIndex ());

        // Negative check 2
        m.run (Event{"Stary", "kot", "ma", "parasol", "i", "psa"});
        REQUIRE (m.getCurrentStateIndex () == "E"_ST.getIndex ());

        m.run (Event{"Stary", "kot", "ma", "psa", "i", "parasol"});
        REQUIRE (m.getCurrentStateIndex () == "F"_ST.getIndex ());

        /*--------------------------------------------------------------------------*/
        // F -> G
        // Negative
        m.run (Event{"Grześ", "Idzie", "aaa", "przez", "wieś", "worek"});
        REQUIRE (m.getCurrentStateIndex () == "F"_ST.getIndex ());

        // Positive
        m.run (Event{"idzie", "aaa", "Grześ", "przez", "wieś", "worek"});
        REQUIRE (m.getCurrentStateIndex () == "G"_ST.getIndex ());
}

namespace op {
template <typename ConA, typename ConB> auto operator&& (ConA conditionA, ConB conditionB)
{
        return [cA = std::move (conditionA), cB = std::move (conditionB)] (auto const &events) { return cA (events) && cB (events); };
}

template <typename ConA, typename ConB> auto operator|| (ConA conditionA, ConB conditionB)
{
        return [cA = std::move (conditionA), cB = std::move (conditionB)] (auto const &events) { return cA (events) || cB (events); };
}

template <typename ConA> auto operator!(ConA conditionA)
{
        return [cA = std::move (conditionA)] (auto const &events) { return !(cA (events)); };
}
} // namespace op

/**
 *
 */
TEST_CASE ("AndOr", "[Event queue]")
{
        using namespace op;

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

        auto m = machine (state ("INIT"_ST, entry (At ("INIT entry")), exit (At ("INIT exit")),
                                 transition ("B"_ST, size (1) && has ("Janek"), At ("transition to B"))),

                          state ("B"_ST, entry (At ("B entry")), exit (At ("B exit")),
                                 transition ("C"_ST, size (3) && has ("Ala") && has ("kota"), At ("transition to C"))),

                          state ("C"_ST, entry (At ("C entry")), exit (At ("C exit")),
                                 transition ("D"_ST, size (4) || has ("Janek"), At ("transition to C")))

        );

        /*
         * We run the machine for the first time with two events in the queue. Both
         * events will be checked by the conditions. Because the transition of "INIT"_ST
         * fires upon event '2', the state is changed.
         */
        m.run (Event{});
        REQUIRE (m.getCurrentStateIndex () == "INIT"_ST.getIndex ());

        /*--------------------------------------------------------------------------*/

        // Negative
        m.run (Event{"Ala"});
        REQUIRE (m.getCurrentStateIndex () != "B"_ST.getIndex ());
        // Positive test
        m.run (Event{"Janek"});
        REQUIRE (m.getCurrentStateIndex () == "B"_ST.getIndex ());

        /*--------------------------------------------------------------------------*/

        // Negative
        m.run (Event{"Ala", "ma", "pas"});
        REQUIRE (m.getCurrentStateIndex () != "C"_ST.getIndex ());
        // Negative
        m.run (Event{"Ala", "ma", "kota", "psa"});
        REQUIRE (m.getCurrentStateIndex () != "C"_ST.getIndex ());
        // Positive test
        m.run (Event{"Ala", "ma", "kota"});
        REQUIRE (m.getCurrentStateIndex () == "C"_ST.getIndex ());

        /*--------------------------------------------------------------------------*/

        // Positive test
        m.run (Event{"Aaa", "Bbb", "Ccc", "ddd"});
        REQUIRE (m.getCurrentStateIndex () == "D"_ST.getIndex ());
}

template <typename OutputIterator, typename Parm, typename... Rst>
void doStore (OutputIterator const &begin, OutputIterator const &end, int paramNumber, int currentParamNumber, Parm &param, Rst &...rest)
{
        if (paramNumber == currentParamNumber) {
                if constexpr (std::is_same_v<Parm, int>) {
                        std::array<char, 16> tmp{};
                        auto dist = std::distance (begin, end);

                        if (dist > 0 && dist < int (tmp.size ())) {
                                auto it = std::copy (begin, end, tmp.begin ());
                                *it = '\0';
                                param = atoi (tmp.data ());
                        }
                }
                else {
                        param.clear ();
                        std::copy (begin, end, std::back_inserter (param));
                }
        }

        if constexpr (sizeof...(rest) > 0) {
                doStore (begin, end, paramNumber, currentParamNumber + 1, rest...);
        }
}

enum class StripInput { DONT_STRIP, STRIP };

/**
 * Like condition (like in SQL), but without '_'. Only '%' works.
 */
template <typename EventT, typename... Parm>
bool like (EventT const &event, const char *conditionStr, StripInput stripInput /* = StripInput::STRIP */, Parm &...params)
{
        std::string_view condition{conditionStr};

        size_t ei = 0;
        size_t ci = 0;
        size_t parseStart = 0;
        size_t parseEnd = 0;
        size_t cSize = condition.size ();
        size_t eSize = event.size ();
        int currentParsedString{};

        // Ltrim
        if (stripInput == StripInput::STRIP) {
                while (ei < eSize && std::isspace (event.at (ei))) {
                        ++ei;
                }
        }

        bool resume;
        do {
                resume = false;

                // Text matches letter by letter.
                while (ei < eSize && ci < cSize && (int (event.at (ei)) == int (condition.at (ci)))) {
                        ++ei;
                        ++ci;
                }

                parseStart = ei;

                // '%' character encountered
                if (ci < cSize && int (condition.at (ci)) == int ('%')) {
                        resume = true;
                        ++ci;

                        if (ci >= cSize) {
                                parseEnd = ei = eSize;
                                goto store;
                        }

                        do {
                                while (ei < eSize && ci < cSize && (int (event.at (ei)) != int (condition.at (ci)))) {
                                        ++ei;
                                }

                                parseEnd = ei;

                                // Skończyło się wejście.
                                if (ei >= eSize) {
                                        return false;
                                }

                                size_t ci1 = ci;
                                while (ei < eSize && ci1 < cSize && condition.at (ci1) != '%' && (event.at (ei) == condition.at (ci1))) {
                                        ++ei;
                                        ++ci1;
                                }

                                if (ci1 < cSize && condition.at (ci1) != '%') {
                                        continue;
                                }

                                ci = ci1;
                                break;

                        } while (true);
                }

        store:
                if constexpr (sizeof...(params) > 0) {
                        doStore (event.cbegin () + parseStart, event.cbegin () + parseEnd, currentParsedString++, 0, params...);
                }

        } while (resume);

        // Rtrim
        if (stripInput == StripInput::STRIP) {
                while (ei < eSize && std::isspace (event.at (ei))) {
                        ++ei;
                }
        }

        return (ci == cSize) && (ei == eSize);
}

/**
 * For modems and GNSSes
 */
TEST_CASE ("LikeFunction", "[Event queue]")
{
        using namespace std::string_literals;
        REQUIRE (like ("Ala"s, "%la", StripInput::STRIP));
        REQUIRE (like (" Ala"s, "%la", StripInput::STRIP));
        REQUIRE (like ("\r\nAla"s, "Ala", StripInput::STRIP));
        REQUIRE (like (" Ala"s, "Ala", StripInput::STRIP));
        REQUIRE (like (" Ala "s, "Ala", StripInput::STRIP));
        REQUIRE (like (" Ala \r\n"s, "Ala", StripInput::STRIP));
        REQUIRE (like (" Ala \r\n"s, "A%a", StripInput::STRIP));

        REQUIRE (like (">41 0C 00 40 "s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like ("\r>41 0C 00 40 "s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like ("\r\n>41 0C 00 40 "s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like (">41 0C 00 40\r"s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like (">41 0C 00 40\r\n"s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like ("\r\n>41 0C 00 40\r\n"s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like ("41 0C 00 40 "s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like ("\r41 0C 00 40 "s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like ("\r\n41 0C 00 40 "s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like ("41 0C 00 40\r"s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like ("41 0C 00 40\r\n"s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like ("\r\n41 0C 00 40\r\n"s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like (">41 0C 00 00 "s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like ("\r>41 0C 00 00 "s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like ("\r\n>41 0C 00 00 "s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like (">41 0C 00 00\r"s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like (">41 0C 00 00\r\n"s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like ("\r\n>41 0C 00 00\r\n"s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like ("41 0C 00 00 "s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like ("\r41 0C 00 00 "s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like ("\r\n41 0C 00 00 "s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like ("41 0C 00 00\r"s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like ("41 0C 00 00\r\n"s, "%41 0C%", StripInput::STRIP));
        REQUIRE (like ("\r\n41 0C 00 00\r\n"s, "%41 0C%", StripInput::STRIP));

        REQUIRE (like ("+CREG: 1,0"s, "+CREG: %,0%", StripInput::DONT_STRIP));
        REQUIRE (like ("+CREG: 0,0"s, "+CREG: %,0%", StripInput::DONT_STRIP));
        REQUIRE (like ("+CREG: 0,0\r\n"s, "+CREG: %,0%", StripInput::DONT_STRIP));

        REQUIRE (like ("99:bb:66|aa|77"s, "99:%:66|%|77", StripInput::DONT_STRIP));
        REQUIRE (like ("99:b:66|a|77"s, "99:%:66|%|77", StripInput::DONT_STRIP));
        REQUIRE (like ("99::66||77"s, "99:%:66|%|77", StripInput::DONT_STRIP));
        REQUIRE (like ("99:bb:77:66|aa|78|77"s, "99:%:66|%|77", StripInput::DONT_STRIP));
        REQUIRE (!like ("99:bb:77:66|aa|78|79"s, "99:%:66|%|77", StripInput::DONT_STRIP));

        REQUIRE (like ("+CSQ: 99:0"s, "+CSQ: 99:%", StripInput::DONT_STRIP));
        REQUIRE (like ("+CSQ: 99:2345"s, "+CSQ: 99:%", StripInput::DONT_STRIP));
        REQUIRE (like ("+CSQ: 99:"s, "+CSQ: 99:%", StripInput::DONT_STRIP));
        REQUIRE (like ("+CSQ: 99:JHGHJ:88"s, "+CSQ: 99:%", StripInput::DONT_STRIP));
        REQUIRE (like ("+CSQ: 99:99"s, "+CSQ: 99:%", StripInput::DONT_STRIP));
        REQUIRE (!like ("+CSQ: 98:0"s, "+CSQ: 99:%", StripInput::DONT_STRIP));
        REQUIRE (!like ("CSQ: 99:0"s, "+CSQ: 99:%", StripInput::DONT_STRIP));

        REQUIRE (like ("+CSQ: 99:0:66"s, "+CSQ: 99:%:66", StripInput::DONT_STRIP));
        REQUIRE (!like ("+CSQ: 99:0:666"s, "+CSQ: 99:%:66", StripInput::DONT_STRIP));
        REQUIRE (like ("+CSQ: 99:2345:66"s, "+CSQ: 99:%:66", StripInput::DONT_STRIP));
        REQUIRE (like ("+CSQ: 99::66"s, "+CSQ: 99:%:66", StripInput::DONT_STRIP));
        REQUIRE (like ("+CSQ: 99:JHGHJ:88:66"s, "+CSQ: 99:%:66", StripInput::DONT_STRIP));
        REQUIRE (like ("+CSQ: 99:99:66"s, "+CSQ: 99:%:66", StripInput::DONT_STRIP));
        REQUIRE (!like ("+CSQ: 99:99#66"s, "+CSQ: 99:%:66", StripInput::DONT_STRIP));
        REQUIRE (!like ("+CSQ: 98:0:66"s, "+CSQ: 99:%:66", StripInput::DONT_STRIP));
        REQUIRE (!like ("CSQ: 99:0:66"s, "+CSQ: 99:%:66", StripInput::DONT_STRIP));
        REQUIRE (!like ("+CSQ: 98:0"s, "+CSQ: 99:%:66", StripInput::DONT_STRIP));
        REQUIRE (!like ("CSQ: 99:0"s, "+CSQ: 99:%:66", StripInput::DONT_STRIP));
        REQUIRE (!like ("+CSQ: 98"s, "+CSQ: 99:%:66", StripInput::DONT_STRIP));
        REQUIRE (!like ("CSQ: 99"s, "+CSQ: 99:%:66", StripInput::DONT_STRIP));
        REQUIRE (!like (""s, "+CSQ: 99:%:66", StripInput::DONT_STRIP));
}

TEST_CASE ("Like parsing", "[Event queue]")
{
        using namespace std::string_literals;
        std::string s1, s2, s3;
        REQUIRE (like ("+QISACK:123"s, "+QISACK:%", StripInput::STRIP, s1));
        REQUIRE (s1 == "123"s);

        REQUIRE (like ("+QISACK:321,456"s, "+QISACK:%,%", StripInput::STRIP, s1, s2));
        REQUIRE (s1 == "321"s);
        REQUIRE (s2 == "456"s);

        REQUIRE (like ("+QISACK:321,456,555"s, "+QISACK:%,%,%", StripInput::STRIP, s1, s2));
        REQUIRE (s1 == "321"s);
        REQUIRE (s2 == "456"s);

        REQUIRE (like ("+QISACK:321,456,555"s, "+QISACK:%,%,%", StripInput::STRIP, s1, s2, s3));
        REQUIRE (s1 == "321"s);
        REQUIRE (s2 == "456"s);
        REQUIRE (s3 == "555"s);

        int a{}, b{}, c{};
        REQUIRE (like ("+QISACK:123"s, "+QISACK:%", StripInput::STRIP, a));
        REQUIRE (a == 123);

        REQUIRE (like ("+QISACK:321,456"s, "+QISACK:%,%", StripInput::STRIP, a, b));
        REQUIRE (a == 321);
        REQUIRE (b == 456);

        REQUIRE (like ("+QISACK:321,456,555"s, "+QISACK:%,%,%", StripInput::STRIP, a, b));
        REQUIRE (a == 321);
        REQUIRE (b == 456);

        REQUIRE (like ("+QISACK:321,456,555"s, "+QISACK:%,%,%", StripInput::STRIP, a, b, c));
        REQUIRE (a == 321);
        REQUIRE (b == 456);
        REQUIRE (c == 555);

        REQUIRE (like ("+QIRD: 216.58.207.46:80,TCP,1500"s, "+QIRD:%,TCP,%", StripInput::STRIP, a, b));
        // REQUIRE (a == 0);
        REQUIRE (b == 1500);

        REQUIRE (!like ("AT+QISACK\r\n"s, "+QISACK:%,%,%", StripInput::STRIP, a, b, c));
        REQUIRE (!like ("+QISACK*\r\n"s, "+QISACK:%,%,%", StripInput::STRIP, a, b, c));
        // REQUIRE (a == 0);
        // REQUIRE (b == 1500);
}

/**
 * For modems and GNSSes
 */
TEST_CASE ("LikeCondition", "[Event queue]")
{
        using namespace std::string_literals;

        /// Checks if event collection has certain size
        auto size = [] (size_t s) { return [s] (auto const &events) { return events.size () == s; }; };

        /**
         * Mind that there are conversions from const char * to std::string everywhere in following
         * lambdas.
         */

        /// Checks if event collection contains this particular value.
        auto has = [] (const char *val) {
                return [val] (auto const &events) {
                        return std::find_if (events.cbegin (), events.cend (),
                                             [val] (auto const &event) { return like (event, val, StripInput::STRIP); })
                                != events.cend ();
                };
        };

        // Returns true if all values are present in the event colection, no matter the order
        auto all = [] (auto const &...val) {
                return [val...] (auto const &events) {
                        auto find = [&events] (auto const &a) {
                                return std::find_if (events.cbegin (), events.cend (),
                                                     [&a] (auto const &event) { return like (event, a, StripInput::STRIP); })
                                        != events.cend ();
                        };

                        return (find (val) && ...);
                };
        };

        // Returns true if one of the values is present in the event collection.
        auto any = [] (auto const &...val) {
                return [val...] (auto const &events) {
                        auto find = [&events] (auto const &a) {
                                return std::find_if (events.cbegin (), events.cend (),
                                                     [&a] (auto const &event) { return like (event, a, StripInput::STRIP); })
                                        != events.cend ();
                        };

                        return (find (val) || ...);
                };
        };

        // Returns true if all values are present in the event colection one after another (with possible values in between).
        auto seq = [] (auto const &...val) {
                return [val...] (auto const &events) {
                        std::array args{val...};
                        size_t currentArg{};

                        for (auto const &ev : events) {
                                if (like (ev, args.at (currentArg), StripInput::STRIP)) {
                                        if (++currentArg == args.size ()) {
                                                return true;
                                        }
                                }
                        }

                        return false;
                };
        };

        auto consecutive = [] (auto const &...val) {
                return [val...] (auto const &events) {
                        std::array args{val...};
                        size_t currentArg{};

                        for (auto const &ev : events) {
                                if (like (ev, args.at (currentArg), StripInput::STRIP)) {
                                        if (++currentArg == args.size ()) {
                                                return true;
                                        }
                                }
                                else {
                                        currentArg = 0;
                                }
                        }

                        return false;
                };
        };

        REQUIRE (seq ("A", "B", "C") (Event{"A", "B", "C"}));
        REQUIRE (seq ("A", "B", "C") (Event{"X", "A", "B", "C"}));
        REQUIRE (seq ("A", "B", "C") (Event{"A", "B", "C", "X"}));
        REQUIRE (seq ("A", "B", "C") (Event{"A", "X", "B", "X", "C"}));
        REQUIRE (seq ("A", "B", "C") (Event{"X", "A", "X", "B", "X", "C", "X"}));
        REQUIRE (!seq ("A", "B", "C") (Event{"A", "C", "B"}));
        REQUIRE (seq ("A", "B", "C") (Event{"A", "A", "C", "B", "C"}));

        REQUIRE (consecutive ("A", "B", "C") (Event{"A", "B", "C"}));
        REQUIRE (consecutive ("A", "B", "C") (Event{"X", "A", "B", "C"}));
        REQUIRE (consecutive ("A", "B", "C") (Event{"A", "B", "C", "X"}));
        REQUIRE (!consecutive ("A", "B", "C") (Event{"A", "B", "X", "C"}));
        REQUIRE (!consecutive ("A", "B", "C") (Event{"A", "X", "B", "X", "C"}));
        REQUIRE (consecutive ("A", "B", "C") (Event{"X", "A", "B", "C", "X"}));
        REQUIRE (!consecutive ("A", "B", "C") (Event{"A", "C", "B"}));
        REQUIRE (consecutive ("A", "B", "C") (Event{"A", "B", "x", "A", "B", "C"}));

        auto m = machine (
                state ("INIT"_ST, entry (At ("INIT entry")), transition ("B"_ST, size (1), At ("transition to B"))),

                state ("B"_ST, entry (At ("B entry")), exit (At ("B exit")), transition ("C"_ST, has ("%och%"), At ("transition to C"))),

                state ("C"_ST, entry (At ("C entry")), exit (At ("C exit")),
                       transition ("D"_ST, all ("%sa", "ma", "A%a"), At ("transition to D"))),

                state ("D"_ST, entry (At ("D entry")), exit (At ("D exit")),
                       transition ("E"_ST, any ("ka%sz", "parasol", "p%cz"), At ("transition to E"))),

                state ("E"_ST, entry (At ("E entry")), exit (At ("E exit")),
                       transition ("F"_ST, seq ("k%", "ma", "psa"), At ("transition to F"))),

                state ("F"_ST, entry (At ("F entry")), exit (At ("F exit")),
                       transition ("G"_ST, seq ("kot", "ma", "p%"), At ("transition to G"))),

                state ("G"_ST, entry (At ("G entry")), exit (At ("G exit")),
                       transition (
                               "Z"_ST, [] (auto /* a */) { return false; }, At ("transition to Z"))),

                state ("Z"_ST, entry (At ("Z entry")), exit (At ("Z exit")),
                       transition (
                               "INIT"_ST, [] (auto const & /* e */) { return false; }, At ("transition to INIT"))));

        using namespace std::string_literals;

        /*
         * We run the machine for the first time with two events in the queue. Both
         * events will be checked by the conditions. Because the transition of "INIT"_ST
         * fires upon event '2', the state is changed.
         */
        REQUIRE (m.getCurrentStateIndex () == "INIT"_ST.getIndex ());
        REQUIRE (std::string{m.getCurrentStateName ()} == "INIT"s);

        // Fires entry actions, does not transition anywhere, because conditions are not satified
        m.run (Event{});
        REQUIRE (m.getCurrentStateIndex () == "INIT"_ST.getIndex ());
        REQUIRE (m.getCurrentStateName () == "INIT"s);

        m.run (Event{"Ala"});
        REQUIRE (m.getCurrentStateIndex () == "B"_ST.getIndex ());
        REQUIRE (m.getCurrentStateName () == "B"s);

        m.run (Event{"Ala", "kocha", "Janka"});
        REQUIRE (m.getCurrentStateIndex () == "C"_ST.getIndex ());

        m.run (Event{"Ala", "ma", "kota", "i", "psa"});
        REQUIRE (m.getCurrentStateIndex () == "D"_ST.getIndex ());

        m.run (Event{"Ala", "ma", "parasol", "i", "psa"});
        REQUIRE (m.getCurrentStateIndex () == "E"_ST.getIndex ());

        /*--------------------------------------------------------------------------*/
        // E -> F
        // Negative check
        m.run (Event{"Stary", "psa", "ma", "parasol", "i", "kot"});
        // We stay in E
        REQUIRE (m.getCurrentStateIndex () == "E"_ST.getIndex ());

        m.run (Event{"Stary", "kot", "ma", "psa", "i", "parasol"});
        REQUIRE (m.getCurrentStateIndex () == "F"_ST.getIndex ());

        /*--------------------------------------------------------------------------*/
        // F -> G
        // Negative
        REQUIRE (!m.run (Event{"Stary", "psa", "ma", "parasol", "i", "kot"}));
        REQUIRE (m.getCurrentStateIndex () == "F"_ST.getIndex ());

        REQUIRE (!m.run (Event{"kot"}));
        REQUIRE (m.getCurrentStateIndex () == "F"_ST.getIndex ());

        // Positive
        m.run (Event{"Stary", "kot", "ma", "parasol", "i", "psa"});
        REQUIRE (m.getCurrentStateIndex () == "G"_ST.getIndex ());
}