// #undef _GLIBCXX_DEBUG // disable run-time bound checking, etc
// #pragma GCC optimize("Ofast,inline") // Ofast = O3,fast-math,allow-store-data-races,no-protect-parens

// #pragma GCC target("bmi,bmi2,lzcnt,popcnt") // bit manipulation
// #pragma GCC target("movbe") // byte swap
// #pragma GCC target("aes,pclmul,rdrnd") // encryption
// #pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2") // SIMD

#include "types.h"
#include "game.h"
#include "agentRandom.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

constexpr int default_n_samples = 10000;


int main(int argc, char *argv[]) {
    Game::init();
    Game game{};
    StateData states[max_depth];

    AgentRandom agent(game);

    int n_samples = default_n_samples;

    if (argc > 1)
        n_samples = std::stoi(argv[1]);

    auto start = std::chrono::steady_clock::now();

    for (int i=0; i < n_samples; ++i) {
        game.reset();
        StateData* sd = &states[0];

        while (!game.is_lost()) {
            auto action = agent.sample();//random_action(game);
            game.apply(action, *sd++);
        }
    }

    std::cout << "Time taken for "
              << n_samples
              << " random playouts: "
              << std::setprecision(2)
              << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()
              << "ms." << std::endl;
}
