/// TODO: Extract as many hard-coded feature into a configurable parameter
/// and write a helper script to tune them automatically.

#include "types.h"
#include "epsilonGreedy.h"
#include "bitboard.h"
#include "game.h"

#include <algorithm>
#include <functional>
#include <random>
#include <iostream>


namespace {

    /// Globals
    std::random_device rd;
    std::mt19937 eng{rd()};

}  // namespace

struct TrueF {
    template<typename T>
    bool operator()(const T& t) { return true; }
};

template<typename Cont>
typename Cont::value_type random_choice(Cont& cont) {
    auto d = cont.size();
    std::uniform_int_distribution<> dist(0, d - 1);
    return cont[dist(eng)];
}

template<typename Cont, typename F = TrueF>
std::pair<bool, typename Cont::value_type> random_choice_with_predicate_old(Cont& cont, F f = TrueF{}) {
    auto d = cont.size();
    std::uniform_int_distribution<> dist(0, d - 1);
    auto ndx = dist(eng);
    int c = 0;

    while (!f(cont[ndx]) && c < d - 1) {
        std::swap(cont[c], cont[ndx]);
        std::uniform_int_distribution<> nex_dist(++c, d - 1);
        ndx = nex_dist(eng);
    }

    return std::make_pair(f(cont[ndx]), cont[ndx]);
}

template<typename Cont, typename F = TrueF>
std::pair<bool, typename Cont::value_type> random_choice_with_predicate(Cont& cont, F f = TrueF{}) {
    static std::vector<int> candidates_ndx;
    candidates_ndx.clear();
    for (auto i = 0; i < cont.size(); ++i) {
        if (f(cont[i])) candidates_ndx.push_back(i);
    }
    if (candidates_ndx.empty())
        return std::make_pair(false, cont[0]);
    auto ndx = random_choice(candidates_ndx);
    return std::make_pair(true, cont[ndx]);
}

/**
 * With probability @e, return false, and
 * with probability (1-@e), return true.
 */
bool should_explore(double e) {
    std::uniform_real_distribution<> dist(0.0, 1.0);
    if (dist(eng) > 1 - e)
        return true;
    return false;
}

/**
 * Evaluate the state of the game statically (without applying any action).
 *
 * Check for quick wins / losses, then consider the piece configuration on
 * both sides. Add bonus for phalanx, columns and levers that are protected
 * at least as many times as they are attacked.
 */
double static_eval(const Game& game) {
    int fastest_win_us = std::numeric_limits<int>::max();
    int fastest_win_them = std::numeric_limits<int>::max();

    Color us = game.player_to_move();
    Color them = opposite_of(us);
    Bitboard ours = game.pieces(us);
    Bitboard theirs = game.pieces(them);

    int our_score = 0;
    int their_score = 0;
    int lever_score = 0;

    auto phalanx = [&game](Color c, Square sq) {
        return bool(shift<Direction::left>(square_bb(sq)) & game.pieces(c));
    };
    auto column = [&game](Color c, Square sq) {
        return bool(shift<Direction::up>(square_bb(sq)) & game.pieces(c));
    };
    auto protected_lever = [&game](Color c, Square sq) {
        Bitboard lever = attacks_bb(c, sq) & game.pieces(c);
        return count(attacks_bb(c, sq) & game.pieces(c))
            >= count(attacks_bb(opposite_of(c), sq) & game.pieces(opposite_of(c)));
    };

    while (ours && fastest_win_us > 1) {
        Square sq = pop_lsb(ours);
        if (count(span_bb(us, sq) & theirs) < 2) {
            int plies_to_win = 7 - to_integral(relative(us, row_of(sq)));
            if (plies_to_win < fastest_win_us) {
                fastest_win_us = plies_to_win;
            }
        }
        our_score += (2 * phalanx(us, sq)) + column(us, sq);
        lever_score += 2 * protected_lever(us, sq);
    }
    if (fastest_win_us == 1)
        return 1.0;

    while (theirs && fastest_win_them > 2) {
        Square sq = pop_lsb(theirs);
        if (count(span_bb(them, sq) & ours) < 2) {
            int plies_to_win = 7 - to_integral(relative(them, row_of(sq))) + 1;
            if (plies_to_win < fastest_win_them) {
                fastest_win_them = plies_to_win;
            }
        }
        their_score += (2 * phalanx(them, sq)) + column(them, sq);
        lever_score -= 2 * protected_lever(them, sq);
    }
    if (fastest_win_them == 2)
        return 0.0;
    if (fastest_win_us == 3)
        return 0.8;

    int my_count = count(game.pieces(us));
    int their_count = count(game.pieces(them));
    double material_score = 0.5 + (my_count - their_count) / (2.0 * (my_count + their_count));
    double our_perf_score = 3 * my_count;
    double their_perf_score = 3 * their_count;
    double dlever_score = 0.5  + (lever_score + 16.0) / 16.0;
    double score = (0.5 + (2.0 * our_score - our_perf_score) / (4.0 * our_perf_score)
                    - (2.0 * their_score - their_perf_score) / (4.0 * their_perf_score));

    if (fastest_win_them == 4)
        return 0.33 * 0.35 + 0.33 * material_score + 0.33 * score;

    return 0.2 * dlever_score + 0.3 * score + 0.5 * material_score;
}


Agent::Agent(Game& game)
    : m_game{game}
{}

/**
 * Sample an action by performing @count rollouts starting with
 * the given @action.
 *
 * Invalidates @m_actions_buffer.
 */
double Agent::sample(Action action, int count) {
    double ret = 0;
    for (int i=0; i<count; ++i)
        ret += rollout(action);
    return ret;
}

