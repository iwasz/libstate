/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include <cassert>
#include <tuple>
#include <vector>

class IState {
public:
        virtual ~IState () = default;
};

template <typename ActT> struct State {
        State (ActT e) : entry{std::move (e)} {}
        void operator() ();
        ActT entry;
        // int i;
};

template <typename ActT> void State<ActT>::operator() ()
{
        std::apply ([] (auto &... entryAction) { (entryAction (), ...); }, entry);
}

int main ()
{
        int results;

        auto res = [&results] (int message) { return [&results, message] { results += message; }; };
        auto eq = [] (int what) { return [what] (auto const &i) { return i == what; }; };

        auto tup = std::make_tuple (State (std::make_tuple (res (1), res (2))), State (std::make_tuple (res (3), res (4))),
                                    State (std::make_tuple (res (5), res (6))), State (std::make_tuple (res (7), res (8))),
                                    State (std::make_tuple (res (9), res (10))), State (std::make_tuple (res (11), res (12))));

        std::apply ([] (auto &... calb) { (calb (), ...); }, tup);
        assert (results == 6);
}

/*
    FILE SIZE        VM SIZE
 --------------  --------------
  60.7%  9.55Ki   0.0%       0    [Unmapped]
   8.9%  1.40Ki   0.0%       0    .symtab
   4.2%     684  26.7%     684    [LOAD #2 [R]]
   3.4%     545   0.0%       0    .strtab
   3.3%     528  18.1%     464    .dynamic
   2.8%     453  15.2%     389    .text
   1.9%     305   0.0%       0    .shstrtab
   1.6%     256   7.5%     192    .rela.dyn
   1.4%     232   6.5%     168    .eh_frame
   1.4%     228   6.4%     164    .dynstr
   1.3%     208   5.6%     144    .dynsym
   0.8%     128   0.0%       0    [ELF Headers]
   0.7%     108   1.7%      44    .eh_frame_hdr
   0.6%     104   1.6%      40    .got
   0.6%     100   1.4%      36    .note.gnu.build-id
   0.6%      96   1.2%      32    .gnu.version_r
   0.6%      96   1.2%      32    .note.ABI-tag
   0.6%      92   1.1%      28    .gnu.hash
   0.6%      92   1.1%      28    .interp
   0.6%      91   1.1%      27    .init
   0.5%      88   0.9%      24    .got.plt
   0.5%      81   0.0%       0    .comment
   0.5%      77   0.5%      13    .fini
   0.5%      76   0.5%      12    .gnu.version
   0.4%      72   0.3%       8    .data
   0.4%      72   0.3%       8    .fini_array
   0.4%      72   0.3%       8    .init_array
   0.0%       0   0.3%       8    .bss
   0.0%       8   0.3%       8    [LOAD #3 [RX]]
   0.0%       4   0.2%       4    [LOAD #4 [R]]
 100.0%  15.7Ki 100.0%  2.50Ki    TOTAL
*/
