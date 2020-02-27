/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "Machine.h"
#include <exception>
#include <string>
#include <vector>

#define DEBUG 0
#if DEBUG
#include <iostream>
#endif

using namespace std::string_literals;
using namespace ls;

/****************************************************************************/

int main ()
{
#if 1
        std::vector<std::string> results;

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
}

/*
-O3
    FILE SIZE        VM SIZE
 --------------  --------------
  43.8%  13.5Ki  73.0%  13.5Ki    .text
  18.5%  5.73Ki   0.0%       0    [Unmapped]
   8.4%  2.58Ki   0.0%       0    .strtab
   6.6%  2.03Ki   0.0%       0    .symtab
   3.3%  1.03Ki   1.4%     273    [15 Others]
   2.3%     720   3.5%     656    .eh_frame
   2.2%     689   3.6%     689    [LOAD #2 [R]]
   2.0%     633   3.0%     569    .dynstr
   1.9%     592   2.8%     528    .dynamic
   1.8%     568   2.7%     504    .dynsym
   1.4%     447   2.0%     383    .rodata
   1.3%     400   1.8%     336    .rela.plt
   1.1%     354   0.0%       0    .shstrtab
   1.0%     328   1.4%     264    .rela.dyn
   1.0%     304   1.3%     240    .plt
   0.9%     291   1.2%     227    .gcc_except_table
   0.7%     224   0.8%     160    .gnu.version_r
   0.6%     200   0.7%     136    .got.plt
   0.5%     156   0.5%      92    .eh_frame_hdr
   0.4%     128   0.0%       0    [ELF Headers]
   0.3%     106   0.2%      42    .gnu.version
 100.0%  30.9Ki 100.0%  18.4Ki    TOTAL



-O0 stripped
    FILE SIZE        VM SIZE
 --------------  --------------
  55.8%  83.9Ki  60.8%  83.9Ki    .text
  25.5%  38.3Ki  27.7%  38.2Ki    .eh_frame
   6.9%  10.4Ki   0.0%       0    [Unmapped]
   6.1%  9.24Ki   6.7%  9.18Ki    .eh_frame_hdr
   1.0%  1.49Ki   1.0%  1.43Ki    .dynstr
   0.6%     904   0.6%     840    .dynsym
   0.5%     829   0.5%     765    .gcc_except_table
   0.5%     771   0.1%     186    [12 Others]
   0.5%     712   0.5%     648    .rela.plt
   0.4%     688   0.5%     688    [LOAD #2 [R]]
   0.4%     592   0.4%     528    .dynamic
   0.3%     512   0.3%     448    .plt
   0.3%     490   0.3%     426    .rodata
   0.2%     325   0.0%       0    .shstrtab
   0.2%     304   0.2%     240    .got.plt
   0.2%     280   0.2%     216    .rela.dyn
   0.1%     208   0.1%     144    .gnu.version_r
   0.1%     134   0.0%      70    .gnu.version
   0.1%     128   0.0%       0    [ELF Headers]
   0.1%     104   0.0%      40    .got
   0.1%     100   0.0%      36    .gnu.hash
 100.0%   150Ki 100.0%   137Ki    TOTAL
*/
