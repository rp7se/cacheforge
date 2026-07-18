# CacheForge

CacheForge is a lightweight C++17 key-value cache server built to explore
backend fundamentals including TCP networking, basic concurrency, TTL
expiration, and snapshot persistence.

## Project Overview

CacheForge is a learning-oriented and portfolio-focused server for practicing
understandable C++ backend design. It combines a thread-safe in-memory store,
a small text command pipeline, a blocking Windows TCP server, TTL expiration,
and a simple snapshot module without external libraries or advanced
infrastructure.

## Key Features

- In-memory key-value storage
- `SET`, `GET`, `DEL`, `EXISTS`, and `SETEX` commands
- Whitespace-based command parsing and command-name normalization
- Separate command parsing and execution modules
- Windows Winsock2 TCP server using a newline-delimited text protocol
- Multiple concurrent clients using one `std::thread` per connection
- Shared KVStore protected by `std::mutex`
- TTL expiration with lazy checks and simple periodic cleanup
- Snapshot save/load APIs for ordinary keys without TTL
- Automatic snapshot restore during server startup
- Tests for storage, parsing, execution, concurrency, TTL, and persistence

## Architecture

The command path follows the current module boundaries:

```text
TCP Client
    |
    v
TcpServer
    |
    v
CommandParser
    |
    v
CommandProcessor
    |
    v
KVStore
    |
    v
Text Response
```

TTL and persistence are connected directly to the storage layer:

```text
KVStore ---> TTL Expiration
    |
    +------> SnapshotPersistence
```

The server concurrency model is intentionally small:

```text
Main Server Thread
    |
    v
blocking accept()
    |
    v
One std::thread per client connection
    |
    v
Shared thread-safe KVStore
```

## Tech Stack

- C++17
- CMake 3.16 or newer
- C++ Standard Library containers, threading, chrono, and filesystem APIs
- Windows Winsock2 (`ws2_32`)
- MSVC / Visual Studio 2022 Build Tools for the documented Windows build

The project has no third-party runtime or test-framework dependencies.

## Quick Start

### Requirements

- Windows
- CMake 3.16 or newer
- A C++17 compiler, such as MSVC from Visual Studio 2022

### Build

Run from the repository root:

```powershell
cmake -S . -B build
cmake --build build --config Debug
```

The executable target is `cacheforge_server`. With the Visual Studio generator,
the Debug executable is typically located at:

```text
build/Debug/cacheforge_server.exe
```

### Run

```powershell
.\build\Debug\cacheforge_server.exe
```

The server reports its default endpoint when startup succeeds:

```text
CacheForge Server
Listening on 127.0.0.1:6380
```

### PowerShell Client Example

The protocol only requires a TCP client that sends one command per line. This
PowerShell example uses .NET's built-in `TcpClient`:

```powershell
$client = [System.Net.Sockets.TcpClient]::new('127.0.0.1', 6380)
$stream = $client.GetStream()
$writer = [System.IO.StreamWriter]::new($stream)
$reader = [System.IO.StreamReader]::new($stream)
$writer.AutoFlush = $true

$writer.WriteLine('SET name Tracy')
$reader.ReadLine()  # OK
$writer.WriteLine('GET name')
$reader.ReadLine()  # Tracy

$client.Dispose()
```

PowerShell is only an example client and is not a project dependency.

## Commands

Commands are case-insensitive, while keys and values retain their original
case. Each request and response ends with a newline.

| Command | Example | Response |
| --- | --- | --- |
| `SET key value` | `SET name Tracy` | `OK` |
| `GET key` | `GET name` | Stored value, or `(nil)` when missing or expired |
| `DEL key` | `DEL name` | `1` when deleted, otherwise `0` |
| `EXISTS key` | `EXISTS name` | `1` when present, otherwise `0` |
| `SETEX key seconds value` | `SETEX session 60 abc123` | `OK` for a valid positive integer TTL |

Wrong argument counts return `ERROR wrong number of arguments`. Invalid
`SETEX` durations return `ERROR invalid expiration`, and unsupported commands
return `ERROR unknown command`.

The parser uses whitespace-delimited tokens. Consequently, the current TCP
command format does not support keys or values containing spaces or newlines.

## Storage and Command Processing

### KVStore

KVStore stores `std::unordered_map<std::string, Entry>`, where each `Entry`
contains a value and an optional expiration timestamp. The container provides
average-case constant-time lookup, insertion, and deletion behavior, but no
strict worst-case constant-time guarantee.

`get()` returns `std::optional<std::string>` so a missing key remains distinct
from a stored empty string. KVStore owns its synchronization and expiration
state, keeping callers independent of its internal container.

### CommandParser

CommandParser performs whitespace tokenization, normalizes only the command
name to uppercase, and extracts arguments. It does not validate command
semantics, execute commands, or access KVStore.

### CommandProcessor

CommandProcessor validates argument counts and `SETEX` durations, executes the
five supported commands against KVStore, and formats the text response. Unknown
commands are rejected here rather than by the parser.

## Concurrency Model

TcpServer uses blocking Winsock2 sockets. The main server thread blocks in
`accept()`, then creates and detaches one `std::thread` for each accepted client.
Each client thread parses and processes its own newline-delimited requests.

