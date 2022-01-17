#include "mcts.h"
#include "game.h"
#include "types.h"
#include "agentRandom.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string_view>

constexpr auto graphviz_fn = "tree.dot";

void any_key() {
    char ch = '\0';
    std::cout << "Press any key to continue..." << std::endl;
    std::cin >> ch; std::cin.ignore();
}

class MctsTest : public Mcts {
public:
    MctsTest(Game& game)
        : Mcts { game }
        , rand { game }
    {
    }

    bool test_setuproot() {
        bool res = true;

        setup_root();
        if (current_node() != root()) {
            std::cout << "Current node: " << current_node().key
                << "Root: " << root().key
                << std::endl;
            return false;
        }
        if (current_node().key != m_game.key()) {
            std::cout << "Current node: " << current_node().key
                << "Game: " << m_game.key()
                << std::endl;
            return false;
        }

        return res;
    }

    void expand_all_child()
    {
        setup_root();

        for (auto& child : root().children) {

            std::cout << "applying " << string_of(child.action) << std::endl;

            apply(child);
            ++child.visits;
            Node* node = get_node(m_game.key());
            expand(*node);

            std::cout << m_game.view() << "\nchildren: \n   ";
            for (const auto& gchild : node->children)
                std::cout << string_of(gchild.action) << ' ';

            std::cout << std::endl;
            undo();
        }

        std::ofstream ofs{ graphviz_fn };
        write_graphviz(ofs);
    }

    bool interactive_test()
    {
        setup_root();



        bool ret = true;

        std::cout << m_game.view() << "Options:\n"
                  << "'S<action>' to sample\n"
                  << "'A' to view root actions\n"
                  << "'T' to test setup_root()\n"
                  << "'P<action>' to play an action'\n"
                  << "'M' to play MCTS's choice\n"
                  << "'R' to play a random action\n"
                  << "'U' to undo the previous action\n"
                  << "'Q' to quit"
                  << std::endl;

        std::getline(std::cin, buf);
        if (buf.size() == 5) {
            std::string_view sv(buf.begin() + 1, buf.end());
            Action a = action_of(sv.data());
            auto it = std::find(current_node().children.begin(),
                                current_node().children.end(),
                                a);
            if (it == current_node().children.end()) {
                std::cout << "Error... Could not find children corresponding to "
                          << sv << std::endl;
                return ret;
            }
            switch (buf[0])
            {
                case 'P':
                case 'p':
                    apply(*it);
                    std::cout << m_game.view() << std::endl;
                    std::cout << "Chosen action: " << string_of(a) << std::endl;
                    break;
                case 'S':
                case 's':
                    double score = sample(a, 1, true);
                    std::cout << "Score: " << score << std::endl;
                    break;
            }
        }
        else if (buf.size() == 1) {
            switch (buf[0])
            {
                Edge e;
                case 'R':
                case 'r':
                    e = *std::find(current_node().children.begin(),
                                   current_node().children.end(),
                                   rand.best_action());
                    apply(e);
                    std::cout << m_game.view() << std::endl;
                    std::cout << "Chosen action: " << string_of(e.action) << std::endl;
                    break;
                case 'M':
                case 'm':
                    e = *std::find(current_node().children.begin(),
                                        current_node().children.end(),
                                        best_action());
                    apply(e);
                    std::cout << m_game.view() << std::endl;
                    std::cout << "Chosen action: " << string_of(e.action)
                              << " with avg value of "
                              << e.total / e.visits + 1
                              << " after "
                              << e.visits << " visits."
                              << std::endl;
                    break;
                case 'U':
                case 'u':
                    undo();
                    std::cout << m_game.view() << std::endl;
                    break;
                case 'A':
                case 'a':
                    print_root_actions(std::cout);
                    break;
                case 'T':
                case 't':
                    if (test_setuproot()) {
                        std::cout << "OK" << std::endl;
                    } else {
                        std::cout << "Test failed" << std::endl;
                    }
                    break;
                case 'Q':
                case 'q':
                    ret = false;
                    break;
            }
        }
        return ret;
    }

private:
    AgentRandom rand;
    std::string buf;
};


constexpr auto n_iterations = 300;
constexpr auto exp_cst = 1.4;
constexpr auto n_initial_samples = 1;

int main(int argc, char* argv[])
{
    Game::init();
    //StateData states[max_depth + 1], *sd = &states[0];

    Game game;
    MctsTest mcts(game);
    AgentRandom random(game);

    mcts.set_exp_cst(exp_cst);
    mcts.set_n_init_samples(n_initial_samples);
    mcts.set_n_iterations(n_iterations);

    mcts.test_setuproot();

    game.reset();
    mcts.reset(game);

    while (mcts.interactive_test()) {
        if (game.is_lost()) {
            std::cout << (game.player_to_move() == Color::white ? "BLACK" : "WHITE")
                      << " wins!" << std::endl;
            game.reset();
            mcts.reset(game);
        }
    }

    // mcts.write_graphviz(graphviz_fn);

    return EXIT_SUCCESS;
}
