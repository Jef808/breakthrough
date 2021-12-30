#include <cassert>
#include <algorithm>
#include <iostream>

#include "game.h"
#include "agent.h"

struct CmpSquaresTest {
    bool operator()(Square a, Square b) {
        if (row_of(a) < row_of(b))
            return true;
        else if (row_of(a) == row_of(b) && column_of(a) < column_of(b))
            return true;
        return false;
    }
};

struct CmpActionsTest {
    bool operator()(Action a, Action b) {
        if (CmpSquaresTest{}(from_square(a), from_square(b)))
            return true;
        else if (from_square(a) == from_square(b) && CmpSquaresTest{}(to_square(a), to_square(b)))
            return true;
        return false;
    }
};

bool equal_as_sets(std::vector<Action>& vec1, std::vector<Action>& vec2) {
    std::sort(vec1.begin(), vec1.end(), CmpActionsTest{});
    std::sort(vec2.begin(), vec2.end(), CmpActionsTest{});

    if (vec1 != vec2) {
        std::cerr << "WARNING: Actions don't match:\nGame actions:\n    ";
        for (auto action : vec1) {
            std::cerr << string_of(action) << ' ';
        }
        std::cerr << std::endl;
        std::cerr << "My actions:\n    ";
        for (auto action : vec2) {
            std::cerr << string_of(action) << ' ';
        }
        std::cerr << std::endl;
        return false;
    }
    return true;
}

int main() {
    Game game;
    Agent agent(game);

    while (true) {
        game.turn_input(std::cin);

        auto game_actions = game.valid_actions();
        game.compute_valid_actions();
        auto my_actions = game.valid_actions();

        bool actions_ok = equal_as_sets(game_actions, my_actions);

        if (!actions_ok) {
            std::cerr << "Valid actions not same as game's input!"
                << std::endl;
            return EXIT_FAILURE;
        }

        Action action = agent.best_action();
        std::cout << string_of(action) << std::endl;
        game.apply(action);
    }

    return EXIT_SUCCESS;
}
