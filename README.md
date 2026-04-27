# video-call-service

`video-call-service` is a local video call prototype with a Qt desktop client and
a Qt TCP/UDP relay server. The client captures local camera frames, encodes video
with FFmpeg, sends media over UDP, and displays remote participant video decoded
from other connected clients.

## Project Layout

- `client/` - Qt 6 desktop client.
- `server/` - Qt 6 TCP control server and UDP media relay.
- `shared/` - protocol and shared client/server types.
- `tests/` - GoogleTest-based unit tests for client, server, and shared protocol
  behavior.
- `GNUmakefile` - convenience targets for configure, build, run, and tests.
- `.envrc.example` - optional direnv template for local Qt discovery.

## Requirements

Assume a fresh development machine has only a C++ compiler and CMake installed.
You still need the following tools and libraries:

- CMake 3.21 or newer.
- A C++17 compiler.
- Qt 6 with these components:
  - Core
  - Widgets
  - Network
  - Multimedia
  - MultimediaWidgets
- FFmpeg development libraries discoverable through `pkg-config`:
  - `libavcodec`
  - `libavutil`
  - `libswscale`
- `pkg-config` or `pkgconf`.
- Git and network access for the first CMake configure, because the test setup
  downloads GoogleTest with CMake `FetchContent`.
- Optional but recommended: `direnv` for automatic local environment setup.

## macOS Setup

Install command-line dependencies with Homebrew:

```bash
brew install cmake pkg-config ffmpeg direnv
```

Install Qt 6 with the official Qt installer from
https://www.qt.io/download, or with Homebrew if that fits your machine:

```bash
brew install qt
```

If you use the official Qt installer, Qt is usually installed under a path like:

```text
$HOME/Qt/6.10.2/macos
```

If you use Homebrew on Apple Silicon, Qt is usually under:

```text
/opt/homebrew/opt/qt
```

If you use Homebrew on Intel macOS, Qt is usually under:

```text
/usr/local/opt/qt
```

## Linux Setup

Package names vary by distribution. On Ubuntu/Debian-style systems, install the
closest available Qt 6 and FFmpeg development packages:

```bash
sudo apt update
sudo apt install build-essential cmake pkg-config git \
  qt6-base-dev qt6-multimedia-dev \
  libavcodec-dev libavutil-dev libswscale-dev
```

If your distribution does not package the required Qt 6 multimedia development
libraries, install Qt from https://www.qt.io/download and set `QT_ROOT` as shown
below.

## Configure Qt Discovery

The top-level `CMakeLists.txt` automatically prepends the `QT_ROOT` environment
variable to `CMAKE_PREFIX_PATH` when `QT_ROOT` is set. This lets CMake find the
right Qt installation without hardcoding a machine-specific path in the repo.

You can set `QT_ROOT` directly in your shell:

```bash
export QT_ROOT="$HOME/Qt/6.10.2/macos"
export PATH="$QT_ROOT/bin:$PATH"
```

For Homebrew Qt on Apple Silicon:

```bash
export QT_ROOT="/opt/homebrew/opt/qt"
export PATH="$QT_ROOT/bin:$PATH"
```

For Homebrew Qt on Intel macOS:

```bash
export QT_ROOT="/usr/local/opt/qt"
export PATH="$QT_ROOT/bin:$PATH"
```

You can also use direnv so this environment loads automatically whenever you
enter the repository:

```bash
cp .envrc.example .envrc
```

Edit `.envrc` if needed so `QT_ROOT` points at your Qt 6 platform prefix. Then
allow the file:

```bash
direnv allow
```

The committed `.envrc.example` tries to auto-detect the latest official Qt
installer path under `$HOME/Qt`. The real `.envrc` file is intentionally ignored
by Git because each machine may use a different Qt path.

## Verify Dependencies

Before configuring the project, these commands should work:

```bash
cmake --version
c++ --version
pkg-config --modversion libavcodec libavutil libswscale
```

