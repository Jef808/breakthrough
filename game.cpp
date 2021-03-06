#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <random>
#include <sstream>
#include <string>
#include <string_view>

#include "types.h"
#include "bitboard.h"
#include "game.h"


namespace {
    StateData root_sd{};
}  // namespace


namespace Zobrist {

// Two keys per square for the two colors, plus
// one more key indicating it is black's turn to play.
Key keyTable[Ncolors][Nsquares];
Key side;

inline Key key(Color c, Square sq) {
    return keyTable[to_integral(c)][to_integral(sq)];
}

}  // namespace Zobrist

Game::Game()
{
    reset();
}

void Game::reset() {
    std::fill_n(std::begin(m_board), 16, Piece::white);
    std::fill_n(std::begin(m_board) + 16, 32, Piece::none);
    std::fill_n(std::begin(m_board) + 48, 16, Piece::black);
    std::fill(std::begin(by_color), std::end(by_color), 0);

    by_color[to_integral(Color::white)] = (row_bb(Row::one) | row_bb(Row::two));
    by_color[to_integral(Color::black)] = (row_bb(Row::eight) | row_bb(Row::seven));

    m_ply = 0;
    m_player_to_move = Color::white;

    sd = &root_sd;
    sd->capture = false;
    sd->action = Action::none;
    sd->key = 0;
    sd->prev = nullptr;

    for (Square sq = Square::a1; sq < Square::a3; ++sq) {
        sd->key ^= Zobrist::key(Color::white, sq);
    }
    for (Square sq = Square::a7; sq < Square::Nb; ++sq) {
        sd->key ^= Zobrist::key(Color::black, sq);
    }
}

void Game::init() {
    BB::init();

    std::random_device rd;
    std::mt19937 eng{rd()};

    for (auto& i : Zobrist::keyTable)
        for (unsigned int& j : i)
            j = eng();

    // for (int i=0; i<Ncolors; ++i)
    //     for (int j=0; j<Nsquares; ++j)
    //         Zobrist::keyTable[i][j] = eng();

    Zobrist::side = eng();
}

void Game::turn_input(std::istream& ins, StateData& sd, bool store_actions) {
    static std::string buf;
    int n_legal_moves;
    buf.clear();
    m_action_buffer.clear();

    std::getline(ins, buf);
    if (buf != "None") {
        Action move = action_of(buf);
        apply(move, sd);
    }
    ins >> n_legal_moves;
    ins.ignore();
    for (int i = 0; i < n_legal_moves; ++i) {
        std::getline(ins, buf);
        if (store_actions)
            m_action_buffer.push_back(action_of(buf));
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


enum class StringT { Raw, Rich };

template<StringT>
std::string_view get_string(const Game& game, Action action);

template<>
std::string_view get_string<StringT::Rich>(const Game& game, Action action) {
    static std::string view_buf;
    std::ostringstream out;

    out << "\n            ply: " << game.ply() << "\n       "
        << game.player_to_move()
        << " to play\n";
    if (action != Action::none)
        out << "       Last played: " << string_of(action) << "\n";
    out << '\n';

    for (int r = to_integral(Row::eight); r >= 0; --r) {
        out << r + 1 << "  ";
        for (Square sq = square_at(Column::a, Row(r)); sq <= square_at(Column::h, Row(r)); ++sq) {
            out << game.piece_at(sq);
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

template<>
std::string_view get_string<StringT::Raw>(const Game& game, Action action) {
    static std::string view_buf;
    std::ostringstream out;

    out << R"(\n            ply: )" << game.ply() << R"(\n       )"
        << game.player_to_move()
        << R"( to play\n)";
    if (action != Action::none)
        out << "       Last played: " << string_of(action) << R"(\n)";
    out << R"(\n)";

    for (int r = to_integral(Row::eight); r >= 0; --r) {
        out << r + 1 << "  ";
        for (Square sq = square_at(Column::a, Row(r)); sq <= square_at(Column::h, Row(r)); ++sq) {
            out << game.piece_at(sq);
        }
        out << R"(\n)";
    }
    out << "   ";
    for (int c = 0; c < width; ++c) {
        char ch = c + 97;
        out << ' ' << ch << ' ';
    }
    out << R"(\n)";

    view_buf = out.str();
    return view_buf;
}

/**
 * Generate a view of a string representation of the board.
 *
 * Optionally write @action in the header to indicate the last
 * action played.
 */
std::string_view Game::view(Action action, bool raw) const {
    return raw ? get_string<StringT::Raw>(*this, action)
               : get_string<StringT::Rich>(*this, action);
}


void Game::apply(Action a, StateData& sd) {
    Square from = from_square(a);
    Square to = to_square(a);

    assert(piece_at(from) == m_player_to_move);
    assert(piece_at(to) != m_player_to_move);
    assert(is_empty(to) || column_of(to) != column_of(from));

    // Move the StateData object
    sd.key = this->sd->key;
    sd.capture = false;
    sd.action = a;
    sd.prev = this->sd;
    this->sd = &sd;

    // Maybe capture a piece
    if (pieces(opposite_of(m_player_to_move)) & square_bb(to)) {
        remove_piece(to);
        sd.key ^= Zobrist::key(opposite_of(m_player_to_move), to);
        sd.capture = true;
    }

    sd.key ^= Zobrist::key(m_player_to_move, from) ^ Zobrist::key(m_player_to_move, to);
    sd.key ^= Zobrist::side;

    move_piece(from, to);
    m_player_to_move = opposite_of(m_player_to_move);
    ++m_ply;
}

void Game::undo(Action a) {
    Square to = to_square(a);
    Square from = from_square(a);

    --m_ply;
    m_player_to_move = opposite_of(m_player_to_move);
    move_piece(to, from);

    if (sd->capture)
        put_piece(make_piece(opposite_of(m_player_to_move)), to);

    sd = sd->prev;
}

/**
 * Populate @out with all legal actions.
 *
 * Does not check if game is already won so
 * only call after checking for a win somewhere
 */
void Game::compute_valid_actions(std::vector<Action>& out) const {
    Bitboard pcs = pieces(m_player_to_move);
    out.clear();
    while (pcs) {
        Square sq = pop_lsb(pcs);
        Bitboard free_ahead = forward_bb(m_player_to_move, sq) & no_pieces();
        if (free_ahead)
            out.push_back(make_action(sq, lsb(free_ahead)));
        Bitboard sides_free = attacks_bb(m_player_to_move, sq) & ~pieces(m_player_to_move);
        while (sides_free) {
            out.push_back(make_action(sq, pop_lsb(sides_free)));
        }
    }
}
