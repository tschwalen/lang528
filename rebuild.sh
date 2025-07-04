#!/usr/bin/env bash

# Handle MacOS not having nproc
if [[ "$OSTYPE" == "darwin"* ]]; then
  alias nproc="sysctl -n hw.logicalcpu"
fi

PROJECT_ROOT=$(git rev-parse --show-toplevel)
cd $PROJECT_ROOT
cmake . && make -j$(nproc)
cd ./runtime
cmake . && make -j$(nproc)
