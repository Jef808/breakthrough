#include "mcts.h"
#include "types.h"
#include "bitboard.h"
#include "game.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <fstream>
#include <random>


std::unordered_map<Key, Node> TTable;

namespace {
/// Globals
    std::random_device rd;
    std::mt19937 eng{rd()};
    std::vector<Action> rollout_buffer;
}  // namespace

template<typename Cont>
typename Cont::value_type random_choice(Cont& cont) {
    auto d = cont.size();
    std::uniform_int_distribution<> dist(0, d - 1);
    return cont[dist(eng)];
}

/**
 * Return a pointer to node corresponding to @key
 * in the transposition table (construct it in place
 * with default values if not found).
 */
Node* Mcts::get_node(Key key) {
    return &TTable.try_emplace(key, key, 0).first->second;
}


Mcts::Mcts(Game& game)
    : m_game(game)
{
    reset(game);
}

void Mcts::reset(Game& game)
{
    std::fill(std::begin(m_states), std::end(m_states), StateData{});
    std::fill(std::begin(m_nodes), std::end(m_nodes), nullptr);
    std::fill(std::begin(m_edges), std::end(m_edges), nullptr);
    TTable.clear();
    m_game = game;
    reset_counters();
}

/**
 * After weighting down by @game.ply(), return 1.0 if @game is
 * a win for a @current_player, otherwise 0.0
 */
inline double eval_terminal(const Game& game, Color current_player) {
    assert(game.is_lost());
    return game.player_to_move() == current_player
        ? 0.0 //+ game.ply() / 300.0
        : 1.0;  //- game.ply();  / 300.0;
}

Action Mcts::best_action() {
    setup_root();

    assert(root().key == m_game.key());
    assert(current_node() == root());
    assert(!current_node().children.empty());

    Color us = m_game.player_to_move();
    int iter_counter = 0;

    while (iter_counter < n_iterations) {
        assert(m_game.key() == current_node().key);
        assert(current_node() == root());

        // Select the next leaf to be expanded
        select();

        assert(current_node() != root());

        double reward = 0.5;

        if (current_node().visits == 0) {
            expand(current_node());
        }
        else {
            assert(is_terminal(current_node()));
            reward = eval_terminal(m_game, us);
        }

        reward = (current_node().children[0].total / (current_node().children[0].visits + 1));

        ++iter_counter;
        backpropagate(reward);
    }

    assert(m_game.key() == root().key);
    assert(current_node() == root());

    return best_child(current_node(), By::visits)->action;
}

/**
 * Reset the tree pointers, store the current game
 * at its base and expand populate its children if needed
 */
void Mcts::setup_root() {
    m_nodes[0] = get_node(m_game.key());

    // Store a copy of the game's StateData at root position
    m_states[0] = *m_game.get_sd();

    // Make the game's StateData point to the newly created copy
    m_game.set_sd(&m_states[0]);

    if (m_nodes[0]->visits == 0)
        expand(*m_nodes[0]);

    nn = &m_nodes[1];
    ee = &m_edges[0];
    sd = &m_states[1];
}

/**
 * Return a pointer to the child maximizing the criterion
 * specified with @by
 */
Edge* Mcts::best_child(Node& node, By by) {
    return &*std::max_element(
        node.children.begin(),
        node.children.end(),
        [&](const auto& a, const auto& b) {
            return by == By::visits ? a.visits < b.visits                      // visits
                : by == By::ucb ? UCB(node, a) < UCB(node, b)                  // UCB
                : by == By::avg ? a.total/(a.visits+1) < b.total/(b.visits+1)  // avg
                : false;
        });
}

/**
 * Traverse the Mcts tree by choosing the best UCB child at
 * each step. When a leaf is found, return a pointer to it
 */
void Mcts::select() {
    while (current_node().visits > 0 && !current_node().children.empty()) {
        ++current_node().visits;
        Edge* best = best_child(current_node(), By::ucb);
        apply(*best);
    }

    ++selections_count;
}

/**
 * Recursively play random actions until the game is lost.
 */
template<bool Trace>
double rollout(Game& game, Action action);

template<>
double rollout<false>(Game& game, Action action) {

    StateData sd{};
    game.apply(action, sd);

    // Report a win if game is lost after playing the action.
    // The score reported is weighted down by the game ply.
    // We put a big weight so that we don't return score
    // valued at less than 0.5 on a winning line.
    if (game.is_lost()) {
        game.undo(action);
        return eval_terminal(game, game.player_to_move());
    }

    // Pick a random action
    game.compute_valid_actions(rollout_buffer);
    Action a = random_choice(rollout_buffer);

    // Swap reward value of a win
    // between 0.0 and 1.0 at each ply
    double reward = 1.0 - rollout<false>(game, a);

    game.undo(action);
    return reward;
}

