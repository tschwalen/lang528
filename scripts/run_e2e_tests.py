#!/usr/bin/env python3
from util import get_project_root_directory, printred, printgreen

import os
import sys
import subprocess

EXECUTABLE_NAME = "output"
EXECUTABLE_PATH = None
VERBOSE = False
ANY_FAILED = False
COMPILE = False
COMPILE_ERROR_PREFIX = "# @COMPILE_ERROR@"

fmt_msg = lambda msg: f': "{msg}"' if msg != "" else ""


def report_pass(event: str, msg: str):
    if VERBOSE:
        printgreen(f"- Passed {event}{fmt_msg(msg)}")


def report_fail(event: str, msg: str = ""):
    printred(f"- Failed {event}{fmt_msg(msg)}")


def run_e2e_test_compiled(directory: str, file: str, root_dir: str):
    global ANY_FAILED
    no_failures = True
    print(f"Compiling and running {file}")

    # Some tests include intentional errors that
    # will surface at compile time. Here we capture
    # the header comment to check for this.
    input_file_path = f"{directory}/{file}"
    expected_compile_error_message = None
    with open(input_file_path, "r") as f:
        first_line = f.readline()
        if first_line.startswith(COMPILE_ERROR_PREFIX):
            expected_compile_error_message = first_line.strip("\n").removeprefix(
                COMPILE_ERROR_PREFIX
            )

    # compile the target file
    outfile_path = f"{root_dir}/a.out"
    compiler_subproc = subprocess.run(
        [
            EXECUTABLE_PATH,
            "--comp-e2e",
            f"--input={directory}/{file}",
            f"--output={outfile_path}",
        ],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    # handle compile errors (any nonzero exit)
    if compiler_subproc.returncode != 0:
        if expected_compile_error_message is not None:
            report_pass("check", expected_compile_error_message)
        else:
            report_fail(f"due to compile error in {file}")
            if VERBOSE:
                stdout = compiler_subproc.stdout.decode("utf-8")
                stderr = compiler_subproc.stderr.decode("utf-8")
                print(f"COMPILER STDOUT:\n{stdout}\n#\nEND OF STDOUT\n#")
                print(f"COMPILER STDERR:\n{stderr}\n#\nEND OF STDERR\n#")
            no_failures = False
    else:
        res = subprocess.run(
            [outfile_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

        stdout = res.stdout.decode("utf-8")
        stderr = res.stderr.decode("utf-8")
        retcode = res.returncode

        failure_expected = False
        for output_line in stdout.split("\n"):
            if output_line.startswith("#"):
                parts = output_line.split("#")
                macro = parts[1]  # pass, fail, log, etc.
                msg = parts[2].strip()

                if macro == "PASS":
                    report_pass("check", msg)
                elif macro == "EXPECT":
                    # very basic, if this macro is encountered we expect a non-zero exit code
                    # this runs into an issue where the test might fail in ways other than we
                    # expected, but it works ok for now
                    failure_expected = True
                    if retcode != 0:
                        report_pass("expect", msg)
                    else:
                        report_fail("expect", msg)
                        no_failures = False
                elif macro == "FAIL":
                    report_fail("check", msg)
                    no_failures = False

    if no_failures:
        printgreen("** TEST PASSED - NO FAILURES **")
    else:
        ANY_FAILED = True


def run_e2e_test(directory: str, file: str):
    global ANY_FAILED
    no_failures = True
    print(f"Running {file}")
    res = subprocess.run(
        [EXECUTABLE_PATH, "--exec", f"--input={directory}/{file}"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    stdout = res.stdout.decode("utf-8")
    stderr = res.stderr.decode("utf-8")
    retcode = res.returncode

    failure_expected = False
    for output_line in stdout.split("\n"):
        if output_line.startswith("#"):
            parts = output_line.split("#")
            macro = parts[1]  # pass, fail, log, etc.
            msg = parts[2].strip()

            if macro == "PASS":
                report_pass("check", msg)
            elif macro == "EXPECT":
                # very basic, if this macro is encountered we expect a non-zero exit code
                # this runs into an issue where the test might fail in ways other than we
                # expected, but it works ok for now
                failure_expected = True
                if retcode != 0:
                    report_pass("expect", msg)
                else:
                    report_fail("expect", msg)
                    no_failures = False
            elif macro == "FAIL":
                report_fail("check", msg)
                no_failures = False

    if not failure_expected and retcode != 0:
        printred("- Unexpected nonzero exit code")
        no_failures = False

    if no_failures:
        printgreen("** TEST PASSED - NO FAILURES **")
    else:
        ANY_FAILED = True


def main():
    global EXECUTABLE_PATH, VERBOSE, COMPILE

    if "-v" in sys.argv[1:]:
        VERBOSE = True

    if "--compile" in sys.argv[1:]:
        COMPILE = True

    print(
        f"Running all end-to-end tests using the {"compiler" if COMPILE else "interpreter"}\n"
    )

    root_dir = get_project_root_directory()
    EXECUTABLE_PATH = f"{root_dir}/{EXECUTABLE_NAME}"
    e2e_test_dir = f"{root_dir}/examples/e2e"

    for filename in os.listdir(e2e_test_dir):
        if filename.startswith("t_"):
            if COMPILE:
                run_e2e_test_compiled(e2e_test_dir, filename, root_dir)
            else:
                run_e2e_test(e2e_test_dir, filename)

    print("-" * 80)
    if ANY_FAILED:
        printred("SOME TESTS HAD FAILURES. TRY RERUNNING WITH -v")
    else:
        printgreen("ALL TESTS PASSED")


if __name__ == "__main__":
    main()
