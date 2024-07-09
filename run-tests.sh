#!/usr/bin/env bash

EXECUTABLE_NAME=output

#
# Check that $EXECUTABLE_NAME exists at pwd and is executable. Complain and exit
# zonzero if not.
#
echo "Verifying that executable exists..."
if [[ ! -f "$EXECUTABLE_NAME" ]] || [[ ! -x "$EXECUTABLE_NAME" ]]; then
    echo "Unable to find executable file \"$EXECUTABLE_NAME\" in \"$PWD\""
    echo ""
    echo "Verify that the file exists, is named correctly, and has its"
    echo "executable bit set. You may need to run \"make\" to rebuild it."
    echo ""
    echo "Aborting test run."
    exit 1
fi

echo "Running unit tests..."
./"$EXECUTABLE_NAME" --test

echo "Running integration tests..."
./scripts/run_parser_test.py
./scripts/run_e2e_tests.py

echo "Finished running tests."