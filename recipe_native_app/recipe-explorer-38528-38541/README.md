# recipe-explorer-38528-38541

Native Qt6 Recipe Explorer

This workspace contains a native Qt6 application located in `recipe_native_app/`. It allows browsing, searching, and managing recipes.

Build and Run locally
- Requirements: CMake >= 3.16, Qt6 (Core, Widgets), a C++17 compiler
- Container directory: `recipe_native_app`

Steps:
1) cd recipe_native_app
2) chmod +x ./run.sh
3) ./run.sh

Notes:
- CMake provides a custom `run` target which executes the built `MainApp`.
- The `run.sh` script offers a consistent entry point for CI/orchestrators to avoid bash syntax errors when no command is specified.
- If running headless, you may need a virtual display (e.g., `xvfb`) to launch a GUI app, e.g., `xvfb-run -a ./run.sh`.
