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

        Machine2<std::string, HeapAllocator, 32> m;

        m.state ("INIT"_STT, entry (res ("INIT entry")), exit (res ("INIT exit")), m.transition ("B"_STT, eq ("2"), res ("action")));

        m.state ("B"_STT, entry (res ("B entry")), exit (res ("B exit")), m.transition ("INIT"_STT, eq ("-3"), res ("action"), res ("another")),
                 m.transition ("C"_STT, eq ("3"), res ("action"), res ("another")));

        m.state ("C"_STT, entry (res ("C entry")), exit (res ("C exit")), m.transition ("B"_STT, eq ("-4"), res ("action"), res ("another")),
                 m.transition ("D"_STT, eq ("4"), res ("action"), res ("another")));

        m.state ("D"_STT, entry (res ("D entry")), exit (res ("D exit")), m.transition ("C"_STT, eq ("-5"), res ("action"), res ("another")),
                 m.transition ("E"_STT, eq ("5"), res ("action"), res ("another")));

        m.state ("E"_STT, entry (res ("E entry")), exit (res ("E exit")), m.transition ("D"_STT, eq ("-6"), res ("action"), res ("another")),
                 m.transition ("F"_STT, eq ("6"), res ("action"), res ("another")));

        m.state ("F"_STT, entry (res ("F entry")), exit (res ("F exit")), m.transition ("E"_STT, eq ("-7"), res ("action"), res ("another")),
                 m.transition ("G"_STT, eq ("7"), res ("action"), res ("another")));

        m.state ("G"_STT, entry (res ("G entry")), exit (res ("G exit")), m.transition ("F"_STT, eq ("-8"), res ("action"), res ("another")),
                 m.transition ("H"_STT, eq ("8"), res ("action"), res ("another")));

        m.state ("H"_STT, entry (res ("H entry")), exit (res ("H exit")), m.transition ("G"_STT, eq ("-9"), res ("action"), res ("another")),
                 m.transition ("I"_STT, eq ("9"), res ("action"), res ("another")));

        m.state ("I"_STT, entry (res ("I entry")), exit (res ("I exit")), m.transition ("H"_STT, eq ("-10"), res ("action"), res ("another")),
                 m.transition ("J"_STT, eq ("10"), res ("action"), res ("another")));

        m.state ("J"_STT, entry (res ("J entry")), exit (res ("J exit")), m.transition ("I"_STT, eq ("-11"), res ("action"), res ("another")),
                 m.transition ("K"_STT, eq ("11"), res ("action"), res ("another")));

        m.state ("K"_STT, entry (res ("K entry")), exit (res ("K exit")), m.transition ("J"_STT, eq ("-12"), res ("action"), res ("another")),
                 m.transition ("L"_STT, eq ("12"), res ("action"), res ("another")));

        m.state ("L"_STT, entry (res ("L entry")), exit (res ("L exit")), m.transition ("K"_STT, eq ("-13"), res ("action"), res ("another")),
                 m.transition ("M"_STT, eq ("13"), res ("action"), res ("another")));

        m.state ("M"_STT, entry (res ("M entry")), exit (res ("M exit")), m.transition ("L"_STT, eq ("-14"), res ("action"), res ("another")),
                 m.transition ("N"_STT, eq ("14"), res ("action"), res ("another")));

        m.state ("N"_STT, entry (res ("N entry")), exit (res ("N exit")), m.transition ("M"_STT, eq ("-15"), res ("action"), res ("another")),
                 m.transition ("O"_STT, eq ("15"), res ("action"), res ("another")));

        m.state ("O"_STT, entry (res ("O entry")), exit (res ("O exit")), m.transition ("N"_STT, eq ("-16"), res ("action"), res ("another")),
                 m.transition ("FINAL"_STT, eq ("16"), res ("action"), res ("another")));

        m.state ("FINAL"_STT, entry (res ("FINAL entry")));

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

        // Ensures (results.size () == 1110001); This true when entering INIT state triggers entry actions when FSM is run for the first time
        Ensures (results.size () == 1110000);
}
