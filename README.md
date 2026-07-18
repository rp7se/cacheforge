# CacheForge

A lightweight C++17 key-value cache server for learning backend fundamentals.

## Current Status

The project currently provides a basic thread-safe in-memory storage layer,
command parser, command processor, and concurrent TCP server for Windows.

## Implemented

- In-memory Key-Value Store
- `set`, `get`, `del`, and `exists` storage operations
- Distinct handling for missing keys and empty string values
- Basic KVStore tests
- Command Parser
- Basic CommandParser tests
- Command Processor with `SET`, `GET`, `DEL`, and `EXISTS` execution
- Basic CommandProcessor and integration tests
- Basic TCP Server using Windows Winsock2
- Newline-delimited `SET`, `GET`, `DEL`, and `EXISTS` commands over TCP
- Concurrent Client Connections using one `std::thread` per client
- Basic thread-safe KVStore protected by `std::mutex`
- Deterministic KVStore thread-safety tests
- TTL Expiration through `SETEX key seconds value`
- Lazy Expiration on `GET`, `DEL`, and `EXISTS`
- Simple Periodic Cleanup of expired keys

The storage layer uses `std::unordered_map`. In the average case, `set`, `get`,
`del`, and `exists` have close to O(1) lookup, insertion, and deletion
complexity. Access to the storage layer is protected by a single `std::mutex`.

The Command Parser supports whitespace tokenization, command normalization to
uppercase, and argument extraction. The Command Processor validates argument
counts, executes supported commands through KVStore, and converts results into
simple text responses.

The complete flow is TCP Client → `TcpServer` → `CommandParser` →
`CommandProcessor` → `KVStore` → text response. The server listens on
`127.0.0.1:6380` by default and uses blocking sockets with one thread per client
connection. All client threads share one KVStore, whose data is protected by a
single `std::mutex`. The concurrency model is intentionally simple and
learning-oriented.

TTL uses an expiration timestamp based on `std::chrono::steady_clock`. Expired
keys are removed lazily when accessed, while one joinable background thread
performs a simple periodic cleanup. Ordinary `SET` removes any previous TTL.

## Build and Run

```sh
cmake -S . -B build
cmake --build build --config Debug
```

With a multi-configuration generator, run the Debug executable from the
generated configuration directory (for example, `build/Debug`).

## Planned Features

The following features are planned and are not implemented yet:

- Snapshot Persistence
