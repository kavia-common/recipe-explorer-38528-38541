# Recipe Native App - Run Instructions

This folder contains a native Qt6 application (CMake project) for the Recipe Explorer.

Build and Run
- Requirements: CMake >= 3.16, Qt6 (Core, Widgets), a C++17 compiler.
- Entry point: ./run.sh (POSIX-safe; works when orchestrators invoke via sh -lc).

Steps:
1) cd recipe_native_app
2) chmod +x ./run.sh
3) ./run.sh

Headless / CI usage
- The script auto-detects headless environments (no DISPLAY) and will:
  - Prefer xvfb-run if available.
  - Otherwise try QT_QPA_PLATFORM=offscreen then QT_QPA_PLATFORM=minimal.
- You can force headless behavior explicitly:
  - RUN_HEADLESS=1 ./run.sh
- You can also invoke xvfb directly:
  - xvfb-run -a ./run.sh

Notes:
- The CMakeLists.txt defines a custom `run` target to execute the built `MainApp`.
- The run.sh script avoids bash-specific syntax to prevent CI/Common setup errors like:
  bash: -c: line 2: syntax error: unexpected end of file
- If parallel build flags are unsupported by the generator, the script falls back to a standard build, and then runs.
