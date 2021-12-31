#include "types.h"
#include "bitboard.h"

#include <string>
#include <sstream>
#include <string_view>
#include <iostream>

std::string_view to_string(Color c) {
    return c == Color::white ? "WHITE" : "BLACK";
}

void bit_view(std::ostream& out, Bitboard bb) {
    std::ostringstream oss;
    while (bb) {
        oss << (bb & 1 ? '1' : '0');
        bb = bb >> 1;
    }
    auto buf = oss.str();
    int padding = 64 - buf.size();
    for (int i = 0; i < padding; ++i) {
        out << '0';
    }
    for (auto it = buf.rbegin(); it != buf.rend(); ++it) {
        out << *it;
    }
}

void view(Bitboard* pbb, std::string_view name, Color color = Color::Nb) {
    for (Square sq = Square::a1; sq < Square::Nb; ++sq) {
        Bitboard bb = *pbb++;
        std::ostringstream oss{ "Span of "};
        oss << name << " of " << string_of(sq);
        if (color < Color::Nb)
            oss << " for " << to_string(color);
        oss << '\n';
        bit_view(oss, bb);
        BB::view(std::cout, bb, oss.str());
    }
}

int main() {
    BB::init();

    view(&BB::Span[to_integral(Color::white)][0], "BB::Span", Color::white);
    view(&BB::Span[to_integral(Color::black)][0], "BB::Span", Color::black);

}
