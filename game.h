#ifndef GAME_H_
#define GAME_H_

#include <array>
#include <iosfwd>
#include <string_view>
#include <vector>

#include "types.h"
#include "bitboard.h"


struct StateData {
    Key key;
    bool capture;
    StateData* prev;
};

class Game {
public:
    Game();
    static void init();
    void turn_input(std::istream&);
    std::string_view view() const;
    void apply(Action, StateData& sd);
    void undo(Action a);
    void compute_valid_actions();
    const std::vector<Action>& valid_actions() const { return m_valid_actions; }
    bool is_lost() const;
    constexpr Color player_to_move() const { return m_player_to_move; }
    constexpr Key key() const { return sd->key; }

    constexpr Piece piece_at(Square s) const;
    constexpr Bitboard pieces(Color c) const;
    constexpr Bitboard no_pieces() const;
    constexpr bool is_empty(Square s) const;
    constexpr int ply() const { return m_ply; }

private:
    Piece m_board[Nsquares];
    Bitboard by_color[Ncolors];
    std::vector<Action> m_valid_actions;
    StateData* sd;
    int m_ply;
    Color m_player_to_move;

    void remove_piece(Square);
    void put_piece(Piece, Square);
    void move_piece(Square from, Square to);
};

constexpr Piece Game::piece_at(Square s) const {
    return m_board[to_integral(s)];
}
constexpr bool Game::is_empty(Square s) const {
    return piece_at(s) == Piece::none;
}
constexpr Bitboard Game::pieces(Color c) const {
    return by_color[to_integral(c)];
}
constexpr Bitboard Game::no_pieces() const {
    return ~(pieces(Color::white) | pieces(Color::black));
}
inline bool Game::is_lost() const {
    return pieces(opposite_of(m_player_to_move)) & row_bb(relative(m_player_to_move, Row::one));
}
inline void Game::remove_piece(Square s) {
    Piece& p = m_board[to_integral(s)];
    by_color[to_integral(color_of(p))] ^= square_bb(s);
    p = Piece::none;
}
inline void Game::put_piece(Piece p, Square s) {
    m_board[to_integral(s)] = p;
    by_color[to_integral(color_of(p))] ^= square_bb(s);
}
inline void Game::move_piece(Square from, Square to) {
    Piece p = m_board[to_integral(from)];
    Bitboard move = square_bb(from) | square_bb(to);
    by_color[to_integral(color_of(p))] ^= move;
    m_board[to_integral(from)] = Piece::none;
    m_board[to_integral(to)] = p;
}

#endif // GAME_H_
