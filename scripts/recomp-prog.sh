#!/usr/bin/env bash
set -e

PROJECT_ROOT=$(git rev-parse --show-toplevel)
WORKDIR="$PROJECT_ROOT/.work"
cd $WORKDIR


INCLUDE_PATH="$PROJECT_ROOT/runtime/include"
LIB_PATH="$PROJECT_ROOT/runtime"

# Do we really need to precompile the library? TBD
gcc -o compiled_exec prog.c -I"$INCLUDE_PATH" -L"$LIB_PATH" -lruntime

# run the compiled executable
./compiled_exec $@