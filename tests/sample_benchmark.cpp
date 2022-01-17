#include "types.h"
#include "game.h"
#include "agentRandom.h"
#include "mcts.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>


constexpr int default_n_playouts = 10000;

void playout_agent_random(Game& game, AgentRandom& agent) {
  StateData states[max_depth], *sd = &states[0];

  while (!game.is_lost()) {
    auto action = agent.sample();
    game.apply(action, *sd++);
  }
}

int main(int argc, char *argv[]) {
    Game::init();
    Game game{};
    StateData states[max_depth];
    AgentRandom agent(game);
    int n_playouts = argc > 1 ? std::stoi(argv[1]) : default_n_playouts;

    auto start = std::chrono::steady_clock::now();

    for (int i=0; i < n_playouts; ++i) {
        game.reset();
        playout_agent_random(game, agent);
    }

    std::cout << "\n    Agent Random:\n Time taken for "
              << n_playouts
              << " random playouts: "
              << std::setprecision(2)
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start).count()
              << "ms." << std::endl;

    start = std::chrono::steady_clock::now();

    Mcts mcts(game);

    game.reset();
    std::vector<Action> actions;
    game.compute_valid_actions(actions);
    const int n_playouts_per_actions = n_playouts / actions.size();

    for (int i=0; i<actions.size(); ++i) {
      mcts.sample(actions[i], n_playouts_per_actions);
    }

    std::cout << "\n    Agent Mcts:\n Time taken for "
              << n_playouts_per_actions * actions.size()
              << " random playouts: "
              << std::setprecision(2)
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start).count()
              << "ms." << std::endl;
}
