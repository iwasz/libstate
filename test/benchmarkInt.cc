/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "StateMachine.h"
#include <deque>

int main ()
{
        using namespace ls;

        int results{};

        auto res = [&results] () { return [&results] { ++results; }; };
        auto eq = [] (int what) { return [what] (auto const &i) { return i == what; }; };

        auto m = machine<int> (state ("INIT"_STATE, entry (res ()), exit (res ()), transition ("B"_STATE, eq (2), res ())),

                               state ("B"_STATE, entry (res ()), exit (res ()), transition ("INIT"_STATE, eq (-3), res (), res ()),
                                      transition ("C"_STATE, eq (3), res (), res ())),

                               state ("C"_STATE, entry (res ()), exit (res ()), transition ("B"_STATE, eq (-4), res (), res ()),
                                      transition ("D"_STATE, eq (4), res (), res ())),

                               state ("D"_STATE, entry (res ()), exit (res ()), transition ("C"_STATE, eq (-5), res (), res ()),
                                      transition ("E"_STATE, eq (5), res (), res ())),

                               state ("E"_STATE, entry (res ()), exit (res ()), transition ("D"_STATE, eq (-6), res (), res ()),
                                      transition ("F"_STATE, eq (6), res (), res ())),

                               state ("F"_STATE, entry (res ()), exit (res ()), transition ("E"_STATE, eq (-7), res (), res ()),
                                      transition ("G"_STATE, eq (7), res (), res ())),

                               state ("G"_STATE, entry (res ()), exit (res ()), transition ("F"_STATE, eq (-8), res (), res ()),
                                      transition ("H"_STATE, eq (8), res (), res ())),

                               state ("H"_STATE, entry (res ()), exit (res ()), transition ("G"_STATE, eq (-9), res (), res ()),
                                      transition ("I"_STATE, eq (9), res (), res ())),

                               state ("I"_STATE, entry (res ()), exit (res ()), transition ("H"_STATE, eq (-10), res (), res ()),
                                      transition ("J"_STATE, eq (10), res (), res ())),

                               state ("J"_STATE, entry (res ()), exit (res ()), transition ("I"_STATE, eq (-11), res (), res ()),
                                      transition ("K"_STATE, eq (11), res (), res ())),

                               state ("K"_STATE, entry (res ()), exit (res ()), transition ("J"_STATE, eq (-12), res (), res ()),
                                      transition ("L"_STATE, eq (12), res (), res ())),

                               state ("L"_STATE, entry (res ()), exit (res ()), transition ("K"_STATE, eq (-13), res (), res ()),
                                      transition ("M"_STATE, eq (13), res (), res ())),

                               state ("M"_STATE, entry (res ()), exit (res ()), transition ("L"_STATE, eq (-14), res (), res ()),
                                      transition ("N"_STATE, eq (14), res (), res ())),

                               state ("N"_STATE, entry (res ()), exit (res ()), transition ("M"_STATE, eq (-15), res (), res ()),
                                      transition ("O"_STATE, eq (15), res (), res ())),

                               state ("O"_STATE, entry (res ()), exit (res ()), transition ("N"_STATE, eq (-16), res (), res ()),
                                      transition ("FINAL"_STATE, eq (16), res (), res ())),

                               state ("FINAL"_STATE, entry (res ()), exit (res ()))

        );

        std::deque<int> evq;
        m.run (evq); // _->INIT

        for (int i = 0; i < 10000; ++i) {
                evq.push_back (2);
                m.run (evq); // INIT->B
                evq.push_back (3);
                m.run (evq); // B->C
                evq.push_back (4);
                m.run (evq);
                evq.push_back (5);
                m.run (evq);
                evq.push_back (6);
                m.run (evq);
                evq.push_back (7);
                m.run (evq);
                evq.push_back (8);
                m.run (evq);
                evq.push_back (9);
                m.run (evq);
                evq.push_back (10);
                m.run (evq);
                evq.push_back (11);
                m.run (evq);
                evq.push_back (12);
                m.run (evq);
                evq.push_back (13);
                m.run (evq);
                evq.push_back (14);
                m.run (evq);
                evq.push_back (15);
                m.run (evq);

                evq.push_back (-16);
                m.run (evq);
                evq.push_back (-15);
                m.run (evq);
                evq.push_back (-14);
                m.run (evq);
                evq.push_back (-13);
                m.run (evq);
                evq.push_back (-12);
                m.run (evq);
                evq.push_back (-11);
                m.run (evq);
                evq.push_back (-10);
                m.run (evq);
                evq.push_back (-9);
                m.run (evq);
                evq.push_back (-8);
                m.run (evq);
                evq.push_back (-7);
                m.run (evq);
                evq.push_back (-6);
                m.run (evq);
                evq.push_back (-5);
                m.run (evq);
                evq.push_back (-4);
                m.run (evq);
                evq.push_back (-3);
                m.run (evq); // B->INIT

                assert (evq.empty ());
        }

        assert (results == 1110001);
}
