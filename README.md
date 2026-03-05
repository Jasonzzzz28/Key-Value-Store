# Redis-like Key-Value Store

A small Redis-inspired in-memory key-value database with a TCP server, line-based text protocol, and thread-per-connection concurrency.

## Features

- **TCP server** — multiple clients supported
- **Line-based protocol** — one request per line, newline-terminated
- **Commands:** `PING`, `SET`, `GET`, `DEL`, `EXISTS`
- **Thread-per-connection** — one `std::thread` per client
- **Thread-safe store** — single `std::unordered_map` protected by `std::shared_mutex`
- **Clean shutdown** — Ctrl+C stops the server gracefully

## Build

**With CMake (recommended):**

```bash
cmake -S . -B build
cmake --build build -j
```

**Without CMake (e.g. clang++):**

```bash
mkdir -p build
clang++ -std=c++17 -Isrc -o build/kv_server \
  src/main.cpp \
  src/net/tcp_server.cpp src/net/connection.cpp \
  src/protocol/parser.cpp src/protocol/response.cpp \
  src/core/dispatcher.cpp src/core/commands.cpp \
  src/storage/kv_store.cpp
```

Requires a C++17 compiler and POSIX (Linux/macOS).

## Run

```bash
./build/kv_server --port 6379
```

Options:

- `--port <port>` — listen port (default: 6379)
- `--host <host>` — bind address (default: 0.0.0.0)

Stop with **Ctrl+C** for a clean shutdown.

## Protocol

- **Request:** one ASCII line per command, tokens separated by spaces. Keys and values must not contain spaces.
- **Response:** one ASCII line per reply, newline-terminated.

| Command   | Args     | Response              |
|----------|----------|------------------------|
| `PING`   | —        | `+PONG`                |
| `SET k v`| key, val | `+OK`                  |
| `GET k`  | key      | `+<value>` or `(nil)`  |
| `DEL k`  | key      | `:1` (removed) or `:0` |
| `EXISTS k` | key    | `:1` or `:0`           |

Errors: `-ERR <message>` (e.g. wrong arity, unknown command).

## Manual test

Using `nc` (netcat):

```bash
nc 127.0.0.1 6379
```

Then type:

```
PING
SET a 1
GET a
GET b
EXISTS a
DEL a
GET a
```

Expected replies: `+PONG`, `+OK`, `+1`, `(nil)`, `:1`, `:1`, `(nil)`.

## Project structure

```
repo/
  CMakeLists.txt
  design-doc.md
  README.md
  src/
    main.cpp              # Entry point, SIGINT, server start
    net/
      tcp_server.{h,cpp}  # Bind, listen, accept, thread per connection
      connection.{h,cpp}  # Buffered ReadLine / WriteLine
    protocol/
      parser.{h,cpp}      # Line → Request (command + args)
      response.{h,cpp}    # Format responses (+OK, :n, -ERR, etc.)
    core/
      dispatcher.{h,cpp}  # Route command → KVStore → response
      commands.{h,cpp}    # (Placeholder; logic in dispatcher)
    storage/
      kv_store.{h,cpp}    # unordered_map + shared_mutex
```

Design details are in [design-doc.md](design-doc.md).
