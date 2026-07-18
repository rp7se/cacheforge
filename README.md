# CacheForge

A lightweight C++17 key-value cache server for learning backend fundamentals.

## Current Status

The project currently provides a basic thread-safe in-memory storage layer,
command parser, command processor, concurrent TCP server for Windows, TTL, and
simple snapshot persistence for ordinary keys.

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
- Snapshot Persistence for ordinary keys without TTL
- Automatic Snapshot Restore during server startup

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

## Snapshot Persistence

`SnapshotPersistence` saves ordinary keys without TTL and restores them as
ordinary keys. It copies persistent entries while holding the KVStore mutex,
then releases the mutex before writing the file. Keys created by `SETEX` are not
saved because `std::chrono::steady_clock` timestamps are process-local. If a TTL
key is later overwritten with ordinary `SET`, the TTL is cleared and that key
becomes eligible for the next snapshot.

The snapshot uses a versioned, length-prefixed text format so keys and values
can contain characters such as `=` or newlines. The default file is
`cacheforge.snapshot` in the server's current working directory. At startup the
server attempts to load this file before creating the command processor and
starting TCP listening. A missing file is treated as the first run. A malformed
file produces an error message and the server continues without restoring it.

Saving is currently an explicit `SnapshotPersistence::save()` operation. The
server has no `SAVE` command or graceful Ctrl+C shutdown hook, so it does not
automatically save on Ctrl+C and does not promise crash-safe persistence. This
is intentionally a simple learning-oriented snapshot mechanism, not a database
or transactional durability system.
## Build and Run

```sh
cmake -S . -B build
cmake --build build --config Debug
```

With a multi-configuration generator, run the Debug executable from the
generated configuration directory (for example, `build/Debug`).
