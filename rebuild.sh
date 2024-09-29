#!/usr/bin/env bash

# Handle MacOS not having nproc
if [[ "$OSTYPE" == "darwin"* ]]; then
  alias nproc="sysctl -n hw.logicalcpu"
fi

# runs both cmake and make for convenience
cmake . && make -j$(nproc)
