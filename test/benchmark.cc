/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "StateMachine.h"
// #include <deque>
#include <etl/deque.h>

int main ()
{
        using namespace ls;

        std::vector<std::string> results;

        auto res = [&results] (std::string const &message) { return [&results, message] { results.push_back (message); }; };
        auto eq = [] (std::string const &what) { return [what] (auto const &i) { return i == what; }; };

        auto m = machine<std::string> (
                state ("INIT"_STATE, entry (res ("INIT entry")), exit (res ("INIT exit")), transition ("B"_STATE, eq ("2"), res ("action"))),

                state ("B"_STATE, entry (res ("B entry")), exit (res ("B exit")),
                       transition ("INIT"_STATE, eq ("-3"), res ("action"), res ("another")),
                       transition ("C"_STATE, eq ("3"), res ("action"), res ("another"))),

                state ("C"_STATE, entry (res ("C entry")), exit (res ("C exit")),
                       transition ("B"_STATE, eq ("-4"), res ("action"), res ("another")),
                       transition ("D"_STATE, eq ("4"), res ("action"), res ("another"))),

                state ("D"_STATE, entry (res ("D entry")), exit (res ("D exit")),
                       transition ("C"_STATE, eq ("-5"), res ("action"), res ("another")),
                       transition ("E"_STATE, eq ("5"), res ("action"), res ("another"))),

                state ("E"_STATE, entry (res ("E entry")), exit (res ("E exit")),
                       transition ("D"_STATE, eq ("-6"), res ("action"), res ("another")),
                       transition ("F"_STATE, eq ("6"), res ("action"), res ("another"))),

                state ("F"_STATE, entry (res ("F entry")), exit (res ("F exit")),
                       transition ("E"_STATE, eq ("-7"), res ("action"), res ("another")),
                       transition ("G"_STATE, eq ("7"), res ("action"), res ("another"))),

                state ("G"_STATE, entry (res ("G entry")), exit (res ("G exit")),
                       transition ("F"_STATE, eq ("-8"), res ("action"), res ("another")),
                       transition ("H"_STATE, eq ("8"), res ("action"), res ("another"))),

                state ("H"_STATE, entry (res ("H entry")), exit (res ("H exit")),
                       transition ("G"_STATE, eq ("-9"), res ("action"), res ("another")),
                       transition ("I"_STATE, eq ("9"), res ("action"), res ("another"))),

                state ("I"_STATE, entry (res ("I entry")), exit (res ("I exit")),
                       transition ("H"_STATE, eq ("-10"), res ("action"), res ("another")),
                       transition ("J"_STATE, eq ("10"), res ("action"), res ("another"))),

                state ("J"_STATE, entry (res ("J entry")), exit (res ("J exit")),
                       transition ("I"_STATE, eq ("-11"), res ("action"), res ("another")),
                       transition ("K"_STATE, eq ("11"), res ("action"), res ("another"))),

                state ("K"_STATE, entry (res ("K entry")), exit (res ("K exit")),
                       transition ("J"_STATE, eq ("-12"), res ("action"), res ("another")),
                       transition ("L"_STATE, eq ("12"), res ("action"), res ("another"))),

                state ("L"_STATE, entry (res ("L entry")), exit (res ("L exit")),
                       transition ("K"_STATE, eq ("-13"), res ("action"), res ("another")),
                       transition ("M"_STATE, eq ("13"), res ("action"), res ("another"))),

                state ("M"_STATE, entry (res ("M entry")), exit (res ("M exit")),
                       transition ("L"_STATE, eq ("-14"), res ("action"), res ("another")),
                       transition ("N"_STATE, eq ("14"), res ("action"), res ("another"))),

                state ("N"_STATE, entry (res ("N entry")), exit (res ("N exit")),
                       transition ("M"_STATE, eq ("-15"), res ("action"), res ("another")),
                       transition ("O"_STATE, eq ("15"), res ("action"), res ("another"))),

                state ("O"_STATE, entry (res ("O entry")), exit (res ("O exit")),
                       transition ("N"_STATE, eq ("-16"), res ("action"), res ("another")),
                       transition ("FINAL"_STATE, eq ("16"), res ("action"), res ("another"))),

                state ("FINAL"_STATE, entry (res ("FINAL entry")), exit (res ("FINAL exit")))

        );

        //  std::deque<std::string> evq;
        etl::deque<std::string, 16> evq;
        m.run (evq); // _->INIT

        for (int i = 0; i < 10000; ++i) {
                evq.push_back ("2");
                m.run (evq); // INIT->B
                evq.push_back ("3");
                m.run (evq); // B->C
                evq.push_back ("4");
                m.run (evq);
                evq.push_back ("5");
                m.run (evq);
                evq.push_back ("6");
                m.run (evq);
                evq.push_back ("7");
                m.run (evq);
                evq.push_back ("8");
                m.run (evq);
                evq.push_back ("9");
                m.run (evq);
                evq.push_back ("10");
                m.run (evq);
                evq.push_back ("11");
                m.run (evq);
                evq.push_back ("12");
                m.run (evq);
                evq.push_back ("13");
                m.run (evq);
                evq.push_back ("14");
                m.run (evq);
                evq.push_back ("15");
                m.run (evq);

                evq.push_back ("-16");
                m.run (evq);
                evq.push_back ("-15");
                m.run (evq);
                evq.push_back ("-14");
                m.run (evq);
                evq.push_back ("-13");
                m.run (evq);
                evq.push_back ("-12");
                m.run (evq);
                evq.push_back ("-11");
                m.run (evq);
                evq.push_back ("-10");
                m.run (evq);
                evq.push_back ("-9");
                m.run (evq);
                evq.push_back ("-8");
                m.run (evq);
                evq.push_back ("-7");
                m.run (evq);
                evq.push_back ("-6");
                m.run (evq);
                evq.push_back ("-5");
                m.run (evq);
                evq.push_back ("-4");
                m.run (evq);
                evq.push_back ("-3");
                m.run (evq); // B->INIT

                assert (evq.empty ());
        }

        assert (results.size () == 1110001);
}
