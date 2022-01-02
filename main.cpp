#include <cassert>
#include <algorithm>
#include <iostream>

#include "game.h"
#include "epsilonGreedy.h"

constexpr double epsilon = 0.3;
constexpr int n_iterations = 5000;
constexpr int n_initial_samples = 10;

int main() {
    Game::init();

    Game game;
    Agent agent(game);
    StateData states[max_depth];
    std::fill(std::begin(states), std::end(states), StateData{});
    StateData* sd = &states[1];

    agent.set_epsilon(epsilon);
    agent.set_n_iterations(n_iterations);
    agent.set_n_initial_samples(n_initial_samples);

    while (true) {
        game.turn_input(std::cin, *sd++);

        Action action = agent.best_action();
        std::cout << string_of(action) << std::endl;
        game.apply(action, *sd++);
    }

    return EXIT_SUCCESS;
}
