#include <cassert>
#include <algorithm>
#include <iostream>
#include <random>

#include "game.h"
#include "agentRandom.h"

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

void output_error(const std::vector<Action>& ga, const std::vector<Action>& ma, std::ostream& out) {
    std::cerr << "Test failed:\nGameActions:\n    ";
    for (auto action : ga)
        std::cerr << string_of(action) << ' ';
    std::cerr << std::endl << "My actions:\n    ";
    for (auto action : ma)
    std::cerr << string_of(action) << ' ';
    std::cerr << std::endl;
}

int main() {
    std::ios_base::sync_with_stdio(false);

    Game::init();
    Game game{};

    StateData states[max_depth];
    StateData* sd = &states[1];

    std::vector<Action> my_actions;

    AgentRandom agent_rand(game);

    while (true) {
        game.turn_input(std::cin, *sd++, true);

        auto game_actions = game.valid_actions();
        game.compute_valid_actions(my_actions);

        std::sort(game_actions.begin(), game_actions.end(), CmpActionsTest{});
        std::sort(my_actions.begin(), my_actions.end(), CmpActionsTest{});

        if (game_actions != my_actions) {
            output_error(game_actions, my_actions, std::cerr);
            return EXIT_FAILURE;
        }

        Action action = agent_rand.sample();
        std::cout << string_of(action) << std::endl;
        game.apply(action, *sd++);
    }

    return EXIT_SUCCESS;
}
