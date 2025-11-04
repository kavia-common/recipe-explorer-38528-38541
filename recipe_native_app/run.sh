#!/usr/bin/env sh
# Simple build-and-run script for the native Qt app.
# Provides a predictable entrypoint for CI/orchestrators and avoids
# "bash: -c: line 2: syntax error: unexpected end of file" from empty/partial commands.
# This script is POSIX-compliant; if bash is needed it re-execs itself with bash.

set -eu

# If invoked by a shell that lacks 'set -o pipefail' and we are on bash, enable it for robustness.
if command -v bash >/dev/null 2>&1; then
  # shellcheck disable=SC3040
  if [ -n "${BASH_VERSION-}" ]; then
    set -o pipefail
  fi
fi

# Resolve script directory portably
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD_DIR="$SCRIPT_DIR/build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Determine parallel build jobs in a portable manner
JOBS=1
if command -v nproc >/dev/null 2>&1; then
  JOBS=$(nproc || echo 1)
elif command -v getconf >/dev/null 2>&1; then
  JOBS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)
fi
case "$JOBS" in
  ''|0) JOBS=1 ;;
esac

# Configure and build
cmake -DCMAKE_BUILD_TYPE=Release ..
# Use make/ninja job flags only if supported by the generator; pass via -- -jN is safe for Make
cmake --build . -- -j"$JOBS" || cmake --build .

# Run via the custom 'run' target; if that fails, try to execute the binary directly.
if cmake --build . --target run; then
  exit 0
fi

# Fallback run
if [ -x "./MainApp" ]; then
  ./MainApp
else
  echo "Error: Built binary not found. Expected ./MainApp"
  exit 1
fi
