#include "types.h"
#include "bitboard.h"

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

        // Initial BB::Span
        BB::Span[to_integral(Color::white)][to_integral(s)] = attacks<Color::white>(square_bb(s)) | forward<Color::white>(square_bb(s));
        BB::Span[to_integral(Color::white)][to_integral(s)] = attacks<Color::black>(square_bb(s)) | forward<Color::black>(square_bb(s));

        // Full BB::Span
        for (int d = 2; d < 7 - to_integral(row_of(s)); ++d) {
            BB::Span[to_integral(Color::white)][to_integral(s)] = forward_span_bb<Color::white>(BB::Span[to_integral(Color::white)][to_integral(s)]);
            BB::Span[to_integral(Color::white)][to_integral(s)] = forward_span_bb<Color::black>(BB::Span[to_integral(Color::black)][to_integral(s)]);
        }
    }
}

}  // namespace BB
