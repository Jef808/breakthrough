#include "types.h"
#include "game.h"
#include "mcts.h"
#include "agentRandom.h"

#include <iostream>
#include <sstream>
#include <chrono>


constexpr auto graphviz_fn = "tree.dot";

constexpr auto n_iterations = 300;
constexpr auto exp_cst = 1.4;
constexpr auto n_initial_samples = 4;
constexpr auto n_games = 10;

int main(int argc, char *argv[]) {
    Game::init();
    Game game;

    Mcts mcts{ game };
    AgentRandom random { game };

    mcts.set_exp_cst(exp_cst);
    mcts.set_n_init_samples(n_initial_samples);
    mcts.set_n_iterations(n_iterations);

    StateData states[max_depth], *sd = &states[0];

    int n_wins_white = 0;
    int n_wins_black = 0;

    double total_avg_time = 0.0;

    for (int i=0; i<n_games; ++i) {
        game.reset();
        mcts.reset(game);
        int move_counter = 0;
        double time = 0.0;

        sd = &states[0];

        Color mcts_color = i & 1 ? Color::white : Color::black;

        while (!game.is_lost()) {
            Action action;

            if (game.player_to_move() == mcts_color) {
                auto start = std::chrono::steady_clock::now();
                action = mcts.best_action();
                time += std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start).count();
            }
            else {
                action = random.best_action();
            }

            game.apply(action, *sd++);
            ++move_counter;
        }
        total_avg_time += time;

        if (game.player_to_move() != mcts_color) {
            ++(mcts_color == Color::white ? n_wins_white : n_wins_black);
            std::cerr << "AGENT_MCTS wins!" << std::endl;
        } else {
            std::cerr << "AGENT_RANDOM wins!" << std::endl;
        }
    }

    std::cout << "\nn_iterations: " << n_iterations
        << "\nexploration constant: " << exp_cst
        << "\nn_initial_samples: " << n_initial_samples << std::endl;

    std::cout << "Games won:\n"
              << n_wins_white << " as white,"
              << n_wins_black << " as black,"
              << "\nWinrate: " << 100.0 * (n_wins_white + n_wins_black) / n_games << "%"
              << std::endl;

    std::cout << "Average time per move: "
        << total_avg_time / n_games << "ms"
        << std::endl;

    //mcts.write_graphviz(graphviz_fn);
    return 0;
}
