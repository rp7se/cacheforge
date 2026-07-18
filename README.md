# CacheForge

CacheForge 是一个基于 C++17 实现的轻量级 Key-Value 缓存服务器，用于学习和展示 TCP 网络通信、基础多线程并发、TTL 过期机制与 Snapshot 快照持久化等 C++ 服务端开发基础。

## 项目简介

CacheForge 是一个面向学习与作品展示的 C++ 后端项目。项目通过清晰、易于理解的模块组合，实现线程安全的内存存储、文本命令处理、Windows TCP Server、TTL 过期和基础 Snapshot Persistence，不依赖第三方库，也不引入复杂基础设施。

## 核心功能

- 内存 Key-Value Store
- 支持 `SET`、`GET`、`DEL`、`EXISTS` 和 `SETEX` 命令
- 基于空白字符的命令分词与 Command 名称标准化
- 相互独立的 CommandParser 与 CommandProcessor
- 基于 Windows Winsock2 的 TCP Server
- 使用 newline-delimited text protocol，每行一个请求和响应
- 支持多个客户端同时连接，每个连接由一个 `std::thread` 处理
- 使用 `std::mutex` 保护共享 KVStore
- TTL Expiration、Lazy Expiration 与 Periodic Cleanup
- 面向普通无 TTL Key 的 Snapshot 保存与加载 API
- Server 启动时自动加载 Snapshot
- 覆盖存储、解析、执行、线程安全、TTL 和 Snapshot 的自动化测试

## 技术栈

- C++17
- CMake 3.16 或更高版本
- C++ Standard Library：容器、线程、`chrono`、`filesystem`
- Windows Winsock2（`ws2_32`）
- MSVC / Visual Studio 2022 Build Tools

项目没有第三方运行时依赖，也不依赖第三方测试框架。

## 系统架构

当前命令处理链路如下：

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
Response
```

TTL 与 Snapshot Persistence 直接围绕存储层工作：

```text
KVStore ---> TTL Expiration
    |
    +------> SnapshotPersistence
```

当前并发模型保持简单：

```text
Server Main Thread
    |
    v
blocking accept()
    |
    v
One Thread Per Client Connection
    |
    v
