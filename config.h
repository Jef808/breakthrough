#ifndef CONFIG_H_
#define CONFIG_H_

#include <iostream>
#include <string_view>
#include <fstream>
#include <filesystem>

#include "nlohmann/json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

const fs::path home_dir = []{
    std::ostringstream ss{ std::getenv("HOME") };
    return fs::path(ss.str());
}();
const fs::path project_dir = home_dir / "projects/breakthrough/";


struct Config {
    bool load(std::string_view fp) {
        std::ifstream ifs{fp.data()};
        if (!ifs)
            return false;
        json j;
        ifs >> j;

        std::cout << "Loading following config:\n"
                  << std::setw(4) << j;

        iterations = j["iterations"];
        exp_cst = j["exp_cst"];
        init_samples = j["init_samples"];

        dump_tree = j["dump_tree"];
        jsontree_datadir = project_dir / j["jsontree_datadir"];
        jsontree_fn = j["jsontree_fn"];
        max_nodes = j["max_nodes"];

        std::cout << "Config successfully loaded" << std::endl;

        return true;
    }

    int iterations = 300;
    double exp_cst = 1.4;
    int init_samples = 1;

    bool dump_tree = false;
    std::filesystem::path jsontree_datadir = "view/data/jsontree";
    std::string jsontree_fn = "jsontree_ply_";
    int max_nodes = 1000;
};

std::pair<bool, Config> get_config(std::string_view fp = "default_config.json") {
    Config config;
    bool success = true;
    success = config.load(fp);
    return { success, config };
}

#endif // CONFIG_H_
