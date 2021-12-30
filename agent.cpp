#include "types.h"
#include "agent.h"
#include "game.h"

#include <algorithm>
#include <random>

Agent::Agent(Game& game)
    : m_game(game)
{}

auto get_random(const std::vector<Action>& actions) {
    static std::random_device rd;
    static std::mt19937 eng{rd()};
    assert(!actions.empty());

    std::uniform_int_distribution<> dist(0, actions.size() - 1);
    return actions[dist(eng)];
}

Action Agent::best_action() {
    return get_random(m_game.valid_actions());
}
