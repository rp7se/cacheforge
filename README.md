# CacheForge

A lightweight C++17 key-value cache server for learning backend fundamentals.

## Current Status

The project currently provides a basic in-memory storage layer, command parser,
command processor, and minimal server executable. Networking is not implemented
yet.

## Implemented

- In-memory Key-Value Store
- `set`, `get`, `del`, and `exists` storage operations
- Distinct handling for missing keys and empty string values
- Basic KVStore tests
- Command Parser
- Basic CommandParser tests
- Command Processor with `SET`, `GET`, `DEL`, and `EXISTS` execution
- Basic CommandProcessor and integration tests

The storage layer uses `std::unordered_map`. In the average case, `set`, `get`,
`del`, and `exists` have close to O(1) lookup, insertion, and deletion
complexity. This storage layer is not thread-safe.

The Command Parser supports whitespace tokenization, command normalization to
uppercase, and argument extraction. The Command Processor validates argument
counts, executes supported commands through KVStore, and converts results into
simple text responses.

The current core flow is `CommandParser` → `CommandProcessor` → `KVStore`.
There is no TCP server or client connection support yet.

## Build and Run

```sh
cmake -S . -B build
cmake --build build --config Debug
```

With a multi-configuration generator, run the Debug executable from the
generated configuration directory (for example, `build/Debug`).

## Planned Features

The following features are planned and are not implemented yet:

- TCP Server
- Multiple Client Connections
- Thread Safety
- TTL Expiration
- Snapshot Persistence