Shared Thread-safe KVStore
```

## 命令说明

Command 名称不区分大小写，Key 和 Value 保留原始大小写。每个请求与响应都以换行符结束。

| 命令 | 示例 | 返回结果 |
| --- | --- | --- |
| `SET key value` | `SET name Tracy` | 成功返回 `OK` |
| `GET key` | `GET name` | 返回 Value；Key 不存在或已过期时返回 `(nil)` |
| `DEL key` | `DEL name` | 删除成功返回 `1`，否则返回 `0` |
| `EXISTS key` | `EXISTS name` | Key 存在返回 `1`，否则返回 `0` |
| `SETEX key seconds value` | `SETEX session 60 abc123` | TTL 为有效正整数时返回 `OK` |

参数数量错误时返回 `ERROR wrong number of arguments`；`SETEX` 的 seconds 无效时返回 `ERROR invalid expiration`；不支持的命令返回 `ERROR unknown command`。

当前协议使用空白字符分隔 Token，因此通过 TCP 发送的 Key 和 Value 不能包含空格或换行。

## KVStore 设计

KVStore 使用以下结构保存数据：

```cpp
std::unordered_map<std::string, Entry>
```

每个 `Entry` 包含一个 Value 和一个可选的 Expiration Timestamp。`std::unordered_map` 在平均情况下能够提供较高效的查找、插入和删除操作，但不保证严格的最坏情况常数时间。

`get()` 返回 `std::optional<std::string>`，因此能够区分“Key 不存在”和“Value 是空字符串”两种状态。KVStore 自己管理内部容器、线程同步和过期状态，调用方不需要直接接触这些实现细节。

## 命令解析与执行流程

### CommandParser

CommandParser 负责：

- 将原始文本命令按空白字符分词
- 仅将 Command 名称标准化为大写
- 提取 Arguments，并保留其原始大小写

CommandParser 不负责命令业务校验、命令执行或 KVStore 操作。

### CommandProcessor

CommandProcessor 负责：

- 校验参数数量
- 校验 `SETEX` 的 seconds
- 调用 KVStore 执行命令
- 格式化文本 Response

当前支持的命令只有 `SET`、`GET`、`DEL`、`EXISTS` 和 `SETEX`。未知命令由 CommandProcessor 返回错误。

## TCP Server

TcpServer 基于 Windows Winsock2，使用 Blocking Socket 和 newline-delimited text protocol。Server 默认监听：

```text
127.0.0.1:6380
```

Server 会处理 TCP 分包与同一接收缓冲区中的多条命令。每个完整行交给 CommandParser 和 CommandProcessor 处理，再将一行文本 Response 返回给客户端。

## 并发模型与线程安全

Server Main Thread 阻塞在 `accept()`。每当接受一个客户端连接，TcpServer 会创建并 detach 一个独立的 `std::thread` 处理该连接。

所有客户端线程共享同一个 CommandProcessor 和 KVStore。KVStore 的普通操作使用 `std::lock_guard<std::mutex>` 保护内部数据；TTL Cleanup Loop 因为需要等待 condition variable，使用同一个 mutex 配合 `std::unique_lock`。

这是一个简单、学习型的并发模型，用于展示基础线程创建、共享数据访问和互斥保护，不使用 Thread Pool 或 Async Networking。

## TTL 过期机制

`SETEX key seconds value` 可以为 Key 设置正整数秒数的 TTL。每个 TTL Entry 使用 `std::chrono::steady_clock` 保存 Expiration Timestamp。

- `GET`、`EXISTS` 和 `DEL` 会执行 Lazy Expiration：过期 Key 会被删除并视为不存在。
- 普通 `SET` 会覆盖 Entry 并清除原有 TTL。
- 一个可 `join` 的后台 Cleanup Thread 每秒扫描一次并删除过期 Entry。
- KVStore 析构时会设置停止标记、唤醒 Cleanup Thread，然后执行 `join()`。

当前实现采用简单遍历，没有使用 Timing Wheel、Min-Heap Scheduler 或复杂 Timer Framework。

## Snapshot 快照持久化

SnapshotPersistence 提供显式的 `save()` 和 `load()` API。保存时，KVStore 会在 mutex 保护下复制所有可持久化 Entry，然后释放锁，再执行文件 I/O。加载时会先完整验证文件内容，验证成功后再通过 KVStore 的 Restore API 恢复数据。

默认 Snapshot 文件为 Server 当前工作目录下的相对路径：

```text
cacheforge.snapshot
```

Snapshot 使用带版本标识的长度前缀格式，避免直接使用简单分隔符造成 Key 或 Value 内容冲突。

- 普通无 TTL Key 可以保存和恢复。
- TTL Key 不写入 Snapshot，因为 `steady_clock` 的时间戳不能直接跨进程复用。
- TTL Key 被普通 `SET` 覆盖后会清除 TTL，可以在下一次 Save 时进入 Snapshot。
- Snapshot 文件不存在时视为 Server 第一次运行。
- Snapshot 文件损坏时会输出错误，Server 继续启动但不会恢复该文件。
- Server 会在开始 TCP 监听前自动调用 `load()`。
- Save 目前仅通过程序 API 显式调用，没有 `SAVE` 命令，也没有 Ctrl+C 自动保存流程。

当前 Snapshot 是基础学习型持久化机制，不提供 Crash-safe 或 Transactional Durability 保证。

## 项目目录结构

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

`build/`、CMake 生成文件、Executable、IDE 配置和运行时 Snapshot 均不属于源码目录，并已通过 `.gitignore` 忽略。

## 环境要求

- Windows
- CMake 3.16 或更高版本
- 支持 C++17 的编译器，例如 Visual Studio 2022 提供的 MSVC

项目不要求安装 Boost、Drogon、SQLite、Redis 或 GoogleTest。

## 构建与运行

在项目根目录执行：

```powershell
cmake -S . -B build
cmake --build build --config Debug
```

CMake executable target 名称为 `cacheforge_server`。使用 Visual Studio Generator 时，Debug executable 通常位于：

```text
build/Debug/cacheforge_server.exe
```

启动 Server：

```powershell
.\build\Debug\cacheforge_server.exe
```

启动成功后会输出：

```text
CacheForge Server
Listening on 127.0.0.1:6380
```

## TCP 客户端测试示例

下面使用 PowerShell 内置的 .NET `TcpClient` 连接 Server，不需要额外安装客户端依赖：

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
$writer.WriteLine('SETEX temp 5 hello')
$reader.ReadLine()  # OK

$client.Dispose()
```

