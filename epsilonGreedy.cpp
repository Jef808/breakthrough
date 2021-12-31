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

    std::random_device rd;
    std::mt19937 eng{rd()};

    struct TrueF {
        template<typename T>
        bool operator()(const T& t) { return true; }
    };
}

Agent::Agent(Game& game)
    : m_game(game)
{
}

template<typename Cont>
typename Cont::value_type random_choice(Cont& cont) {
    auto d = cont.size();
    std::uniform_int_distribution<> dist(0, d - 1);
    return cont[dist(eng)];
}

template<typename Cont, typename F = TrueF>
std::pair<bool, typename Cont::value_type> random_choice_with_predicate(Cont& cont, F f = TrueF{}) {
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

/**
 * With probability e return false, and
 * with probability (1-e) return true.
 */
bool chance_node(double e) {
    std::uniform_real_distribution<> dist(0.0, 1.0);
    if (dist(eng) > 1 - e)
        return true;
    return false;
}

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
        return count(attacks_bb(c, sq) & game.pieces(c)) >= count(attacks_bb(opposite_of(c), sq) & game.pieces(opposite_of(c)));
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
    double score = 0.5 + (2.0 * our_score - our_perf_score) / (4.0 * our_perf_score) - (2.0 * their_score - their_perf_score) / (4.0 * their_perf_score);
    double dlever_score = 0.5  + (lever_score + 16.0) / 16.0;

    if (fastest_win_them == 4)
        return 0.33 * 0.35 + 0.33 * material_score + 0.33 * score;

    return 0.2 * dlever_score + 0.3 * score + 0.5 * material_score;
}

std::pair<bool, Action> defend_critical(const Game& game, Square th) {

    static std::vector<Action> actions;

    bool found_counter = false;
    Action action = Action::none;

    Color us = game.player_to_move();
    Color them = opposite_of(us);
    actions = game.valid_actions();

    // Check if we can capture advantageously
    Bitboard attackers = attackers_bb(us, th) & game.pieces(us);
    if (attackers) {
        if (count(attackers) - count(attackers_bb(them, th) & game.pieces(them)) > 0) {
            action = random_choice_with_predicate(actions, [&th]
                                                  (Action a){ return to_square(a) == th; }).second;
            found_counter = true;
        }
    }

    // Otherwise try to ensure that all squares in the intruder's path are protected.
    Bitboard forward_free = forward_bb(them, th) & game.no_pieces();
    if (forward_free) {
        Bitboard prot = us == Color::white
            ? attacks<Color::black>(forward_free)
            : attacks<Color::white>(forward_free);
        if (!(prot & game.pieces(us))) {
            auto [found, action] = random_choice_with_predicate(actions, [&prot]
                                                                (Action a){ return square_bb(to_square(a)) & prot; });
        }
    }

    Bitboard diags_prot = us == Color::white
        ? attacks<Color::black>(attackers_bb(us, th))
        : attacks<Color::white>(attackers_bb(us, th));
    if (!(diags_prot & game.pieces(us))) {
        auto [found, action] = random_choice_with_predicate(actions, [&diags_prot]
                                                            (Action a){ return square_bb(to_square(a)) & diags_prot; });
    }

    // If we didn't find a counter for the critical threat, we desperately try to find a quicker win!
    Bitboard runners = row_bb(row_of(frontmost_sq(us, game.pieces(us)))) & game.pieces(us);

    while (runners) {
        Square s = pop_lsb(runners);
        if (count(span_bb(us, s) & game.pieces(them)) < 2) {
            int plies_to_win = 7 - to_integral(relative(us, row_of(s)));
            if (plies_to_win < to_integral(relative(us, row_of(th))) + 1) {
                found_counter = true;
                action = random_choice_with_predicate(actions, [&s]
                                                      (Action a){ return to_square(a) == s; }).second;
            }
        }
    }

    return std::make_pair(found_counter, action);
}

