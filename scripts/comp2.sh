#!/usr/bin/env bash
set -e

# find the project root from git and set a clean work directory
PROJECT_ROOT=$(git rev-parse --show-toplevel)
WORKDIR="$PROJECT_ROOT/.work"
rm -r "$WORKDIR"
mkdir "$WORKDIR"

# set up library & include paths and generate a compile_commands.json file
# for clangd debugging
cd $WORKDIR
INCLUDE_PATH="$PROJECT_ROOT/runtime/include"
LIB_PATH="$PROJECT_ROOT/runtime"
cat << EOF > "$WORKDIR/compile_commands.json"
[
{
  "directory": "$WORKDIR",
  "command": "/usr/bin/gcc -o $WORKDIR/compiled_exec $WORKDIR/prog.c -I$INCLUDE_PATH -L$LIB_PATH -lruntime",
  "file": "$WORKDIR/prog.c",
  "output": "$WORKDIR/compiled_exec"
}
]
EOF

# read the passed input program
program=$1
shift

# compile the passed program into C
"$PROJECT_ROOT/output" --comp --input="$program" > "$WORKDIR/prog.c"

# then compile the generated C program linked against the runtime library
gcc -o compiled_exec prog.c -I"$INCLUDE_PATH" -L"$LIB_PATH" -lruntime