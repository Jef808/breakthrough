#ifndef BITBOARD_H_
#define BITBOARD_H_

#include "types.h"

#include <cstdint>
#include <iosfwd>
#include <string_view>

using Bitboard = uint64_t;

namespace BB {

void init();
void view(std::ostream& out, Bitboard bb, std::string_view bb_name = "");

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

constexpr Bitboard QueenSide           =   ColA | ColB  |  ColC | ColD;
constexpr Bitboard KingSide            =   ColE | ColF  |  ColG | ColH;
constexpr Bitboard Center              =   (ColD| ColE) & (Row4 | Row5);
constexpr Bitboard BottomSide[Ncolors] = { Row2 | Row3 | Row4  , Row7  | Row6 | Row5 };
constexpr Bitboard UpperSide[Ncolors]  = { Row5 | Row6 | Row7  , Row4 | Row3  | Row2 };
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

constexpr Bitboard crit_rows(Color c) {
    return BB::BottomSide[to_integral(c)];
}
constexpr Bitboard scoring_rows(Color c) {
    return BB::UpperSide[to_integral(c)];
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

/**
 * Bitboard representing the squares attacking a given square
 */
inline Bitboard attackers_bb(Color c, Square sq) {
    return BB::Defenders[to_integral(c)][to_integral(sq)];
}

/**
 * Bitboard representing the cone of squares opening forward
 */
inline Bitboard span_bb(Color c, Square sq) {
    return BB::Span[to_integral(c)][to_integral(sq)];
}

/**
 * Flip a bitboard accross the middle vertically
 */
constexpr Bitboard flip_vertical(Bitboard bb) {
    return __builtin_bswap64(bb);
}

/**
 * Count the number of set bits in a bitboard
 */
constexpr int count(Bitboard bb) {
    return __builtin_popcountll(bb);
}

/**
 * Least significant / Most significant bit respectively
 */
inline Square lsb(Bitboard b) {
  assert(b);
  return Square(__builtin_ctzll(b));
}
inline Square msb(Bitboard b) {
  assert(b);
  return Square(63 ^ __builtin_clzll(b));
}

/**
 * Return the bitboard of the least significant
 * square of a non-zero bitboard.
 */
inline Bitboard least_significant_square_bb(Bitboard b) {
  assert(b);
  return b & -b;
}

/**
 * Finds and clears the least significant bit in a non-zero bitboard
 */
inline Square pop_lsb(Bitboard& b) {
  assert(b);
  const Square s = lsb(b);
  b &= b - 1;
  return s;
}

/**
 * Return the most advanced square for the given color,
 * requires a non-zero bitboard.
 */
inline Square frontmost_sq(Color c, Bitboard b) {
  assert(b);
  return c == Color::white ? msb(b) : lsb(b);
}

#endif // BITBOARD_H_
