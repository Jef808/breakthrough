#ifndef AGENT_H_
#define AGENT_H_

#include "game.h"

class Agent {
public:
    Agent(Game&);
    Action best_action();
    Action random_action();
private:
    Game& m_game;
};

#endif // AGENT_H_
