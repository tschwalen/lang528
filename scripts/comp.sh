#!/usr/bin/env bash
set -e

#
# Quick and dirty proof-of-concept for generating a C file and linking it against
# our runtime library during compilation.
#

PROJECT_ROOT=$(git rev-parse --show-toplevel)

cd "$PROJECT_ROOT"
make 
pushd runtime
make
popd

WORKDIR="$PROJECT_ROOT/.work"
rm -r "$WORKDIR"
mkdir "$WORKDIR"
cd $WORKDIR


# cat << 'EOF' > "$WORKDIR/prog.c"
# #include <stdio.h>
# #include "runtime.h"
# int main(int argc, char* argv[]) {
#     printf("%d\n", placeholder(3));
#     return 0;
# }
# EOF
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


# test_program="test1.src"
# test_program="test2.src"
# test_program="if_elseif_else.src"
# test_program="floats.src"
# test_program="fib.src"
test_program="expr2.src"

"$PROJECT_ROOT/output" --comp --input="$PROJECT_ROOT/examples/$test_program" > "$WORKDIR/prog.c"

# Do we really need to precompile the library? TBD
gcc -o compiled_exec prog.c -I"$INCLUDE_PATH" -L"$LIB_PATH" -lruntime

# create a compile commands json file in the work dir so that clangd can be useful for the generated C file

# run the compiled executable
./compiled_exec