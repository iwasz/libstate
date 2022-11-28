/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "Machine.h"
#include <etl/deque.h>

int main ()
{
        using namespace ls;

        std::vector<std::string> results;

        auto res = [&results] (std::string const &message) { return [&results, message] { results.push_back (message); }; };
        auto eq = [] (std::string const &what) { return [what] (auto const &i) { return i == what; }; };

        Machine<std::string, HeapAllocator, 32> m;

        m.state ("INIT"_ST, entry (res ("INIT entry")), exit (res ("INIT exit")), m.transition ("B"_ST, eq ("2"), res ("action")));

        m.state ("B"_ST, entry (res ("B entry")), exit (res ("B exit")), m.transition ("INIT"_ST, eq ("-3"), res ("action"), res ("another")),
                 m.transition ("C"_ST, eq ("3"), res ("action"), res ("another")));

        m.state ("C"_ST, entry (res ("C entry")), exit (res ("C exit")), m.transition ("B"_ST, eq ("-4"), res ("action"), res ("another")),
                 m.transition ("D"_ST, eq ("4"), res ("action"), res ("another")));

        m.state ("D"_ST, entry (res ("D entry")), exit (res ("D exit")), m.transition ("C"_ST, eq ("-5"), res ("action"), res ("another")),
                 m.transition ("E"_ST, eq ("5"), res ("action"), res ("another")));

        m.state ("E"_ST, entry (res ("E entry")), exit (res ("E exit")), m.transition ("D"_ST, eq ("-6"), res ("action"), res ("another")),
                 m.transition ("F"_ST, eq ("6"), res ("action"), res ("another")));

        m.state ("F"_ST, entry (res ("F entry")), exit (res ("F exit")), m.transition ("E"_ST, eq ("-7"), res ("action"), res ("another")),
                 m.transition ("G"_ST, eq ("7"), res ("action"), res ("another")));

        m.state ("G"_ST, entry (res ("G entry")), exit (res ("G exit")), m.transition ("F"_ST, eq ("-8"), res ("action"), res ("another")),
                 m.transition ("H"_ST, eq ("8"), res ("action"), res ("another")));

        m.state ("H"_ST, entry (res ("H entry")), exit (res ("H exit")), m.transition ("G"_ST, eq ("-9"), res ("action"), res ("another")),
                 m.transition ("I"_ST, eq ("9"), res ("action"), res ("another")));

        m.state ("I"_ST, entry (res ("I entry")), exit (res ("I exit")), m.transition ("H"_ST, eq ("-10"), res ("action"), res ("another")),
                 m.transition ("J"_ST, eq ("10"), res ("action"), res ("another")));

        m.state ("J"_ST, entry (res ("J entry")), exit (res ("J exit")), m.transition ("I"_ST, eq ("-11"), res ("action"), res ("another")),
                 m.transition ("K"_ST, eq ("11"), res ("action"), res ("another")));

        m.state ("K"_ST, entry (res ("K entry")), exit (res ("K exit")), m.transition ("J"_ST, eq ("-12"), res ("action"), res ("another")),
                 m.transition ("L"_ST, eq ("12"), res ("action"), res ("another")));

        m.state ("L"_ST, entry (res ("L entry")), exit (res ("L exit")), m.transition ("K"_ST, eq ("-13"), res ("action"), res ("another")),
                 m.transition ("M"_ST, eq ("13"), res ("action"), res ("another")));

        m.state ("M"_ST, entry (res ("M entry")), exit (res ("M exit")), m.transition ("L"_ST, eq ("-14"), res ("action"), res ("another")),
                 m.transition ("N"_ST, eq ("14"), res ("action"), res ("another")));

        m.state ("N"_ST, entry (res ("N entry")), exit (res ("N exit")), m.transition ("M"_ST, eq ("-15"), res ("action"), res ("another")),
                 m.transition ("O"_ST, eq ("15"), res ("action"), res ("another")));

        m.state ("O"_ST, entry (res ("O entry")), exit (res ("O exit")), m.transition ("N"_ST, eq ("-16"), res ("action"), res ("another")),
                 m.transition ("FINAL"_ST, eq ("16"), res ("action"), res ("another")));

        m.state ("FINAL"_ST, entry (res ("FINAL entry")));

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
