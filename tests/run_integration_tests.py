#!/usr/bin/env python3
from util import get_project_root_directory

EXECUTABLE_NAME = "output"

def main():
    # todo: the path library is the "correct" way to do this
    root_dir = get_project_root_directory()
    executable_path = f"{root_dir}/{EXECUTABLE_NAME}"
    print(executable_path)

if __name__ == "__main__":
    main()