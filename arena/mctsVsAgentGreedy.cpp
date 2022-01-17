#include "game.h"
#include "epsilonGreedy.h"
#include "mcts.h"

#include <chrono>
#include <iostream>
#include <unordered_map>
#include <random>
#include <vector>


constexpr int default_n_battles = 10;
constexpr auto exp_cst = 0.7;
constexpr auto n_mcts_iterations = 300;
constexpr auto n_initial_samples = 1;

int main(int argc, char *argv[]) {
    Game::init();
    Game game{};
    StateData states[max_depth];
    StateData* sd = &states[0];

    int n_battles = default_n_battles;

    if (argc > 1)
        int n_battles = std::stoi(argv[1]);

    int mcts_wins_white = 0;
    int mcts_wins_black = 0;

    Mcts mcts(game);
    mcts.set_exp_cst(exp_cst);
    mcts.set_n_init_samples(n_initial_samples);
    mcts.set_n_iterations(n_mcts_iterations);

    double total_avg_time_mcts = 0.0;
    double total_avg_time_greedy = 0.0;

    for (int i=0; i<n_battles; ++i) {
        game.reset();
        sd = &states[0];

        game.reset();
        Agent agent(game);
        mcts.reset(game);

        int move_counter = 0;
        double time_mcts = 0.0;
        double time_greedy = 0.0;

        Color mcts_color = i & 1 ? Color::white : Color::black;

        Action action;
        while (!game.is_lost()) {
            if (game.player_to_move() == mcts_color) {
                auto start = std::chrono::steady_clock::now();
                action = mcts.best_action();
                time_mcts += std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start).count();
            } else {
                auto start = std::chrono::steady_clock::now();
                action = agent.best_action();
                time_greedy += std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start).count();
            }

            game.apply(action, *sd++);
            ++move_counter;
        }
        total_avg_time_mcts += time_mcts;
        total_avg_time_greedy += time_greedy;

        if (game.player_to_move() != mcts_color) {
            ++(mcts_color == Color::white ? mcts_wins_white : mcts_wins_black);
            std::cerr << "AGENT_MCTS wins!" << std::endl;
        }
        else {
            std::cerr << "AGENT_EPSILON_GREEDY wins!" << std::endl;
        }
    }

    std::cout << "**** AGENT_MCTS vs AGENT_EPSILON_GREEDY: "
        << n_battles
        << " battles]\n"
        << mcts_wins_white << " wins as white "
        << mcts_wins_black << " wins as black\n"
        << "    Winrate: "
        << 100.0 * (mcts_wins_white + mcts_wins_black) / n_battles << "%"
        << std::endl;

    std::cout << "\nn_iterations: " << n_mcts_iterations
        << "\nexploration constant: " << exp_cst
        << "\nn_initial_samples: " << n_initial_samples << std::endl;

    std::cout << "\nMCTS's Average time per move: "
        << total_avg_time_mcts / n_battles << "ms"
        << "AGENT_GREEDY's Average time per move: "
        << total_avg_time_greedy / n_battles << "ms"
        << std::endl;
}
