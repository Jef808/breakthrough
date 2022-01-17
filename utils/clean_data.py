#!/usr/bin/env python3

from pathlib import Path
import json

project_dir = Path.home() / "projects" / "breakthrough"

if __name__ == '__main__':
    jsontree_datadir = Path()
    with open(project_dir / "default_config.json", "r") as config_fp:
        config = json.load(config_fp)
        jsontree_datadir = project_dir / Path(config["jsontree_datadir"])
    list(map(lambda f: f.unlink(), jsontree_datadir.glob("*.json")))
