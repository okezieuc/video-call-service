# video-call-service

## Bootstrap Status
- Qt client scaffolded with a minimal window and a no-op `Start Call` button.

## Project Layout
- `client/` Qt desktop client
- `server/` server placeholder
- `shared/` shared types/utilities placeholder
- `tests/` test placeholder

## Build and Run (Client)
Requirements:
- CMake 3.21+
- Qt6 (Widgets)
- A C++17 compiler

Environment setup (recommended):
```bash
cp .envrc.example .envrc
direnv allow
```

Notes:
- `.envrc` is intentionally ignored and should stay local to each machine.
- `.envrc.example` is the committed template.
- `QT_ROOT` should point to the Qt platform prefix (for example: `<...>/6.10.2/macos`).
- The root `CMakeLists.txt` prepends `QT_ROOT` to `CMAKE_PREFIX_PATH` automatically when set.

Generate the build system configuration in the 'build' directory:
```bash
cmake -S . -B build
```

Build:
```bash
cmake --build build
```

Run:
```bash
./build/client/video_call_client
```
