\# CacheForge Development Guide



\## Project Goal



CacheForge is a lightweight C++17 Key-Value cache server created as a learning and portfolio project.



The project should demonstrate understandable C++ backend fundamentals without unnecessary complexity.



Core planned capabilities include:



\- Key-Value storage

\- SET / GET / DEL / EXISTS commands

\- Simple command parsing

\- TCP client-server communication

\- Multiple client connections

\- Basic thread-safe shared data access

\- TTL expiration

\- Simple snapshot persistence

\- Unit tests and documentation



\## Development Principles



1\. Develop incrementally through focused pull requests.

2\. Each PR should solve one clear problem.

3\. Preserve all previously working functionality.

4\. Prefer simple and understandable designs over advanced abstractions.

5\. Do not introduce unnecessary dependencies.

6\. Use clear C++17 code.

7\. Prefer RAII and standard library facilities.

8\. Avoid premature optimization.

9\. Validate inputs and handle errors explicitly.

10\. Keep modules small and responsibilities clear.



\## Knowledge Boundary



This project is intended for a university student preparing for an internship.



Do NOT introduce advanced infrastructure unless explicitly requested.



Do not introduce:



\- epoll

\- IOCP

\- io\_uring

\- Reactor framework

\- Proactor framework

\- coroutines

\- lock-free data structures

\- custom memory allocators

\- distributed systems

\- replication

\- clustering

\- consistent hashing

\- Redis protocol compatibility

\- complex thread pools

\- complex async frameworks

\- complex performance optimization



Networking and concurrency implementations should remain simple enough to explain clearly in an internship interview.



\## Architecture Direction



Prefer a modular structure such as:



\- application entry point

\- storage layer

\- command parser

\- command processor

\- network server

\- persistence module



Do not force this structure if the current code suggests a simpler equivalent design.



\## C++ Guidelines



\- Target C++17.

\- Use CMake.

\- Prefer STL containers and utilities.

\- Use smart pointers only when ownership semantics require them.

\- Avoid raw owning pointers.

\- Use const correctness where appropriate.

\- Avoid global mutable state.

\- Handle resource cleanup using RAII.

\- Keep headers and implementations reasonably separated.

\- Do not use `using namespace std;` in headers.



\## Testing



Each feature should be testable independently when practical.



Before completing a task:



1\. Build the project.

2\. Run relevant tests.

3\. Check error cases.

4\. Confirm existing functionality still works.



\## Git / PR Workflow



\- One focused task per PR.

\- Use meaningful branch names.

\- Use conventional commit prefixes where appropriate:

&#x20; - feat:

&#x20; - fix:

&#x20; - refactor:

&#x20; - test:

&#x20; - docs:

\- PR descriptions should explain:

&#x20; - functionality

&#x20; - implementation

&#x20; - testing

&#x20; - important design decisions



\## Documentation



README should remain accurate.



Do not claim:



\- high performance

\- high concurrency

\- production ready

\- Redis replacement

\- distributed cache



unless those capabilities are actually implemented and validated.



Use accurate wording such as:



"lightweight C++ key-value cache server"

or

"learning-oriented C++ cache server".



\## Completion Rule



When completing a Codex task:



1\. Summarize modified files.

2\. Explain the implementation.

3\. Report build results.

4\. Report test results.

5\. Mention important design decisions.

6\. Stop after the requested PR scope.



Do not automatically start the next PR.

