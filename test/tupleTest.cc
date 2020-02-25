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
  46.0%  14.1Ki  73.3%  14.1Ki    .text
  16.1%  4.97Ki   0.0%       0    [Unmapped]
   8.0%  2.45Ki   0.0%       0    .strtab
   6.7%  2.05Ki   0.0%       0    .symtab
   2.3%     720   3.3%     656    .eh_frame
   2.2%     691   3.5%     691    [LOAD #2 [R]]
   2.2%     685   3.2%     621    .dynstr
   1.9%     592   2.7%     528    .dynamic
   1.9%     592   2.7%     528    .dynsym
   1.4%     447   1.9%     383    .rodata
   1.3%     424   1.8%     360    .rela.plt
   1.1%     354   0.0%       0    .shstrtab
   1.0%     328   1.3%     264    .rela.dyn
   1.0%     320   1.3%     256    .plt
   0.9%     291   1.2%     227    .gcc_except_table
   0.8%     240   0.9%     176    .gnu.version_r
   0.7%     208   0.7%     144    .got.plt
   0.5%     156   0.5%      92    .eh_frame_hdr
   0.4%     128   0.0%       0    [ELF Headers]
   0.3%     108   0.2%      44    .gnu.version
   0.3%     104   0.2%      40    .got
   0.3%     100   0.2%      36    .note.gnu.build-id
   0.3%      96   0.2%      32    .note.ABI-tag
   0.3%      92   0.1%      28    .gnu.hash
   0.3%      92   0.1%      28    .interp
   0.3%      91   0.1%      27    .init
   0.3%      81   0.0%       0    .comment
   0.3%      80   0.1%      16    .data
   0.3%      80   0.1%      16    .data.rel.ro
   0.2%      77   0.1%      13    .fini
   0.2%      72   0.0%       8    .fini_array
   0.2%      72   0.0%       8    .init_array
   0.0%       0   0.0%       8    .bss
   0.0%       8   0.0%       8    [LOAD #3 [RX]]
   0.0%       5   0.0%       5    [LOAD #4 [R]]
 100.0%  30.8Ki 100.0%  19.2Ki    TOTAL


-O0 stripped
    FILE SIZE        VM SIZE
 --------------  --------------
  56.6%  82.8Ki  60.4%  82.8Ki    .text
  26.3%  38.4Ki  28.0%  38.3Ki    .eh_frame
   6.3%  9.26Ki   6.7%  9.20Ki    .eh_frame_hdr
   4.9%  7.22Ki   0.0%       0    [Unmapped]
   1.0%  1.50Ki   1.1%  1.44Ki    .dynstr
   0.6%     928   0.6%     864    .dynsym
   0.6%     839   0.6%     775    .gcc_except_table
   0.5%     736   0.5%     672    .rela.plt
   0.5%     688   0.5%     688    [LOAD #2 [R]]
   0.4%     592   0.4%     528    .dynamic
   0.4%     556   0.4%     492    .rodata
   0.4%     528   0.3%     464    .plt
   0.2%     325   0.0%       0    .shstrtab
   0.2%     312   0.2%     248    .got.plt
   0.2%     280   0.2%     216    .rela.dyn
   0.1%     208   0.1%     144    .gnu.version_r
   0.1%     136   0.1%      72    .gnu.version
   0.1%     128   0.0%       0    [ELF Headers]
   0.1%     104   0.0%      40    .got
   0.1%     100   0.0%      36    .gnu.hash
   0.1%     100   0.0%      36    .note.gnu.build-id
   0.1%      96   0.0%      32    .note.ABI-tag
   0.1%      92   0.0%      28    .interp
   0.1%      91   0.0%      27    .init
   0.1%      81   0.0%       0    .comment
   0.1%      80   0.0%      16    .data
   0.1%      77   0.0%      13    .fini
   0.0%      72   0.0%       8    .fini_array
   0.0%      72   0.0%       8    .init_array
   0.0%       0   0.0%       8    .bss
   0.0%       8   0.0%       8    [LOAD #3 [RX]]
 100.0%   146Ki 100.0%   137Ki    TOTAL


*/
