#!/usr/bin/env bash
set -e

#
# Script used for testing and debugging during development of the compiler.
# 
# Given the name of a test program in the <root>/examples directory, the script
# does the following:
#    - Recompiles the main project and the runtime
#    - Deletes and recreates the <root>/.work directory
#    - Creates a clangd compile_commands.json file for the generated C file
#    - runs the --comp command of the main project to generate prog.c
#    - compiles prog.c and links it against the included runtime using system standard tooling
#    - runs the resulting compiled executable passing along the arguments in the script (e.g. $@)

# get the project root from git
PROJECT_ROOT=$(git rev-parse --show-toplevel)

# build the L528 compiler (Main C++ project) and the runtime (C project under 
# PROJECT_ROOT/runtime)
cd "$PROJECT_ROOT"
make 
pushd runtime
make
popd

# Clean up the work directory
WORKDIR="$PROJECT_ROOT/.work"
rm -r "$WORKDIR"
mkdir "$WORKDIR"
cd $WORKDIR

# set the include and library paths to the runtime
INCLUDE_PATH="$PROJECT_ROOT/runtime/include"
LIB_PATH="$PROJECT_ROOT/runtime"

# create a compile commands json file in the work dir so that clangd can be useful 
# for the generated C file. (type anotations, go-to-definition, etc.)
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


# get the test program name from the command line
test_program=$1
shift

"$PROJECT_ROOT/output" --comp --input="$PROJECT_ROOT/examples/$test_program" > "$WORKDIR/prog.c"

# Do we really need to precompile the library? TBD
cc -o compiled_exec prog.c -I"$INCLUDE_PATH" -L"$LIB_PATH" -lruntime

# run the compiled executable
./compiled_exec $@