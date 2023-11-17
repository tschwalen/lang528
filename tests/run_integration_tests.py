#!/usr/bin/env python3
from util import get_project_root_directory

import os
import subprocess

EXECUTABLE_NAME = "output"

def main():
    # todo: the path library is the "correct" way to do this
    root_dir = get_project_root_directory()
    executable_path = f"{root_dir}/{EXECUTABLE_NAME}"
    example_dir = f"{root_dir}/examples"

    # rudimentary parser integration test
    total = 0
    errors = 0
    for example_program in os.listdir(example_dir):
        res = subprocess.run(
            [executable_path, 
            "--parse",
            f"--input={example_dir}/{example_program}"]
        )
        total += 1
        if res.returncode != 0:
            errors += 1

    print(f"{(total - errors)}/{total} example programs parsed without errors")

if __name__ == "__main__":
    main()