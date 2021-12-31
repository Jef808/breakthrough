#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>

#include "bitboard.h"
#include "game.h"

Game::Game() {
    std::fill_n(std::begin(m_board), 16, Piece::white);
    std::fill_n(std::begin(m_board) + 16, 32, Piece::none);
    std::fill_n(std::begin(m_board) + 48, 16, Piece::black);
    by_color[to_integral(Color::white)] = (row_bb(Row::one) | row_bb(Row::two));
    by_color[to_integral(Color::black)] = (row_bb(Row::eight) | row_bb(Row::seven));
    m_ply = 0;
    m_player_to_move = Color::white;
}

void Game::turn_input(std::istream& ins) {
    static std::string buf;
    int n_legal_moves;
    buf.clear();
    m_valid_actions.clear();
    std::getline(ins, buf);
    if (buf != "None") {
        Action move = action_of(buf);
        apply(move);
    }
    ins >> n_legal_moves;
    ins.ignore();
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

    for (int r = to_integral(Row::eight); r >= 0; --r) {
        out << r + 1 << "  ";
        for (Square sq = square_at(Column::a, Row(r)); sq <= square_at(Column::h, Row(r)); ++sq) {
            out << piece_at(sq);
        }
        out << '\n';
    }
    out << "   ";
    for (int c = 0; c < width; ++c) {
        char ch = c + 97;
        out << ' ' << ch << ' ';
    }
    out << '\n' << std::endl;

    view_buf = out.str();
    return view_buf;
}


void Game::apply(Action a) {
    Square from = from_square(a);
    Square to = to_square(a);
    Piece p = piece_at(from);

    assert(piece_at(from) == m_player_to_move);
    assert(piece_at(to) != m_player_to_move);
    assert(is_empty(to) || column_of(to) != column_of(from));

    if (!is_empty(to))
        remove_piece(to);

    move_piece(from, to);
    m_player_to_move = opposite_of(m_player_to_move);
    ++m_ply;
}

void Game::compute_valid_actions() {
    Bitboard pcs = pieces(m_player_to_move);
    m_valid_actions.clear();
    while (pcs) {
        Square sq = pop_lsb(pcs);
        Bitboard free_ahead = forward_bb(m_player_to_move, sq) & no_pieces();
        if (free_ahead)
            m_valid_actions.push_back(make_action(sq, lsb(free_ahead)));//relative(m_player_to_move, Direction::up)));
        Bitboard sides_free = attacks_bb(m_player_to_move, sq) & ~pieces(m_player_to_move);
        while (sides_free) {
            m_valid_actions.push_back(make_action(sq, pop_lsb(sides_free)));
        }
    }
}
