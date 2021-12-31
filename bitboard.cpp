#include "types.h"
#include "bitboard.h"

#include <sstream>
#include <iostream>
#include <vector>

namespace BB {

Bitboard Square[Nsquares];
Bitboard Attacks[Ncolors][Nsquares];
Bitboard Defenders[Ncolors][Nsquares];
Bitboard Forward[Ncolors][Nsquares];
Bitboard Span[Ncolors][Nsquares];

void init() {

    for (enum Square s = Square::a1; s < Square::Nb; ++s) {
        // BB::Square
        BB::Square[to_integral(s)] = (1ULL << to_integral(s));

        // BB::Attacks
        BB::Attacks[to_integral(Color::white)][to_integral(s)] = attacks<Color::white>(square_bb(s));
        BB::Attacks[to_integral(Color::black)][to_integral(s)] = attacks<Color::black>(square_bb(s));

        BB::Defenders[to_integral(Color::white)][to_integral(s)] = attacks<Color::black>(square_bb(s));
        BB::Defenders[to_integral(Color::black)][to_integral(s)] = attacks<Color::white>(square_bb(s));

        // BB::Forward
        BB::Forward[to_integral(Color::white)][to_integral(s)] = forward<Color::white>(square_bb(s));
        BB::Forward[to_integral(Color::black)][to_integral(s)] = forward<Color::black>(square_bb(s));
    }

    Color c = Color::white;
    for (enum Square sq = Square::a8; sq < Square::Nb; ++sq) {
        BB::Span[to_integral(c)][to_integral(sq)] = square_bb(sq);
    }
    for (int r = 6; r >= 0; --r) {
        enum Square sq = square_at(Column::a, Row(r));
        BB::Span[to_integral(c)][to_integral(sq)] = (square_bb(sq)
                                                     | span_bb(c, sq + Direction::up)
                                                     | span_bb(c, sq + Direction::up_right));
        ++sq;
        for (; column_of(sq) < Column::h; ++sq) {
            BB::Span[to_integral(c)][to_integral(sq)] = (square_bb(sq)
                                                         | span_bb(c, sq + Direction::up_left)
                                                         | span_bb(c, sq + Direction::up)
                                                         | span_bb(c, sq + Direction::up_right));
        }
        BB::Span[to_integral(c)][to_integral(sq)] = (square_bb(sq)
                                                     | span_bb(c, sq + Direction::up_left)
                                                     | span_bb(c, sq + Direction::up));
    }
    c = Color::black;
    for (enum Square sq = Square::a1; sq < Square::Nb; ++sq) {
        BB::Span[to_integral(c)][to_integral(sq)] = flip_vertical(span_bb(Color::white, relative(c, sq)));
    }
}

void view(std::ostream& out, Bitboard bb, std::string_view bb_name) {
    bool in_bb[Nsquares];
    std::fill(std::begin(in_bb), std::end(in_bb), false);

    while (bb) {
        enum Square sq = pop_lsb(bb);
        in_bb[to_integral(sq)] = true;
    }

    out << "\n     " << bb_name << ":\n";

    for (int r = to_integral(Row::eight); r >= 0; --r) {
        out << r + 1 << "  ";
        for (enum Square sq = square_at(Column::a, Row(r)); sq <= square_at(Column::h, Row(r)); ++sq) {
            out << (in_bb[to_integral(sq)] ? " X " : " . ");
        }
        out << '\n';
    }
    out << "   ";
    for (int c = 0; c < width; ++c) {
        char ch = c + 97;
        out << ' ' << ch << ' ';
    }
    out << '\n' << std::endl;
}

}  // namespace BB
