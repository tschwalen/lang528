#!/usr/bin/env bash
set -e

#
# Quick and dirty proof-of-concept for generating a C file and linking it against
# our runtime library during compilation.
#

PROJECT_ROOT=$(git rev-parse --show-toplevel)
WORKDIR="$PROJECT_ROOT/.work"
rm -r "$WORKDIR"
mkdir "$WORKDIR"
cd $WORKDIR

# cat << 'EOF' > "$WORKDIR/dummyprog.c"
# #include <stdio.h>
# #include "runtime.h"

# int main(int argc, char* argv[]) {
#     printf("%d\n", placeholder(3));
#     return 0;
# }
# EOF

# test_program="test1.src"
test_program="floats.src"

"$PROJECT_ROOT/output" --comp --input="$PROJECT_ROOT/examples/$test_program" > "$WORKDIR/dummyprog.c"

INCLUDE_PATH="$PROJECT_ROOT/runtime/include"
LIB_PATH="$PROJECT_ROOT/runtime"

# Do we really need to precompile the library? TBD
gcc -o compiled_exec dummyprog.c -I"$INCLUDE_PATH" -L"$LIB_PATH" -lruntime