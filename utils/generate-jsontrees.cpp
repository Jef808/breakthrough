#include "types.h"
#include "mcts.h"
#include "game.h"
#include "agentRandom.h"
#include "config.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <string_view>
#include <utility>


bool save_tree(Mcts& mcts, std::filesystem::path fp) {
    std::ofstream ofs{fp};

    if (!ofs) {
        std::cerr << "Failed to open output file " << fp;
        return false;
    }

    //mcts.write_graphviz(ofs);
    mcts.write_json_tree(ofs);
    return true;
}

int main(int argc, char* argv[]) {
    auto [config_ok, config] = get_config();

    if (!config_ok) {
        std::cerr << "Failed to load config" << std::endl;
        return EXIT_FAILURE;
    }

    Game::init();
    Game game;
    Mcts mcts(game);
    AgentRandom random(game);
    StateData states[max_depth], *sd = &states[0];

    /// Return an absolute path to output filename of type
    /// std::filesystem::path.
    auto make_output_fp = [&jsontree_datadir=config.jsontree_datadir,
                           &jsontree_fn=config.jsontree_fn] (int ply)
    {
        auto ret = jsontree_datadir;
        ret /= jsontree_fn;
        std::ostringstream ss;
        ss << ply << ".json";
        //ret += "_" + std::to_string(ply) + ".json";
        ret += ss.str();
        return ret;
    };

    mcts.set_n_iterations(config.iterations);
    mcts.set_exp_cst(config.exp_cst);
    mcts.set_n_init_samples(config.init_samples);

    while (!game.is_lost()) {
        Action a;
        bool mcts_turn = false,
             rand_turn = false;

        if (game.player_to_move() == Color::white) {
            a = mcts.best_action();
            mcts_turn = true;
        }
        else {
            a = random.best_action();
            rand_turn = true;
        }

        // Update the tree after Mcts plays
        if (mcts_turn) {
            std::cout << "Game ply: " << game.ply() << "\n\n";
            mcts.print_counters(std::cout);
            auto fp = make_output_fp(game.ply());
            std::cout << "\nWriting jsontree file "
                      << fp << "..." << std::endl;;
            bool ok = save_tree(mcts, make_output_fp(game.ply()));
            if (!ok) {
                std::cerr << "Writing jsontree failed, aborting..." << std::endl;
                return EXIT_FAILURE;
            }
        }

        game.apply(a, *sd++);
    }

    return EXIT_SUCCESS;
}
