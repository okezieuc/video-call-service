# Shared

This directory contains protocol and type definitions used by both the client
and server.

- `Protocol.h` defines TCP control message framing, UDP packet framing, join
  response payloads, and helper encode/decode functions.
- `SharedTypes.h` contains shared application-level types.

Because these headers are included by both `client/` and `server/`, changes here
can affect the desktop client, relay server, and tests. After editing shared
protocol code, run:

```bash
ctest --test-dir build --output-on-failure
```
