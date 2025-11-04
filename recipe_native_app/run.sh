#!/usr/bin/env bash
# Simple build-and-run script for the native Qt app.
# This script ensures a predictable entrypoint for CI/orchestrators to avoid
# "bash: -c: line 2: syntax error: unexpected end of file" when no command is provided.

set -euo pipefail

# Create and use build directory
BUILD_DIR="$(dirname "$0")/build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure and build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -- -j"$(nproc)"

# Run the app via the custom 'run' target configured in CMakeLists.txt
# Falls back to executing the binary directly if needed.
if cmake --build . --target run; then
    exit 0
fi

# Fallback run
if [[ -x ./MainApp ]]; then
    ./MainApp
else
    echo "Error: Built binary not found. Expected ./MainApp"
    exit 1
fi