template<>
double rollout<true>(Game& game, Action action) {

    StateData sd{};
    game.apply(action, sd);
    std::cerr << game.view() << std::endl;

    if (game.is_lost()) {
        game.undo(action);
        std::cerr << (game.player_to_move() == Color::white ? "WHITE" : "BLACK")
            << " wins, returning "
            << eval_terminal(game, game.player_to_move())
            << std::endl;
        return eval_terminal(game, game.player_to_move());
    }

    game.compute_valid_actions(rollout_buffer);
    Action a = random_choice(rollout_buffer);

    double reward = 1.0 - rollout<true>(game, a);

    game.undo(action);
    return reward;
}

/**
 * Sample an action by performing @count rollouts starting with
 * the given @edge.action.
 */
double Mcts::sample(Action action, int count, bool trace) {
    double ret = 0.0;
    for (int i=0; i<count; ++i) {
        double score = trace ? rollout<true>(m_game, action)
                     : rollout<false>(m_game, action);
        ret += score;
    }
    ++rollouts_count;
    return ret;
}

/**
 * Populate @node's children from @m_game's valid_actions()
 *
 * @Remark  We reuse the actions_buffer while sampling
 * so do not sample before entering every children!
 */
void Mcts::expand(Node& node) {
    m_game.compute_valid_actions(m_actions_buffer);

    std::transform(
        m_actions_buffer.begin(),
        m_actions_buffer.end(),
        std::back_inserter(node.children),
        [&](Action a){ return Edge{ a, sample(a, n_initial_samples) / n_initial_samples }; });

    std::sort(
        node.children.begin(),
        node.children.end(),
        [](const auto& a, const auto& b){ return a.total > b.total; });

    node.visits = 1;
    ++expansions_count;
}

void update_stats(Edge& edge, double reward) {
    edge.total += reward;
    ++edge.visits;
}

/**
 * Update the stats of every nodes on the
 * current branch after sampling a leaf.
 */
void Mcts::backpropagate(double reward) {
    assert(current_node() != root());

    while (current_node() != root()) {
        // Add the current reward to the previous edge's average
        update_stats(previous_edge(), reward);

        // Swap the reward from win to loss and vice versa
        reward = 1.0 - reward;

        // Undo the last action in @m_game.
        undo();
    }

    assert(m_game.key() == current_node().key);
}

/**
 * Compute the Upper Confidence Bound for the regret
 * associated to selecting @child next.
 */
double Mcts::UCB(const Node& parent, const Edge& child) {
    double ret = (child.total) / (1.0 + child.visits);
    ret += exp_cst * std::sqrt(std::log(parent.visits) / (child.visits + 1.0));
    return ret;
}

/**
 * Wrapper around @Game::apply()'s for applying
 * an action to also traverse it in the abstract
 * tree
 */
void Mcts::apply(Edge& edge) {
    // Push the edge on the edge stack
    *ee++ = &edge;

    // Apply it to the game, using @m_states[ply]
    // as the StateData object
    m_game.apply(edge.action, *sd++);

    // Locate/instantiate resulting state in
    // the tree and push it on the node stack
    *nn++ = get_node(m_game.key());
}

/**
 * Wrapper around @Game::undo()'s method
 * to manage the stacks @sd, @nn, @ee when
 * undoing an action
 */
void Mcts::undo() {
    assert(ee != &m_edges[0]);

    // Pop the edge stack
    --ee;

    // Undo the action
    m_game.undo((*ee)->action);

    // Pop the nodes stack
    --nn;

    // Pop the states stack
    --sd;
}

std::string_view string_of(const Edge& edge) {
    static std::string buf;
    std::stringstream ss;

    ss << string_of(edge.action) << ": " << edge.total << " / " << edge.visits;

    buf = ss.str();
    return buf;
}

void Mcts::recurse_graphviz(std::ostream& out, Node& node) {
    for (auto& e : node.children) {

        // Only draw expanded nodes
        if (e.visits == 0)
            continue;

        apply(e);

        out << node.key << " -> " << current_node().key
            << " [label=\"" << string_of(e) << "\"]\n";
        recurse_graphviz(out, current_node());

        undo();
    }
}

void Mcts::write_graphviz(std::string_view fn) {
    std::ofstream out{ fn.data() };
    if (!out) {
        std::cerr << "Failed to open " << fn.data();
        return;
    }

    m_game.reset();
    setup_root();

    out << "digraph {\n  "
        << root().key << " [label=\"root\"]\n";

    recurse_graphviz(out, current_node());

    out << '}' << std::endl;
}

void Mcts::print_counters(std::ostream& out) const {
    out << "Selections: " << selections_count << '\n'
        << "Expansions: " << expansions_count << '\n'
        << "Rollouts: "   << rollouts_count << '\n'
        << "Total number of nodes: " << TTable.size() << '\n'
        << std::endl;
}

void Mcts::reset_counters() {
    selections_count = 0;
    expansions_count = 0;
    rollouts_count = 0;
}

void Mcts::print_root_actions(std::ostream& out) {
    for (auto e : current_node().children) {
        out << string_of(e) << std::endl;
    }
}
