#include <cassert>
#include <algorithm>
#include <iostream>

#include "game.h"
#include "epsilonGreedy.h"

constexpr double epsilon = 0.3;
constexpr int n_iterations = 5000;

int main() {
    BB::init();
    Game game;
    Agent agent(game);
    StateData states[max_depth];
    StateData* sd = &states[0];

    agent.set_epsilon(epsilon);
    agent.set_n_iterations(n_iterations);

    while (true) {
        game.turn_input(std::cin, *sd++);

        Action action = agent.best_action();
        std::cout << string_of(action) << std::endl;
        game.apply(action, *sd++);
    }

    return EXIT_SUCCESS;
}
