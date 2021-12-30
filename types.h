#ifndef TYPES_H_
#define TYPES_H_

#include <cassert>
#include <cstdint>
#include <string>
#include <string_view>

constexpr int width = 8;
constexpr int height = 8;
constexpr int Nsquares = 64;
constexpr int max_n_moves = 48;
constexpr int max_depth = 256;

enum class Color : uint8_t {
    white = 0,
    black = 1
};

enum class Piece : uint8_t {
    white = 0,
    black = 1,
    none  = 2
};

enum class Square {
    a1=0 , b1=1 , c1=2 , d1=3 , e1=4 , f1=5 , g1=6 , h1=7 ,
    a2=8 , b2=9 , c2=10, d2=11, e2=12, f2=13, g2=14, h2=15,
    a3=16, b3=17, c3=18, d3=19, e3=20, f3=21, g3=22, h3=23,
    a4=24, b4=25, c4=26, d4=27, e4=28, f4=29, g4=30, h4=31,
    a5=32, b5=33, c5=34, d5=35, e5=36, f5=37, g5=38, h5=39,
    a6=40, b6=41, c6=42, d6=43, e6=44, f6=45, g6=46, h6=47,
    a7=48, b7=49, c7=50, d7=51, e7=52, f7=53, g7=54, h7=55,
    a8=56, b8=57, c8=58, d8=59, e8=60, f8=61, g8=62, h8=63,
};

enum class Column : char {
    a=0, b=1, c=2, d=3, e=4, f=5, g=6, h=7
};

enum class Row : char {
    one=0, two=1, three=2, four=3, five=4, six=5, seven=6, eight=7
};

enum class Direction {
    up = width,
    right= 1,
    left = -1,
    down = -width,

    up_right  = up   + right,
    down_right= right+ down,
    down_left = down + left,
    up_left   = left + up,
};

/**
 * The least significant byte stores the source square,
 * the other byte stores the destination square.
 *
 * Note: 0 is a1 to a1 and 257 is b1 to b1.
 *       They are both illegal moves.
 */
enum class Action : uint16_t {
    none = 0,
    invalid = 257
};

template<class E>
constexpr auto to_integral(E e) {
  return static_cast<std::underlying_type_t<E>>(e);
}

/**
 * Operations for squares.
 *
 * Notice that in binary, a square is 0brrrccc
 * where 0brrr is the row, 0bccc is the column.
 */
constexpr Square square_at(int col, int row) {
  return Square(col + (row << 3));
}
constexpr Square square_at(Column col, Row row) {
  return Square(to_integral(col) + (to_integral(row) << 3));
}
constexpr Column column_of(Square s) {
  return Column(to_integral(s) & 7);
}
constexpr Row row_of(Square s) {
  return Row((to_integral(s) >> 3) & 7);
}
constexpr Square operator+(Square s, Direction dir) {
  return Square(static_cast<int>(s) + static_cast<int>(dir));
}
constexpr Square& operator++(Square& s) {
  assert(s < Square::h8);
  return s = Square(to_integral(s) + 1);
}
constexpr Square& operator+=(Square& s, int i) {
  assert(0 <= to_integral(s) && to_integral(s) + i <= 63);
  return s = Square(to_integral(s) + i);
}
constexpr Row operator+(Row r, int i) {
  return Row(to_integral(r) + i);
}
constexpr Row& operator+=(Row& r, int i) {
  assert(0 <= to_integral(r) + i && to_integral(r) + i <= 7);
  return r = Row(to_integral(r) + 1);
}
constexpr Row operator-(Row r, int i) {
  return Row(to_integral(r) - i);
}
constexpr Row& operator-=(Row& r, int i) {
  assert(0 <= to_integral(r) - i && to_integral(r) - i <= 7);
  return r = Row(to_integral(r) - 1);
}
/// 010 <-> 101, 110 <-> 001, etc...
constexpr Row relative(Color c, Row r) {
  return Row(to_integral(r) ^ (7 * to_integral(c)));
}
constexpr Column& operator++(Column& c) {
  assert(c < Column::h);
  return c = Column(to_integral(c) + 1);
}
constexpr Column operator+(Column c, int i) {
  return Column(to_integral(c) + i);
}
/// Same thing applied to the row
constexpr Square relative(Color c, Square s) {
   return Square(to_integral(s) ^ ((7 << 3) * to_integral(c)));
}
constexpr Square relative_square_at(Color c, int col, int row) {
  return Square(col + (row << 3) ^ (56 * to_integral(c)));
}
/**
 * Operations for actions
 */
constexpr Action make_action(Square from, Square to) {
  return Action((uint16_t)from + ((uint16_t)to << 8));
}
constexpr Action make_action(Square from, Direction dir) {
  return make_action(from, from + dir);
}
constexpr Square from_square(Action a) {
  return Square((uint16_t)a & 0x3f);
}
constexpr Square to_square(Action a) {
  return Square((uint16_t)a >> 8);
}
constexpr Direction direction_of(Action a) {
  return Direction(to_integral(to_square(a)) - to_integral(from_square(a)));
}
/// If (Row of to_square) < (Row of from_square), then Color is black.
constexpr Color player_of(Action a) {
  return Color(((uint16_t)a >> 11) < (((uint16_t)a & 0x3f) >> 3));
}

/**
 * Methods to convert squares and actions to and from strings
 */
constexpr Square square_of(std::string_view sv) {
  return Square((sv[0] - 97) + width * (sv[1] - 49));
}
inline std::string string_of(Square s) {
  static std::string sret(2, '\0');
  int as_int = static_cast<int>(s);
  sret[0] = (as_int & 7) + 97;   // col
  sret[1] = (as_int >> 3) + 49;  // row
  return sret;
}
constexpr Action action_of(std::string_view sv) {
  return make_action(square_of(sv.substr(0, 2)), square_of(sv.substr(2, 2)));
}
inline std::string string_of(Action a) {
  return string_of(from_square(a)) + string_of(to_square(a));
}

/**
 * Operations for colors and pieces
 */
constexpr bool operator==(Piece piece, Color color) {
  return Piece(to_integral(color)) == piece;
}
constexpr bool operator!=(Piece piece, Color color) {
  return Piece(to_integral(color)) != piece;
}
constexpr Color opposite_of(Color color) {
   return Color(to_integral(color) ^ 1);
}
constexpr Piece make_piece(Color color) {
  return Piece(to_integral(color));
}
constexpr Color color_of(Piece piece) {
  assert(piece != Piece::none);
  return Color(to_integral(piece));
}

constexpr Row goal_of(Color player) {
  return Row(7 ^ (7 * to_integral(player)));
}

#endif // TYPES_H_
