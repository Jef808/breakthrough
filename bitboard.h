#ifndef BITBOARD_H_
#define BITBOARD_H_

#include "types.h"

#include <cstdint>


using Bitboard = uint64_t;

namespace BB {

void init();

// Starting with LSB, every 8th bit is on. i.e. the first bit of each byte:
constexpr Bitboard ColA = 0x0101010101010101;
constexpr Bitboard ColB = ColA << 1;
constexpr Bitboard ColC = ColA << 2;
constexpr Bitboard ColD = ColA << 3;
constexpr Bitboard ColE = ColA << 4;
constexpr Bitboard ColF = ColA << 5;
constexpr Bitboard ColG = ColA << 6;
constexpr Bitboard ColH = ColA << 7;

// Each one of the first eight bit is on,
constexpr Bitboard Row1 = 0xFF;
constexpr Bitboard Row2 = Row1 << (1 * 8);
constexpr Bitboard Row3 = Row1 << (2 * 8);
constexpr Bitboard Row4 = Row1 << (3 * 8);
constexpr Bitboard Row5 = Row1 << (4 * 8);
constexpr Bitboard Row6 = Row1 << (5 * 8);
constexpr Bitboard Row7 = Row1 << (6 * 8);
constexpr Bitboard Row8 = Row1 << (7 * 8);

constexpr Bitboard QueenSide   = ColA | ColB | ColC | ColD;
constexpr Bitboard KingSide    = ColE | ColF | ColG | ColH;
constexpr Bitboard Center      = (ColD| ColE) & (Row4 | Row5);

extern Bitboard Square[Nsquares];
extern Bitboard Attacks[Ncolors][Nsquares];
extern Bitboard Forward[Ncolors][Nsquares];
extern Bitboard Defenders[Ncolors][Nsquares];
extern Bitboard Span[Ncolors][Nsquares];

} // namespace BB

inline Bitboard square_bb(Square sq) {
    return BB::Square[to_integral(sq)];
}
constexpr Bitboard col_bb(Column c) {
    return BB::ColA << to_integral(c);
}
constexpr Bitboard col_bb(Square s) {
    return BB::ColA << col_bb(column_of(s));
}
constexpr Bitboard row_bb(Row r) {
    return BB::Row1 << (8 * to_integral(r));
}
constexpr Bitboard row_bb(Square s) {
    return row_bb(row_of(s));
}

template<Direction D>
constexpr Bitboard shift(Bitboard b) {
    return D == Direction::up        ? b << 8               : D == Direction::down       ? b >> 8
         : D == Direction::left      ? (b & ~BB::ColA) >> 1 : D == Direction::right      ? (b & ~BB::ColH) << 1
         : D == Direction::up_left   ? (b & ~BB::ColA) << 7 : D == Direction::up_right   ? (b & ~BB::ColH) << 9
         : D == Direction::down_left ? (b & ~BB::ColA) >> 9 : D == Direction::down_right ? (b & ~BB::ColH) >> 7
         : 0;
}

template<Color C>
constexpr Bitboard attacks(Bitboard b) {
    return C == Color::white
        ? shift<Direction::up_left>(b)   | shift<Direction::up_right>(b)
        : shift<Direction::down_left>(b) | shift<Direction::down_right>(b);
}

template<Color C>
constexpr Bitboard forward(Bitboard b) {
    return C == Color::white ? shift<Direction::up>(b) : shift<Direction::down>(b);
}

inline Bitboard attacks_bb(Color c, Square sq) {
    return c == Color::white ? attacks<Color::white>(square_bb(sq)) : attacks<Color::black>(square_bb(sq));
}

inline Bitboard forward_bb(Color c, Square sq) {
    return c == Color::white ? forward<Color::white>(square_bb(sq)) : forward<Color::black>(square_bb(sq));
}

constexpr Bitboard forward_row_bb(Color c, Square s) {
    return c == Color::white
        ? ~BB::Row1 << 8 * to_integral(relative(Color::white, row_of(s)))
        : ~BB::Row1 >> 8 * to_integral(relative(Color::black, row_of(s)));
}

constexpr Bitboard adjacent_cols_bb(Square s) {
    return shift<Direction::left>(col_bb(s)) | shift<Direction::right>(col_bb(s));
}

template<Color C>
constexpr Bitboard forward_span_bb(Bitboard b) {
    return C == Color::white
        ? shift<Direction::up_left>(b) | shift<Direction::up_right>(b)
        : shift<Direction::down_left>(b) | shift<Direction::down_right>(b);
}

/**
 * Taken from Stockfish:
 */

/// lsb() and msb() return the least/most significant bit in a non-zero bitboard
inline Square lsb(Bitboard b) {
  assert(b);
  return Square(__builtin_ctzll(b));
}

inline Square msb(Bitboard b) {
  assert(b);
  return Square(63 ^ __builtin_clzll(b));
}

/// least_significant_square_bb() returns the bitboard of the least significant
/// square of a non-zero bitboard. It is equivalent to square_bb(lsb(bb)).
inline Bitboard least_significant_square_bb(Bitboard b) {
  assert(b);
  return b & -b;
}

/// pop_lsb() finds and clears the least significant bit in a non-zero bitboard
inline Square pop_lsb(Bitboard& b) {
  assert(b);
  const Square s = lsb(b);
  b &= b - 1;
  return s;
}


/// frontmost_sq() returns the most advanced square for the given color,
/// requires a non-zero bitboard.
inline Square frontmost_sq(Color c, Bitboard b) {
  assert(b);
  return c == Color::white ? msb(b) : lsb(b);
}

#endif // BITBOARD_H_
