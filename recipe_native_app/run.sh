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

# Decide how to run (headless-friendly).
# Signals to force headless: RUN_HEADLESS=1 or DISPLAY is unset/empty.
WANT_HEADLESS="${RUN_HEADLESS-}"
if [ -z "${DISPLAY-}" ]; then
  WANT_HEADLESS=1
fi

run_mainapp_direct() {
  if cmake --build . --target run; then
    return 0
  fi
  if [ -x "./MainApp" ]; then
    ./MainApp
  else
    echo "Error: Built binary not found. Expected ./MainApp"
    return 1
  fi
}

if [ "${WANT_HEADLESS:-0}" = "1" ]; then
  # Prefer xvfb if available; otherwise try Qt offscreen/minimal platforms.
  if command -v xvfb-run >/dev/null 2>&1; then
    # Ensure XDG_RUNTIME_DIR to avoid warnings that can be treated as errors by some runners
    XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-/tmp/runtime-$(id -u)}"
    export XDG_RUNTIME_DIR
    mkdir -p "$XDG_RUNTIME_DIR" || true
    chmod 700 "$XDG_RUNTIME_DIR" || true
    xvfb-run -a sh -lc 'cmake --build . --target run || ./MainApp'
    exit $?
  else
    # Try offscreen first, then minimal as fallback
    export QT_QPA_PLATFORM=offscreen
    if run_mainapp_direct; then
      exit 0
    fi
    export QT_QPA_PLATFORM=minimal
    if run_mainapp_direct; then
      exit 0
    fi
    echo "Failed to run in headless mode (no xvfb and offscreen/minimal failed)."
    exit 1
  fi
else
  # Non-headless path (DISPLAY available) - run normally.
  if run_mainapp_direct; then
    exit 0
  fi
  # If normal run failed due to display, attempt a graceful fallback to headless.
  if command -v xvfb-run >/dev/null 2>&1; then
    echo "Display run failed, attempting headless fallback with xvfb-run..."
    XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-/tmp/runtime-$(id -u)}"
    export XDG_RUNTIME_DIR
    mkdir -p "$XDG_RUNTIME_DIR" || true
    chmod 700 "$XDG_RUNTIME_DIR" || true
    xvfb-run -a sh -lc 'cmake --build . --target run || ./MainApp'
    exit $?
  else
    echo "Display run failed and xvfb-run is not available. Try RUN_HEADLESS=1."
    exit 1
  fi
fi
