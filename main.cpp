#include <cassert>
#include <algorithm>
#include <iostream>

#include "game.h"
#include "agent.h"

constexpr double epsilon = 0.3;
constexpr int n_iterations = 5000;

int main() {
    BB::init();
    Game game;
    Agent agent(game);

    agent.set_epsilon(epsilon);
    agent.set_n_iterations(n_iterations);

    while (true) {
        game.turn_input(std::cin);

        Action action = agent.best_action();
        std::cout << string_of(action) << std::endl;
        game.apply(action);
    }

    return EXIT_SUCCESS;
}
