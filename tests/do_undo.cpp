#include "types.h"
#include "game.h"

#include <iostream>
#include <random>


int main() {
    Game::init();
    Game game;

    StateData states[max_depth];
    std::fill(std::begin(states), std::end(states), StateData{});
    StateData* sd = &states[1];

    Action actions[max_depth];
    std::fill(std::begin(actions), std::end(actions), Action::none);
    Action* pa = &actions[1];

    std::random_device rd;
    std::mt19937 eng{rd()};

    while (!game.is_lost()) {
        std::cout << game.view() << '\n'
                  << "Press N to play a random action, press P to undo an action."
                  << std::endl;
        char ch = '\0';
        std::cin >> ch;
        std::cin.ignore();

        if (ch == 'N' || ch == 'n') {
            game.compute_valid_actions();
            std::uniform_int_distribution<> dist(0, game.valid_actions().size() - 1);
            *pa = game.valid_actions()[dist(eng)];

            game.apply(*pa++, *sd++);
        }
        else if (ch == 'P' || ch == 'p') {
            Action action = *(pa - 1);
            if (action == Action::none) {
                std::cout << "Cannot further undo." << std::endl;
            }
            else
                game.undo(*(--pa));
        }
    }

    std::cout << game.view() << '\n'
              << "Game over!" << std::endl;
}