All clients share one CommandProcessor and one KVStore. Regular KVStore
operations use `std::lock_guard<std::mutex>` around the internal data. The TTL
cleanup loop uses the same mutex with `std::unique_lock` because it waits on a
condition variable. This is a simple learning-oriented concurrency model, not
a thread-pool or asynchronous networking architecture.

## TTL Design

`SETEX key seconds value` stores a value with a positive integer lifetime. Each
TTL Entry keeps an expiration timestamp based on `std::chrono::steady_clock`.

- `GET`, `EXISTS`, and `DEL` lazily remove an expired key and treat it as absent.
- Ordinary `SET` overwrites the Entry and clears any previous TTL.
- A joinable background cleanup thread scans for expired entries every second.
- KVStore destruction sets a stop flag, wakes the cleanup thread, and joins it.

The implementation uses a simple scan rather than a timing wheel, heap-based
scheduler, or advanced timer framework.

## Snapshot Persistence

SnapshotPersistence exposes explicit `save()` and `load()` APIs. Before writing,
KVStore copies all ordinary entries while holding its mutex; file I/O then runs
after the lock is released. Loading validates the whole file before restoring
entries through a dedicated KVStore API.

The default file is the relative path `cacheforge.snapshot` in the server's
current working directory. It uses a versioned length-prefixed format, avoiding
delimiter conflicts in keys and values stored directly through KVStore.

- Ordinary keys without TTL are saved and restored.
- TTL keys are excluded because `steady_clock` timestamps are not meaningful
  across process restarts.
- A TTL key overwritten by ordinary `SET` becomes eligible for the next save.
- A missing file is treated as the first server run.
- A malformed file produces an error; the server continues without restoring it.
- Server startup automatically calls `load()` before TCP listening begins.
- Saving is currently a programmatic API only. There is no `SAVE` command or
  graceful Ctrl+C auto-save path.

The snapshot mechanism does not provide crash-safe or transactional durability.

## Project Structure

```text
CacheForge/
|-- include/cacheforge/
|   |-- CommandParser.h
|   |-- CommandProcessor.h
|   |-- KVStore.h
|   |-- SnapshotPersistence.h
|   `-- TcpServer.h
|-- src/
|   |-- CommandParser.cpp
|   |-- CommandProcessor.cpp
|   |-- KVStore.cpp
|   |-- SnapshotPersistence.cpp
|   |-- TcpServer.cpp
|   `-- main.cpp
|-- tests/
|   |-- CommandParserTests.cpp
|   |-- CommandProcessorTests.cpp
|   |-- KVStoreTests.cpp
|   |-- KVStoreThreadTests.cpp
|   |-- SnapshotPersistenceTests.cpp
|   `-- TtlTests.cpp
|-- AGENTS.md
|-- CMakeLists.txt
|-- README.md
`-- .gitignore
```

Generated build files, executables, IDE settings, and runtime snapshots are
excluded from the documented source tree and ignored by Git.

## Testing

Build first, then run every registered test with CTest:

```powershell
ctest --test-dir build -C Debug --output-on-failure
```

| Test executable | CTest name | Coverage |
| --- | --- | --- |
| `cacheforge_kvstore_tests` | `kvstore_tests` | Storage operations and missing/empty-value behavior |
| `cacheforge_command_parser_tests` | `command_parser_tests` | Tokenization, normalization, whitespace, and edge cases |
| `cacheforge_command_processor_tests` | `command_processor_tests` | Validation, execution, responses, and integration |
| `cacheforge_kvstore_thread_tests` | `kvstore_thread_tests` | Shared KVStore thread safety |
| `cacheforge_ttl_tests` | `ttl_tests` | SETEX, expiration, cleanup, and TTL concurrency |
| `cacheforge_snapshot_tests` | `snapshot_tests` | Save/load, TTL exclusion, corruption, and file cleanup |

Tests are small standalone executables and do not require GoogleTest.

## Design Decisions

- `std::unordered_map` keeps the storage implementation small and gives useful
  average-case lookup behavior for a cache exercise.
- `std::optional` distinguishes a missing key from a stored empty value.
- Separating CommandParser from CommandProcessor keeps tokenization independent
  from validation, execution, and storage.
- KVStore owns its mutex so every caller receives the same synchronization
  guarantees without coordinating external locks.
- One thread per client keeps blocking network code understandable and allows
  multiple simultaneous connections without a complex scheduler.
- Lazy expiration keeps normal operations correct, while periodic cleanup
  removes expired keys that are never accessed again.
- TTL entries are excluded from snapshots because their `steady_clock`
  timestamps cannot be safely reused after a process restart.

## Current Limitations

- Windows Winsock2 networking only
- Blocking sockets and one detached thread per client
- No thread pool, non-blocking I/O, IOCP, or event loop
- No graceful server shutdown or Ctrl+C snapshot auto-save
- Whitespace-delimited commands do not support spaces or newlines in keys/values
- No LRU eviction or configurable memory limit
- TTL keys are not persisted
- Snapshot persistence is simple and not crash-safe or transactional
- No Redis protocol compatibility
- No replication, clustering, or distributed-cache features
- No advanced benchmark suite

## Future Improvements

The following are possible future directions and are not implemented:

- Cross-platform socket abstraction
- Bounded thread pool
- LRU eviction and configurable memory limits
- More robust persistence and graceful shutdown support
- Protocol improvements for values containing whitespace
- Repeatable benchmarking and profiling