If Qt was installed outside a standard system prefix, this should also find Qt:

```bash
cmake -S . -B build -D CMAKE_PREFIX_PATH="$QT_ROOT"
```

You do not need both `QT_ROOT` and `-D CMAKE_PREFIX_PATH=...`; use whichever is
more convenient.

## Build

From the repository root:

```bash
cmake -S . -B build
cmake --build build
```

The first configure may take longer because CMake downloads GoogleTest into the
build dependency area.

The repository also includes convenience Make targets:

```bash
make setup
make build
```

`make setup` runs CMake configure into `build/`, and `make build` compiles the
client, server, and tests.

## Run Locally

The server listens for TCP control connections on port `5555` and starts the UDP
media relay on port `5556`.

The easiest local command is:

```bash
make run-with-server
```

This target builds the project, starts `video_call_server`, launches the desktop
client, and stops the server process after the client exits.

To run the pieces manually, start the server in one terminal:

```bash
./build/server/video_call_server
```

Then start the client in another terminal.

On macOS:

```bash
./build/client/video_call_client.app/Contents/MacOS/video_call_client
```

On Linux or other non-bundle builds:

```bash
./build/client/video_call_client
```

The convenience target for launching only the client is:

```bash
make run
```

`make run` uses the macOS app bundle path, so use the direct executable path on
non-macOS systems if needed.

## Join a Call

1. Start the server.
2. Start one or more client instances.
3. In each client, enter the server address.
4. For local testing on the same machine, use:

   ```text
   127.0.0.1
   ```

5. If you change the server port in code or run against a non-default server,
   use:

   ```text
   host:port
   ```

6. Click `Join Call`.
7. Allow camera permission if the operating system asks.
8. The local preview should show the local camera.
9. Once another client joins and sends video, the remote participant area should
   show that client's decoded video.

To test multiple local clients, open two terminals and run the client command in
each one while the server is running.

## macOS Camera Permission

On macOS, the first run may trigger a camera permission prompt. Allow camera
access for `video_call_client`.

If macOS blocks the app or does not show a prompt, check:

```text
System Settings > Privacy & Security > Camera
System Settings > Privacy & Security > Security
```

Enable camera access for `video_call_client` if it appears there. If the app was
blocked as untrusted, authorize it from the Security section and launch it again.

## Run Tests

Build first:

```bash
cmake --build build
```

Run all tests discovered under the build tree:

```bash
ctest --test-dir build --output-on-failure
```

Run only client tests:

```bash
ctest --test-dir build/tests/client --output-on-failure
```

Run only server tests:

```bash
ctest --test-dir build/tests/server --output-on-failure
```

The convenience target:

```bash
make test
```

currently runs the client test directory. Use the explicit `ctest` commands
above when you want all tests or only server tests.

## Clean and Rebuild

Remove the generated build directory:

```bash
make clean
```

Then configure and build again:

```bash
make setup
make build
```

You can also remove `build/` manually if needed. It is ignored by Git.

## Troubleshooting

If CMake cannot find Qt, set `QT_ROOT` to your Qt 6 prefix and make sure
`$QT_ROOT/bin` is on `PATH`. The Qt prefix is the directory containing Qt's
`lib/cmake/Qt6` package files.

If CMake cannot find FFmpeg, verify that `pkg-config` can see the required
libraries:

```bash
pkg-config --modversion libavcodec libavutil libswscale
```

If that command fails, install the FFmpeg development packages for your platform
and make sure their `.pc` files are on `PKG_CONFIG_PATH`.

If the first CMake configure fails while downloading GoogleTest, check network
access and run the configure command again:

```bash
cmake -S . -B build
```

If the client connects over TCP but never reaches `UDP registered`, check that
UDP traffic is allowed on port `5556` between the client and server. Local
same-machine testing with `127.0.0.1` should not require firewall changes.

If no camera preview appears, confirm that the machine has a camera, no other
application is exclusively using it, and camera permission has been granted to
the client application.
