#include "game.h"

#include <random>

struct AgentRandom {

    Game& m_game;
    static inline std::vector<Action> actions;

    AgentRandom(Game& game) :
        m_game(game)
    {
    }

    std::random_device rd;
    std::mt19937 eng{rd()};

    Action sample() {
        actions.clear();
        m_game.compute_valid_actions(actions);
        std::uniform_int_distribution<> dist(0, AgentRandom::actions.size() - 1);
        return AgentRandom::actions[dist(eng)];
    }

    Action best_action() {
        return sample();
    }
};
