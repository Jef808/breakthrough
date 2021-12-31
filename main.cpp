#include <cassert>
#include <algorithm>
#include <iostream>

#include "game.h"
#include "agent.h"

int main() {
    BB::init();
    Game game;
    Agent agent(game);

    while (true) {
        game.turn_input(std::cin);

        Action action = agent.best_action();
        std::cout << string_of(action) << std::endl;
        game.apply(action);
    }

    return EXIT_SUCCESS;
}
