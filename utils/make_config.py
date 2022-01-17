#!/usr/bin/env python3

import json

default_config = {"iterations": 600,
                  "exp_cst": 1.0,
                  "init_samples": 1,
                  "dump_tree": True,
                  "jsontree_datadir": "view/data/jsontree",
                  "jsontree_fn": "jsontree_ply_",
                  "max_nodes": -1}

if __name__ == '__main__':
    with open("../default_config.json", "w") as file:
        json.dump(default_config, file, indent=4)