/**
 * Recursively play random actions until the game is lost.
 *
 * Invalidates @m_rollout_buffer.
 */
double Agent::rollout(Action a) {

    StateData sd{};
    m_game.apply(a, sd);

    if (m_game.is_lost()) {
        // Report a winning score, weighted by the game ply.
        // We put a big weight so that we don't return scores
        // less than 0.5 even for very long winning lines.
        m_game.undo(a);
        return 1.0 - m_game.ply() / 300.0;
    }

    m_game.compute_valid_actions(m_actions_buffer);
    Action action = random_choice(m_actions_buffer);

    // A win changes from 0.0 to 1.0 at each ply!
    double reward = 1.0 - rollout(action);

    m_game.undo(a);
    return reward;
}

/**
 * Populate the @root_actions vector with all valid actions.
 */
void Agent::setup_rootactions() {
    root_actions.clear();
    m_game.compute_valid_actions(m_actions_buffer);
    std::transform(m_actions_buffer.begin(),
                   m_actions_buffer.end(),
                   std::back_inserter(root_actions),
                   [&](auto a){
                       return ExtAction{a};
                   });
}

/**
 * Find the appropriate action to defend against an
 * intruder threat found at square @th.
 */
Action Agent::defend_critical(Square th) {
    Color us = m_game.player_to_move();
    Color them = opposite_of(us);

    // Check if we can capture advantageously
    Bitboard attackers = attackers_bb(us, th) & m_game.pieces(us);
    if (attackers) {
        if (count(attackers) - count(attackers_bb(them, th) & m_game.pieces(them)) > 0) {
            return random_choice_with_predicate(
                root_actions,
                [&th](Action a){ return to_square(a) == th; }).second;
        }
    }

    bool safe = true;

    // Otherwise try to ensure that all squares in the intruder's path are protected.
    Bitboard forward_free = forward_bb(them, th) & m_game.no_pieces();
    if (forward_free) {
        Bitboard prot = us == Color::white
            ? attacks<Color::black>(forward_free)
            : attacks<Color::white>(forward_free);
        if (!(prot & m_game.pieces(us))) {
            auto [found, action] = random_choice_with_predicate(
                root_actions,
                [&prot](Action a){ return square_bb(to_square(a)) & prot; });
            if (found)
                return action;

            safe = false;
        }
    }

    if (safe) {
        Bitboard diagonals = attackers_bb(us, th);
        int n_open_diags = 0;
        Action _action = Action::none;

        while (diagonals) {
            Square diag = pop_lsb(diagonals);
            Bitboard diags_prot = attackers_bb(us, diag);

            // If already pin that diagonal, nothing to do
            if (diags_prot & m_game.pieces(us))
                continue;

            ++n_open_diags;
            auto [found, action] = random_choice_with_predicate(
                root_actions,
                [&diags_prot](Action a){ return square_bb(to_square(a)) & diags_prot; });
            if (found)
                _action = action;

            safe = false;
            break;
        }
        if (n_open_diags == 2) {
            std::cerr << "Got two open diagonals!" << std::endl;
            safe = false;
        }
        if (safe && _action != Action::none)
            return _action;
    }

    // If we didn't find a counter for the critical threat, we desperately try to find a quicker win!
    Bitboard runners = row_bb(row_of(frontmost_sq(us, m_game.pieces(us)))) & m_game.pieces(us);
    int smallest_span_count = std::numeric_limits<int>::max();

    // Pick an action from a runner's square having minimal amount of opposing pieces in its span.
    while (runners) {
        Square s = pop_lsb(runners);
        int c = count(span_bb(us, s) & m_game.pieces(them));
        if (c < smallest_span_count)
            smallest_span_count = c;
    }
    Action action = random_choice_with_predicate(root_actions, [&] (Action a){
        return count(span_bb(us, from_square(a)) & m_game.pieces(them)) == smallest_span_count;
    }).second;

    return action;
}

/**
 * Pick the next action to play.
 *
 * First check if we have direct wins or if we should defend against
 * one. If not, sample the valid actions @n_iterations times
 * according to an epsilon greedy policy before selecting the best
 * candidate according to the agent's @cmpGreater.
 */
Action Agent::best_action() {
    setup_rootactions();

    Color us = m_game.player_to_move();
    Color them = opposite_of(m_game.player_to_move());

    double static_score = static_eval(m_game);

    // If we have a win in 1
    if (static_score > 0.8) {
        Square sq = frontmost_sq(us, m_game.pieces(us));
        return *find_if(root_actions.begin(), root_actions.end(), [&sq](const auto& ra) {
            return from_square(ra) == sq;
        });
    }

    Bitboard critical = crit_rows(us) & m_game.pieces(them);
    if (critical) {
        Square th = frontmost_sq(them, m_game.pieces(them));
        auto action = defend_critical(frontmost_sq(them, m_game.pieces(them)));
        if (action != Action::none)
            return action;
    }

    // Initial samples
    for (auto& ra : root_actions) {
        ra.total_value = sample(ra, n_initial_samples);
        ra.n_visits = n_initial_samples;
    }

    // epsilon-greedy exploitation/exploration
    for (int i=0; i<n_iterations; ++i) {
        ExtAction* ra = nullptr;

        if (should_explore((m_game.ply() < 32 ? epsilon : m_game.ply() < 64 ? epsilon / 2 : epsilon / 4)))
            ra = &*std::find(root_actions.begin(), root_actions.end(), random_choice(root_actions));
        else
            ra = &*std::min_element(root_actions.begin(), root_actions.end(), cmpGreater);

        bool reward = sample(*ra);
        ra->update(reward);
    }

    return *std::min_element(root_actions.begin(), root_actions.end(), cmpGreater);
}
