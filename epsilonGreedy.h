#ifndef AGENT_H_
#define AGENT_H_

#include "game.h"

struct ExtAction {
    Action action;
    double total_value;
    int n_visits;
    explicit ExtAction(Action a)
        : action{a}, total_value{0}, n_visits{0} {}
    ExtAction(Action a, double val, int vis)
        : action{a}, total_value{val}, n_visits{vis} {}
    operator Action() const { return action; }
    void update(double stats, int visits = 1) {
        total_value += stats;
        n_visits += visits;
    }
};

struct CmpActionsGreater {
    bool operator()(const ExtAction& a, const ExtAction& b) {
        return a.total_value / a.n_visits > b.total_value / b.n_visits;
    }
};

class Agent {
public:
    Agent(Game&);
    Action best_action();

    void set_epsilon(double e) { epsilon = e; }
    void set_n_iterations(int n) { n_iterations = n; }
    void set_n_initial_samples(int n) { n_initial_samples = n; }
    double sample(Action a, int count=1);

private:
    Game& m_game;
    std::vector<ExtAction> root_actions;
    std::vector<Action> m_actions_buffer;
    std::vector<Action> m_rollout_buffer;
    CmpActionsGreater cmpGreater = CmpActionsGreater{};
    double epsilon = 0.1;
    int n_iterations = 5000;
    int n_initial_samples = 10;

    void setup_rootactions();
    Action defend_critical(Square);
    double rollout(Action);
};

#endif // AGENT_H_