PowerShell 仅用于演示 TCP Client，不是 CacheForge 的项目依赖。

## 测试

完成构建后，可以通过 CTest 运行所有已注册测试：

```powershell
ctest --test-dir build -C Debug --output-on-failure
```

| Test Executable | CTest Name | 测试内容 |
| --- | --- | --- |
| `cacheforge_kvstore_tests` | `kvstore_tests` | KVStore 基础操作、缺失 Key 与空 Value |
| `cacheforge_command_parser_tests` | `command_parser_tests` | 分词、大小写标准化、空白与边界输入 |
| `cacheforge_command_processor_tests` | `command_processor_tests` | 参数校验、命令执行、Response 与模块集成 |
| `cacheforge_kvstore_thread_tests` | `kvstore_thread_tests` | 共享 KVStore 的基础线程安全 |
| `cacheforge_ttl_tests` | `ttl_tests` | SETEX、过期、Cleanup 与 TTL 并发操作 |
| `cacheforge_snapshot_tests` | `snapshot_tests` | Save/Load、TTL 排除、损坏文件与临时文件清理 |

所有测试都是独立的小型 Executable，不依赖 GoogleTest。

## 关键设计说明

- 使用 `std::unordered_map`，让存储层保持简洁，并获得适合缓存练习项目的平均查找效率。
- 使用 `std::optional`，区分不存在的 Key 和空字符串 Value。
- 分离 CommandParser 与 CommandProcessor，使分词职责独立于参数校验、执行和存储操作。
- 由 KVStore 自己管理 mutex，让所有调用方获得一致的线程安全保证，无需协调外部锁。
- 每个 Client 一个线程，使 Blocking Socket 代码容易理解，同时支持多个客户端连接。
- Lazy Expiration 保证访问时的正确性，Periodic Cleanup 则清理长期不再访问的过期 Key。
- Snapshot 排除 TTL Entry，因为基于 `steady_clock` 的 Expiration Timestamp 不能安全跨进程复用。

## 当前限制

- 网络层当前仅支持 Windows Winsock2。
- 使用 Blocking Socket，每个客户端连接对应一个 detach Thread。
- 未实现 Thread Pool、Non-blocking I/O、IOCP 或 Event Loop。
- 没有优雅关闭流程，也不支持 Ctrl+C 自动保存 Snapshot。
- 命令使用空白字符分词，Key 和 Value 不能包含空格或换行。
- 未实现 LRU Eviction 和可配置 Memory Limit。
- TTL Key 不进入 Snapshot。
- Snapshot 是简单持久化方案，不提供 Crash-safe 或 Transactional 保证。
- 不兼容 Redis Protocol。
- 未实现 Replication、Clustering 或 Distributed Cache。
- 暂无完整 Benchmark Suite。

## 未来可改进方向

以下内容属于未来方向，当前尚未实现：

- 跨平台 Socket 抽象
- 有界 Thread Pool
- LRU Eviction 与可配置 Memory Limit
- 更完善的持久化和优雅关闭流程
- 支持包含空白字符 Value 的协议格式
- 可重复执行的 Benchmark 与 Profiling
