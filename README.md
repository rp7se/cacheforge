# CacheForge

A lightweight C++17 key-value cache server for learning backend fundamentals.

## Current Status

Project Foundation — the repository currently provides a CMake-based C++17
project skeleton and a minimal server executable. Cache functionality and
networking are not implemented yet.

## Build and Run

```sh
cmake -S . -B build
cmake --build build --config Debug
```

With a multi-configuration generator, run the Debug executable from the
generated configuration directory (for example, `build/Debug`).

## Planned Features

The following features are planned and are not implemented yet:

- Key-Value Store
- `SET` / `GET` / `DEL` / `EXISTS` commands
- TCP Server
- Multiple Client Connections
- Thread-safe Storage
- TTL Expiration
- Snapshot Persistence
- Testing
