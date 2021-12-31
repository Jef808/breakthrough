#ifndef AGENT_H_
#define AGENT_H_

#include "game.h"

class Agent {

        struct ExtAction {
        Action action;
        double prior;
        double total_value;
        int n_visits;
        explicit ExtAction(Action a)
            : action{a}, total_value{0}, n_visits{0} {}
        operator Action() const { return action; }
        //bool operator==(const Action a) { return action == a; }
        void update(double stats, int visits = 1) {
            total_value += stats;
            n_visits += visits;
        }
    };

public:
    Agent(Game&);
    Action best_action();

    void set_epsilon(double e) { epsilon = e; }
    void set_n_iterations(int n) { n_iterations = n; }
private:
    Game& m_game;
    std::vector<ExtAction> root_actions;
    double epsilon = 0.1;
    int n_iterations = 5000;
};

#endif // AGENT_H_
