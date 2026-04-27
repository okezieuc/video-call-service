# Server

The server provides:

- A TCP control listener on port `5555`.
- A UDP media relay on port `5556`.
- Client ID assignment after a `JoinCall` control message.
- UDP endpoint registration before media forwarding.
- Video packet forwarding from each registered client to the other registered
  clients.

## Build

From the repository root:

```bash
cmake -S . -B build
cmake --build build
```

## Run

From the repository root:

```bash
./build/server/video_call_server
```

Expected startup output includes messages similar to:

```text
UDP relay listening on port 5556
Server listening on port 5555
```

Leave this process running while clients connect.

## Ports

- TCP `5555` - client control connection.
- UDP `5556` - media relay endpoint.

For local testing, clients can connect to `127.0.0.1`. For another machine on the
same network, use the server machine's reachable IP address and allow TCP `5555`
and UDP `5556` through any firewall.

## Tests

From the repository root:

```bash
ctest --test-dir build/tests/server --output-on-failure
```
