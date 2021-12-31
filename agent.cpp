#include "types.h"
#include "agent.h"
#include "game.h"

#include <algorithm>
#include <random>
#include <iostream>


namespace {

    std::random_device rd;
    std::mt19937 eng{rd()};

}

Agent::Agent(Game& game)
    : m_game(game)
{
}

template<typename Cont>
auto random_choice(const Cont& cont) {
    auto d = cont.size();
    std::uniform_int_distribution<> dist(0, d - 1);
    return cont[dist(eng)];
}

/**
 * With probability e return false, and
 * with probability (1-e) return true.
 */
bool chance_node(double e) {
    std::uniform_real_distribution<> dist{};
    if (dist(eng) > 1 - e)
        return true;
    return false;
}

// double static_eval(const Game& game) {

// }

/**
 * Perform a random playout and return true iff ~game_.player_to_move()~ won.
 */
int sample(const Game& game_, Action a, int n=1) {
    const Color us = game_.player_to_move();
    int ret = 0;
    for (int i=0; i<n; ++i) {
        Game game = game_;
        Action action = a;
        game.apply(action);
        while (!game.is_lost()) {
            game.compute_valid_actions();
            action = random_choice(game.valid_actions());
            game.apply(action);
        }
        ret += game.player_to_move() != us;
    }
    return ret;
}

Action Agent::best_action() {
    root_actions.clear();
    std::transform(m_game.valid_actions().begin(),
                   m_game.valid_actions().end(),
                   std::back_inserter(root_actions),
                   [](auto a){ return ExtAction{a}; });

    for (auto& ra : root_actions) {
        int n_wins = sample(m_game, ra, 10);
        ra.update(n_wins, 10);
    }

    std::swap(root_actions[0], *std::max_element(root_actions.begin(), root_actions.end()));

    for (int i=0; i<n_iterations; ++i) {
        Action action = Action::none;
        if (chance_node(epsilon))
            action = random_choice(m_game.valid_actions());
        else
            action = root_actions[0];

        bool reward = sample(m_game, action);
        ExtAction& ra = *std::find(root_actions.begin(), root_actions.end(), action);
        ra.update(reward);

        if (ra.total_value / ra.n_visits > root_actions[0].total_value / root_actions[0].n_visits)
            std::swap(root_actions[0], ra);
    }

    return root_actions[0];
}
