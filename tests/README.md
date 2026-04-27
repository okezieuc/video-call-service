# Tests

Tests are built with GoogleTest. The top-level CMake configuration downloads
GoogleTest with `FetchContent` during the first configure.

## Build Tests

From the repository root:

```bash
cmake -S . -B build
cmake --build build
```

## Run All Tests

```bash
ctest --test-dir build --output-on-failure
```

## Run Client Tests

```bash
ctest --test-dir build/tests/client --output-on-failure
```

The convenience target also runs the client test directory:

```bash
make test
```

## Run Server Tests

```bash
ctest --test-dir build/tests/server --output-on-failure
```

## Test Targets

- `build/tests/client/video_call_client_test`
- `build/tests/server/video_call_server_test`

Use the direct test executables when you need to pass GoogleTest flags manually.
