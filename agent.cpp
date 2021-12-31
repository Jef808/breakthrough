#include "types.h"
#include "agent.h"
#include "game.h"

#include <algorithm>
#include <random>
#include <iostream>


Agent::Agent(Game& game)
    : m_game(game)
{}

Action Agent::random_action() {
    static std::random_device rd;
    static std::mt19937 eng{rd()};

    auto d = m_game.valid_actions().size();
    std::uniform_int_distribution<> dist(0, d - 1);

    return m_game.valid_actions()[dist(eng)];
}

Action Agent::best_action() {

}
