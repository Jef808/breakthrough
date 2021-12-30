#include <cassert>
#include <algorithm>
#include <iostream>

#include "game.h"
#include "agent.h"

int main() {
    Game game;
    Agent agent(game);

    while (true) {
        game.turn_input(std::cin);

        Action action = agent.random_action();
        std::cout << string_of(action) << std::endl;
        game.apply(action);
    }

    return EXIT_SUCCESS;
}
