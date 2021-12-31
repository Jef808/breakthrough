// #undef _GLIBCXX_DEBUG // disable run-time bound checking, etc
// #pragma GCC optimize("Ofast,inline") // Ofast = O3,fast-math,allow-store-data-races,no-protect-parens

// #pragma GCC target("bmi,bmi2,lzcnt,popcnt") // bit manipulation
// #pragma GCC target("movbe") // byte swap
// #pragma GCC target("aes,pclmul,rdrnd") // encryption
// #pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2") // SIMD

#include "types.h"
#include "game.h"
#include "agent.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

using namespace std;

Action random_action(const Game& game) {
    static std::random_device rd;
    static std::mt19937 eng{rd()};

    auto d = game.valid_actions().size();
    std::uniform_int_distribution<> dist(0, d - 1);

    return game.valid_actions()[dist(eng)];
}

int main() {
    BB::init();
    Game game_backup;

    auto start = std::chrono::steady_clock::now();

    for (int i=0; i<1000000; ++i) {
        Game game = game_backup;
        while (!game.is_lost()) {
            game.compute_valid_actions();
            Action a = random_action(game);
            game.apply(a);
        }
    }

    std::cout << "Time taken for 1,000,000 random playouts: "
              << std::setprecision(2)
              << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()
              << "ms." << std::endl;
}
