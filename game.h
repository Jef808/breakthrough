#ifndef GAME_H_
#define GAME_H_

#include <array>
#include <iosfwd>
#include <string_view>
#include <vector>

#include "types.h"

class Game {
public:
    Game();
    void turn_input(std::istream&);
    std::string_view view() const;
    bool apply(Action);
    void apply_nochecks(Action);
    void compute_valid_actions();
    const std::vector<Action>& valid_actions() const { return m_valid_actions; }

    constexpr Color player_to_move() const { return m_player_to_move; }
    constexpr Piece piece_at(int col, int row) const;
    constexpr Piece piece_at(Column col, Row row) const;
    constexpr Piece piece_at(Square s) const;
    constexpr int ply() const { return m_ply; }

private:
    std::array<Piece, width * height> m_board;
    std::vector<Action> m_valid_actions;
    int m_ply;
    Color m_player_to_move;

    void remove_piece(Square);
    void put_piece(Piece, Square);
};

constexpr Piece Game::piece_at(int col, int row) const {
    return m_board[col + width * row];
}
constexpr Piece Game::piece_at(Column col, Row row) const {
    return piece_at(to_integral(col), to_integral(row));
}
constexpr Piece Game::piece_at(Square s) const {
    return m_board[to_integral(s)];
}
inline void Game::remove_piece(Square s) {
    m_board[to_integral(s)] = Piece::none;
}
inline void Game::put_piece(Piece p, Square s) {
    m_board[to_integral(s)] = p;
}

#endif // GAME_H_
