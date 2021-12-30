#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>

#include "game.h"

Game::Game() {
    m_board.fill(Piece::none);
    for (int row : { 0, 1 }) {
        for (int col = 0; col < 8; ++col) {
            m_board[col + (row << 3)] = Piece::white;
        }
    }
    for (int row : { 6, 7 }) {
        for (int col = 0; col < 8; ++col) {
            m_board[col + (row << 3)] = Piece::black;
        }
    }
    m_ply = 0;
    m_player_to_move = Color::white;
}

void Game::turn_input(std::istream& ins) {
    static std::string buf;
    int n_legal_moves;
    buf.clear();
    std::getline(ins, buf);

    if (buf != "None") {
        Action move = action_of(buf);
        apply(move);
    }

    ins >> n_legal_moves;
    ins.ignore();

    m_valid_actions.clear();
    for (int i = 0; i < n_legal_moves; ++i) {
        std::getline(ins, buf);
        m_valid_actions.push_back(action_of(buf));
    }
}

std::ostream& operator<<(std::ostream& out, Piece piece) {
    switch(piece) {
        case Piece::none: return out << " . ";
        case Piece::white: return out << " W ";
        case Piece::black: return out << " B ";
        default:
            assert(false);
    }
}

std::ostream& operator<<(std::ostream& out, Color player) {
    switch (player) {
        case Color::white: return out << "WHITE";
        case Color::black: return out << "BLACK";
        default:
            assert(false);
    }
}

std::string_view Game::view() const {
    static std::string view_buf;
    std::ostringstream out;

    out << "\n            ply " << m_ply << "\n       "
        << m_player_to_move
        << " to play\n";

    for (int row = height - 1; row >= 0; --row) {
        out << row + 1 << "  ";
        for (int col = 0; col < width; ++col) {
            out << piece_at(col, row);
        }
        out << '\n';
    }
    out << "   ";
    for (int col = 0; col < width; ++col) {
        char c = col + 97;
        out << ' ' << c << ' ';
    }
    out << '\n'
        << std::endl;

    view_buf = out.str();

    return view_buf;
}


bool Game::apply(Action a) {
    Square from = from_square(a);
    Square to = to_square(a);
    Piece p = piece_at(from);

    if (p != m_player_to_move)
        return false;
    if (column_of(to) == column_of(from) && piece_at(to) != Piece::none)
        return false;
    remove_piece(from);
    put_piece(p, to);

    m_player_to_move = opposite_of(m_player_to_move);
    ++m_ply;
    return true;
}

void Game::apply_nochecks(Action a) {
    Piece p = piece_at(from_square(a));
    remove_piece(from_square(a));
    put_piece(p, to_square(a));

    m_player_to_move = opposite_of(m_player_to_move);
    ++m_ply;
}

void Game::compute_valid_actions() {
    const Direction up = m_player_to_move == Color::white ? Direction::up : Direction::down;
    const int row_first = m_player_to_move == Color::white ? 0 : 7;
    const int row_last = m_player_to_move == Color::white ? 7 : 0;
    const int plus_one = m_player_to_move == Color::white ? 1 : -1;

    m_valid_actions.clear();
    auto out = std::back_inserter(m_valid_actions);

    for (int r = row_last; r >= row_first; r -= plus_one) {
        Row row = Row(r);
        Column col = Column::a;
        if (piece_at(col, row) == m_player_to_move) {
            if (piece_at(col, row + plus_one) == Piece::none)
                out = make_action(square_at(col, row), square_at(col, row + plus_one));
            if (piece_at(col + 1, row + plus_one) != m_player_to_move)
                out = make_action(square_at(col, row), square_at(col + 1, row + plus_one));
        }
        for (Column col = Column::b; col != Column::h; ++col) {
            if (piece_at(col, row) != m_player_to_move)
                continue;
            if (piece_at(col + -1, row + plus_one) != m_player_to_move)
                out = make_action(square_at(col, row), square_at(col + -1, row + plus_one));
            if (piece_at(col, row + plus_one) == Piece::none)
                out = make_action(square_at(col, row), square_at(col, row + plus_one));
            if (piece_at(col + 1, row + plus_one) != m_player_to_move)
                out = make_action(square_at(col, row), square_at(col + 1, row + plus_one));
        }
        col = Column::h;
        if (piece_at(col, row) == m_player_to_move) {
            if (piece_at(col + -1, row + plus_one) != m_player_to_move)
                out = make_action(square_at(col, row), square_at(col + -1, row + plus_one));
            if (piece_at(col, row + plus_one) == Piece::none)
                out = make_action(square_at(col, row), square_at(col, row + plus_one));
        }
    }
}
