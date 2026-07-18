# CacheForge

A lightweight C++17 key-value cache server for learning backend fundamentals.

## Current Status

The project currently provides a basic in-memory storage layer and a minimal
server executable. Command parsing and networking are not implemented yet.

## Implemented

- In-memory Key-Value Store
- `set`, `get`, `del`, and `exists` storage operations
- Distinct handling for missing keys and empty string values
- Basic KVStore tests

The storage layer uses `std::unordered_map`. In the average case, `set`, `get`,
`del`, and `exists` have close to O(1) lookup, insertion, and deletion
complexity. This storage layer is not thread-safe.

## Build and Run

```sh
cmake -S . -B build
cmake --build build --config Debug
```

With a multi-configuration generator, run the Debug executable from the
generated configuration directory (for example, `build/Debug`).

## Planned Features

The following features are planned and are not implemented yet:

- Command Parser and command responses
- TCP Server
- Multiple Client Connections
- Thread Safety
- TTL Expiration
- Snapshot Persistence
