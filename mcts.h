#ifndef MCTS_H_
#define MCTS_H_

#include "types.h"
#include "game.h"

#include <iosfwd>
#include <string_view>
#include <vector>
#include <unordered_map>


struct Edge {
    Edge() = default;
    explicit Edge(Action a)
        : action{a}, visits{0}, total{0.0} {}
    Edge(Action a, double t)
        : action{a}, visits{0}, total{t} {}
    Action action;
    int visits;
    double total;
    bool operator==(Action a) { return action == a; }
};

struct Node {
    Node() = default;
    Node(Key k, int v) : key{k}, visits{v} {}
    Key key;
    int visits;
    std::vector<Edge> children;
    bool operator==(const Node& other) const { return key == other.key; }
};

enum class By {
    visits, ucb, avg
};

class Mcts {
public:
    Mcts(Game& game);
    double sample(Action action, int count=1, bool trace=false);
    Action best_action();
    void reset(Game& game);

    void set_n_iterations(int n);
    void set_exp_cst(double c);
    void set_n_init_samples(int n);
    void write_graphviz(std::string_view fn);
    void print_counters(std::ostream&) const;
    void print_root_actions(std::ostream&);
    void reset_counters();

protected:
    void setup_root();
    Edge* best_child(Node& parent, By by);
    void select();
    void expand(Node& node);
    double UCB(const Node& parent, const Edge& child);
    void backpropagate(double reward);

    Node* get_node(Key key);
    Node& root();
    Node& current_node();
    Edge& previous_edge();
    void apply(Edge& a);
    void undo();
    bool is_terminal(const Node&) const;

    void recurse_graphviz(std::ostream&, Node& node);

    Game& m_game;

private:
    std::vector<Action> m_actions_buffer;
    StateData m_states[max_depth], *sd = &m_states[0];
    Node* m_nodes[max_depth], **nn = &m_nodes[0];
    Edge* m_edges[max_depth], **ee = &m_edges[0];

    double exp_cst = 1.4;
    int n_initial_samples = 1;
    int n_iterations = 500;

    int rollouts_count = 0;
    int expansions_count = 0;
    int selections_count = 0;
};

extern std::unordered_map<Key, Node> TTable;

inline bool Mcts::is_terminal(const Node& node) const { return node.children.empty() && node.visits > 0; }
inline Node& Mcts::root() { return *m_nodes[0]; }
inline Node& Mcts::current_node() { return **(nn - 1); }
inline Edge& Mcts::previous_edge() { return **(ee - 1); }
inline void Mcts::set_n_iterations(int n) { n_iterations = n; }
inline void Mcts::set_exp_cst(double c) { exp_cst = c; }
inline void Mcts::set_n_init_samples(int n) { n_initial_samples = n; }


#endif // MCTS_H_
