/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "StateMachine.h"
#include <etl/deque.h>

int main ()
{
        using namespace ls;

        std::vector<std::string> results;

        auto res = [&results] (std::string const &message) { return [&results, message] { results.push_back (message); }; };
        auto eq = [] (std::string const &what) { return [what] (auto const &i) { return i == what; }; };

        Machine2<std::string, HeapAllocator, 32> m;

        m.state ("INIT"_STATE, entry (res ("INIT entry")), exit (res ("INIT exit")), m.transition ("B"_STATE, eq ("2"), res ("action")));

        m.state ("B"_STATE, entry (res ("B entry")), exit (res ("B exit")),
                 m.transition ("INIT"_STATE, eq ("-3"), res ("action"), res ("another")),
                 m.transition ("C"_STATE, eq ("3"), res ("action"), res ("another")));

        m.state ("C"_STATE, entry (res ("C entry")), exit (res ("C exit")), m.transition ("B"_STATE, eq ("-4"), res ("action"), res ("another")),
                 m.transition ("D"_STATE, eq ("4"), res ("action"), res ("another")));

        m.state ("D"_STATE, entry (res ("D entry")), exit (res ("D exit")), m.transition ("C"_STATE, eq ("-5"), res ("action"), res ("another")),
                 m.transition ("E"_STATE, eq ("5"), res ("action"), res ("another")));

        m.state ("E"_STATE, entry (res ("E entry")), exit (res ("E exit")), m.transition ("D"_STATE, eq ("-6"), res ("action"), res ("another")),
                 m.transition ("F"_STATE, eq ("6"), res ("action"), res ("another")));

        m.state ("F"_STATE, entry (res ("F entry")), exit (res ("F exit")), m.transition ("E"_STATE, eq ("-7"), res ("action"), res ("another")),
                 m.transition ("G"_STATE, eq ("7"), res ("action"), res ("another")));

        m.state ("G"_STATE, entry (res ("G entry")), exit (res ("G exit")), m.transition ("F"_STATE, eq ("-8"), res ("action"), res ("another")),
                 m.transition ("H"_STATE, eq ("8"), res ("action"), res ("another")));

        m.state ("H"_STATE, entry (res ("H entry")), exit (res ("H exit")), m.transition ("G"_STATE, eq ("-9"), res ("action"), res ("another")),
                 m.transition ("I"_STATE, eq ("9"), res ("action"), res ("another")));

        m.state ("I"_STATE, entry (res ("I entry")), exit (res ("I exit")),
                 m.transition ("H"_STATE, eq ("-10"), res ("action"), res ("another")),
                 m.transition ("J"_STATE, eq ("10"), res ("action"), res ("another")));

        m.state ("J"_STATE, entry (res ("J entry")), exit (res ("J exit")),
                 m.transition ("I"_STATE, eq ("-11"), res ("action"), res ("another")),
                 m.transition ("K"_STATE, eq ("11"), res ("action"), res ("another")));

        m.state ("K"_STATE, entry (res ("K entry")), exit (res ("K exit")),
                 m.transition ("J"_STATE, eq ("-12"), res ("action"), res ("another")),
                 m.transition ("L"_STATE, eq ("12"), res ("action"), res ("another")));

        m.state ("L"_STATE, entry (res ("L entry")), exit (res ("L exit")),
                 m.transition ("K"_STATE, eq ("-13"), res ("action"), res ("another")),
                 m.transition ("M"_STATE, eq ("13"), res ("action"), res ("another")));

        m.state ("M"_STATE, entry (res ("M entry")), exit (res ("M exit")),
                 m.transition ("L"_STATE, eq ("-14"), res ("action"), res ("another")),
                 m.transition ("N"_STATE, eq ("14"), res ("action"), res ("another")));

        m.state ("N"_STATE, entry (res ("N entry")), exit (res ("N exit")),
                 m.transition ("M"_STATE, eq ("-15"), res ("action"), res ("another")),
                 m.transition ("O"_STATE, eq ("15"), res ("action"), res ("another")));

        m.state ("O"_STATE, entry (res ("O entry")), exit (res ("O exit")),
                 m.transition ("N"_STATE, eq ("-16"), res ("action"), res ("another")),
                 m.transition ("FINAL"_STATE, eq ("16"), res ("action"), res ("another")));

        m.state ("FINAL"_STATE, entry (res ("FINAL entry")));

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

        // Ensures (results.size () == 1110001); This true when entering INIT state triggers entry actions when FSM is run for the first time
        Ensures (results.size () == 1110000);
}
