#!/usr/bin/env python3
from util import get_project_root_directory, printred, printgreen

import os
import sys
import subprocess
import filecmp
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor

EXECUTABLE_NAME = "output"
EXECUTABLE_PATH = None
VERBOSE = False
ANY_FAILED = False

ARGV = "12 13 14"

fmt_msg = lambda msg: f': "{msg}"' if msg != "" else ""


def exec_all_and_save_output(source_files: list[Path], output_dir: Path):
    def run_and_save(source_file: Path):
        result = subprocess.run(
            [EXECUTABLE_PATH, "--exec", f"--input={source_file}", f"--argv={ARGV}"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        output_path = output_dir / f"{source_file.name}.out"
        with output_path.open("w") as f:
            f.write(result.stdout)
            f.write("\n--- STDERR ---\n")
            f.write(result.stderr)

    with ThreadPoolExecutor() as executor:
        executor.map(run_and_save, source_files)


def comp_and_exec_all_and_save_output(
    source_files: list[Path], output_dir: Path, exec_path: Path, comp_script: Path
):
    for source_file in source_files:
        # compile first
        comp_result = subprocess.run(
            [comp_script, source_file],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        output_path = output_dir / f"{source_file.name}.out"

        # collect output - if compile error, then write it out to file
        output = f"{comp_result.stdout}\n--- STDERR ---\n{comp_result.stderr}"
        if comp_result.stderr:
            output = f"COMPILE ERROR:\n{output}"
        else:
            # if no compile error, run the resulting executable for the
            # final result.
            run_result = subprocess.run(
                [exec_path] + ARGV.split(),
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            )
            output = f"{run_result.stdout}\n--- STDERR ---\n{run_result.stderr}"

        with output_path.open("w") as f:
            f.write(output)


def main():
    global EXECUTABLE_PATH, VERBOSE

    if "-v" in sys.argv[1:]:
        VERBOSE = True

    root_dir = get_project_root_directory()
    EXECUTABLE_PATH = f"{root_dir}/{EXECUTABLE_NAME}"

    # run rebuild.sh here
    rebuild_script_path = f"{root_dir}/rebuild.sh"
    subprocess.run(
        [rebuild_script_path],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    comp_script_path = Path(f"{root_dir}/scripts/comp2.sh")
    compiled_exec_path = Path(f"{root_dir}/.work/compiled_exec")

    temp_dir = Path(f"{root_dir}/temp")
    exec_dir = temp_dir / "exec"
    comp_dir = temp_dir / "comp"
    for path in (temp_dir, exec_dir, comp_dir):
        path.mkdir(parents=True, exist_ok=True)

    # for every .src file in example_program_test_dir
    example_program_test_dir = f"{root_dir}/examples"
    dir_path = Path(example_program_test_dir)
    source_files = [f for f in dir_path.glob("*.src") if f.is_file()]

    # run all the source files using the interpreter, collect the output in exec_dir
    exec_all_and_save_output(source_files, exec_dir)
    # run all the source files using the compiler, collect the output in comp_dir
    comp_and_exec_all_and_save_output(
        source_files, comp_dir, compiled_exec_path, comp_script_path
    )

    # compare each corresponding <source_file>.out file
    # print the number that are identical, and the number that differ
    # then print the filenames of the ones that differ
    total_files = len(source_files)
    identical = 0
    different = 0
    different_files = []

    for src_file in source_files:
        file_name = f"{src_file.name}.out"
        exec_out = exec_dir / file_name
        comp_out = comp_dir / file_name

        if exec_out.exists() and comp_out.exists():
            if filecmp.cmp(exec_out, comp_out, shallow=False):
                identical += 1
            else:
                different += 1
                different_files.append(src_file.name)
        else:
            # handle missing output (optional)
            different += 1
            different_files.append(src_file.name)

    printgreen(f"Identical outputs in: {identical} / {total_files} files.")
    printred(f"Differing outputs in: {different} / {total_files} files")

    if different_files:
        print("Files with differing output:")
        for fname in different_files:
            print(f"  - {fname}")


if __name__ == "__main__":
    main()
