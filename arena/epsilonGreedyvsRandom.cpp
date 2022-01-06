#include "game.h"
#include "epsilonGreedy.h"
#include "agentRandom.h"

#include <iostream>
#include <unordered_map>
#include <random>
#include <vector>


constexpr int default_n_battles = 10;


int main(int argc, char *argv[]) {
    Game::init();
    Game game{};
    StateData states[max_depth];
    StateData* sd = &states[0];

    int n_battles = default_n_battles;

    if (argc > 1)
        int n_battles = std::stoi(argv[1]);

    int n_wins_white = 0;
    int n_wins_black = 0;

    for (int i=0; i<n_battles; ++i) {
        game.reset();
        sd = &states[0];

        Agent agent(game);
        AgentRandom agent_random(game);

        Color agent_color = i & 1 ? Color::white : Color::black;

        while (!game.is_lost()) {
            Action action = game.player_to_move() == agent_color
                ? agent.best_action()
                : agent_random.best_action();
            game.apply(action, *sd++);
        }

        if (game.player_to_move() != agent_color) {
            ++(agent_color == Color::white ? n_wins_white : n_wins_black);
            std::cerr << "AGENT_EPSILON_GREEDY wins!" << std::endl;
        }
        else {
            std::cerr << "AGENT_RANDOM wins!" << std::endl;
        }
    }

    std::cout << "**** AGENT_EPSILON_GREEDY vs AGENT_RANDOM ["
        << n_battles
        << " battles]\n"
        << n_wins_white << " wins as white "
        << n_wins_black << " wins as black\n"
        << "    Winrate: "
        << 100.0 * (n_wins_white + n_wins_black) / n_battles << "%."
        << std::endl;
}
