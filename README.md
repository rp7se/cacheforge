# CacheForge

A lightweight C++17 key-value cache server for learning backend fundamentals.

## Current Status

The project currently provides a basic in-memory storage layer, command parser,
and minimal server executable. Command execution and networking are not implemented yet.

## Implemented

- In-memory Key-Value Store
- `set`, `get`, `del`, and `exists` storage operations
- Distinct handling for missing keys and empty string values
- Basic KVStore tests
- Command Parser
- Basic CommandParser tests

The storage layer uses `std::unordered_map`. In the average case, `set`, `get`,
`del`, and `exists` have close to O(1) lookup, insertion, and deletion
complexity. This storage layer is not thread-safe.

The Command Parser supports whitespace tokenization, command normalization to
uppercase, and argument extraction. It only parses input; command validation,
execution, and KVStore integration are not implemented yet.

## Build and Run

```sh
cmake -S . -B build
cmake --build build --config Debug
```

With a multi-configuration generator, run the Debug executable from the
generated configuration directory (for example, `build/Debug`).

## Planned Features

The following features are planned and are not implemented yet:

- Command Execution and command responses
- TCP Server
- Multiple Client Connections
- Thread Safety
- TTL Expiration
- Snapshot Persistence