double rollout(Game& game, Action a) {

    StateData sd{};
    game.apply(a, sd);

    if (game.is_lost())
        // Report a winning score, weighted by the game ply.
        // We put a big weight so that we don't return scores
        // less than 0.5 even for very long winning lines.
        game.undo(a);
        return 1.0 - game.ply() / 300.0;

    game.compute_valid_actions();
    Action action = random_choice(game.valid_actions());

    // A win changes from 0.0 to 1.0 at each ply!
    double reward = 1.0 - rollout(game, action);

    game.undo(a);
    return reward;
}

double sample(Game& game, Action a, int n=1) {
    double ret = 0;
    for (int i=0; i<n; ++i)
        ret += rollout(game, a);
    return ret;
}

/**
 * Perform a random playout and return true iff ~game_.player_to_move()~ won.
 */
// double sample(Game& game, Action a, int n=1) {

//     const Color us = game.player_to_move();
//     const Color them = opposite_of(us);
//     double ret = 0;
//     static std::vector<Action> actions;


//     for (int i=0; i<n; ++i) {
//         Action action = a;
//         game.apply(action);

//         int init_ply = game.ply();

//         while (!game.is_lost()) {
//             game.compute_valid_actions();
//             actions = game.valid_actions();

//             action = random_choice(actions);
//             game.apply(action);
//         }

//         // At this point the game is over
//         ret += (game.player_to_move() != us) * (1 - (game.ply() - init_ply) / 150.0);
//     }
//     return ret;
// }

/**
 * Leaves the action with the best static evaluation at index 0.
 */
void Agent::setup_rootactions() {
    root_actions.clear();
    std::transform(m_game.valid_actions().begin(),
                   m_game.valid_actions().end(),
                   std::back_inserter(root_actions),
                   [](auto a){ return ExtAction{a}; });

    ExtAction* best_ra = &root_actions[0];
    double best_score = 0.0;

    for (auto& ra : root_actions) {
        StateData sd{};
        m_game.apply(ra, sd);
        ra.prior = 1.0 - static_eval(m_game);
        ra.total_value = 0;
        ra.n_visits = 1;
        if (ra.prior > best_score) {
            best_ra = &ra;
            best_score = ra.prior;
        }
        m_game.undo(ra);
    }

    std::swap(root_actions[0], *best_ra);
}

Action Agent::best_action() {

    setup_rootactions();

    auto get_score = [](const auto& ra) {
        return (ra.total_value + ra.prior) / (ra.n_visits + 1);
    };

    Color us = m_game.player_to_move();
    Color them = opposite_of(m_game.player_to_move());
    ExtAction* best_ra = &root_actions[0];
    double best_score = 0.0;

    // If we have a win in 1
    if (best_score > 0.8) {
        Square sq = frontmost_sq(us, m_game.pieces(us));
        return *find_if(root_actions.begin(), root_actions.end(), [&sq](const auto& ra) {
            return from_square(ra) == sq;
        });
    }

    Bitboard critical = crit_rows(us) & m_game.pieces(them);
    if (critical) {
        Square th = frontmost_sq(them, m_game.pieces(them));
        auto [found, action] = defend_critical(m_game, frontmost_sq(them, m_game.pieces(them)));
        if (found)
            return action;
    }

    // To ensure that every moves get some exposure.
    for (auto& ra : root_actions) {
        ra.update(sample(m_game, ra, 20), 20);
        if (get_score(ra) > best_score) {
            best_score = get_score(ra);
            best_ra = &ra;
        }
    }

    // epsilon-greedy exploitation/exploration
    for (int i=0; i<n_iterations; ++i) {
        Action action = Action::none;
        if (chance_node((m_game.ply() < 32 ? epsilon : m_game.ply() < 64 ? epsilon / 2 : epsilon / 4)))
            action = random_choice(m_game.valid_actions());
        else
            action = *best_ra;

        bool reward = sample(m_game, action);
        best_ra->update(reward);

        best_ra = &*std::max_element(root_actions.begin(), root_actions.end(), [&get_score](const auto& ra, const auto& rb) {
            return get_score(ra) < get_score(rb);
        });
        best_score = get_score(*best_ra);
    }

    return *best_ra;
}
