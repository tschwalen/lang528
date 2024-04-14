#!/usr/bin/env python3
from util import get_project_root_directory

import os
import subprocess

EXECUTABLE_NAME = "output"
EXECUTABLE_PATH = None

def run_e2e_test(directory: str, file: str):
    print(f"Running {file}")
    res = subprocess.run(
        [EXECUTABLE_PATH, "--exec", f"--input={directory}/{file}"],
        stdout=subprocess.PIPE, 
        stderr=subprocess.PIPE
    )

    stdout = res.stdout.decode("utf-8")
    stderr = res.stderr.decode("utf-8")
    retcode = res.returncode

    failure_expected = False
    for output_line in stdout.split("\n"):
        if output_line.startswith("#"):
            parts = output_line.split("#")
            macro = parts[1] # pass, fail, log, etc.
            msg = parts[2].strip()

            if macro == "PASS":
                print("\033[32m - Passed check" + (f": \"{msg}\"" if msg != "" else "") + "\033[0m")
            elif macro == "EXPECT":
                # very basic, if this macro is encountered we expect a non-zero exit code
                # this runs into an issue where the test might fail in ways other than we
                # expected, but it works ok for now
                failure_expected = True
                if retcode != 0:
                    print("\033[32m - Passed expect" + (f": \"{msg}\"" if msg != "" else "") + "\033[0m")
                else:
                    print("\033[31m - Failed expect" + (f": \"{msg}\"" if msg != "" else "") + "\033[0m")
            elif macro == "FAIL":
                print("\033[31m - Failed check" + (f": \"{msg}\"" if msg != "" else "") + "\033[0m")

    if not failure_expected and retcode !=0:
        print("\033[31m - Unexpected nonzero exit code\033[0m")



def main():
    global EXECUTABLE_PATH

    root_dir = get_project_root_directory()
    EXECUTABLE_PATH = f"{root_dir}/{EXECUTABLE_NAME}"
    e2e_test_dir = f"{root_dir}/examples/e2e"

    for e2e_test in os.listdir(e2e_test_dir):
        run_e2e_test(e2e_test_dir, e2e_test)
    

if __name__ == "__main__":
    main()