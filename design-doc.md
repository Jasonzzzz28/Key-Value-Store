TITLE
Redis-like Key-Value Store

1) SCOPE (MVP)
Build a small Redis-inspired in-memory key-value database with:
- TCP server (multiple clients)
- Line-based text protocol
- Commands: PING, SET, GET, DEL, EXISTS
- Thread-per-connection concurrency
- In-memory store using unordered_map
- Clean shutdown (Ctrl+C)

Out of scope:
- Tests
- Observability/logging/metrics
- Security hardening
- TTL/EXPIRE
- Persistence (AOF/RDB)
- RESP compatibility

2) SINGLE DEVELOPMENT OPTION (CHOSEN)
Concurrency model: Thread-per-connection (std::thread)
Storage safety: Single std::shared_mutex protecting one unordered_map
Protocol: One request per line, space-separated tokens, values have no spaces

Why this option:
- Fastest to build and debug
- Clear architecture and correctness
- Good enough concurrency for an MVP

3) PROTOCOL SPEC
Transport: TCP
Request: ASCII line terminated by '\n'
Response: ASCII line terminated by '\n'

Command format:
- PING
- SET <key> <value>
- GET <key>
- DEL <key>
- EXISTS <key>

Constraints:
- <key> and <value> contain no spaces
- Ignore empty lines

Responses (simple):
- Success simple string: +OK
- PING: +PONG
- GET hit: +<value>
- GET miss: (nil)
- Integers: :<number>
- Errors: -ERR <message>

Examples:
> PING
< +PONG

> SET a 1
< +OK

> GET a
< +1

> GET b
< (nil)

> EXISTS a
< :1

> DEL a
< :1

4) HIGH-LEVEL ARCHITECTURE
Client
  -> TCP Socket
    -> Connection thread:
         - buffered ReadLine()
         - parse tokens
         - dispatch command
         - call KVStore
         - WriteLine(response)

Modules:
- net/: TcpServer, Connection (buffered I/O)
- protocol/: parse line -> Request(tokens)
- core/: dispatcher + command handlers
- storage/: KVStore (unordered_map + shared_mutex)

5) PROJECT STRUCTURE (CMake)
repo/
  CMakeLists.txt
  src/
    main.cpp
    net/
      tcp_server.h
      tcp_server.cpp
      connection.h
      connection.cpp
    protocol/
      parser.h
      parser.cpp
      response.h
      response.cpp
    core/
      dispatcher.h
      dispatcher.cpp
      commands.h
      commands.cpp
    storage/
      kv_store.h
      kv_store.cpp

6) KEY DATA TYPES & INTERFACES
6.1 protocol::Request
- command: std::string (uppercased)
- args: std::vector<std::string>

6.2 protocol::Parser
ParseLine(line) -> Request OR error string
- Trim whitespace
- Split by spaces (collapse multiple spaces)
- Uppercase command

6.3 storage::KVStore (thread-safe)
Internals:
- std::unordered_map<std::string, std::string> data_;
- mutable std::shared_mutex mu_;

API:
- void Set(const std::string& key, const std::string& value)
- std::optional<std::string> Get(const std::string& key) const
- int Del(const std::string& key)
- bool Exists(const std::string& key) const

Locking:
- Get/Exists: std::shared_lock
- Set/Del: std::unique_lock

6.4 core::Dispatcher
- Holds reference to KVStore
- Handle(Request) -> std::string response_line (without trailing '\n' or with—choose one convention)

Command semantics:
- PING (0 args) -> +PONG
- SET (2 args) -> Set -> +OK
- GET (1 arg) -> +value or (nil)
- DEL (1 arg) -> :1 if removed else :0
- EXISTS (1 arg) -> :1 or :0
- Unknown -> -ERR unknown command
- Wrong arity -> -ERR wrong number of arguments

6.5 net::Connection
Goal: provide line-based reads with buffering (handle partial recv)
Members:
- int fd_
- std::string read_buf_

API:
- bool ReadLine(std::string& out_line)   // returns false on EOF/disconnect
- bool WriteLine(const std::string& line) // writes line + "\n"

Implementation hints:
- recv() into temp buffer, append to read_buf_
- find '\n' in read_buf_, extract one line
- remove trailing '\r' if present (support telnet/netcat)

6.6 net::TcpServer
Responsibilities:
- Create socket, bind, listen
- accept() loop
- For each client socket:
  - spawn std::thread(ConnectionHandler, client_fd)
  - detach thread (or store and join on shutdown)

Shutdown strategy:
- Install SIGINT handler that sets an atomic<bool> stop = true
- accept loop checks stop and breaks (may require closing listen fd to unblock accept)

7) CONTROL FLOW (END-TO-END)
main():
- parse args (host, port) [minimal: port only is fine]
- create KVStore
- create Dispatcher(KVStore)
- start TcpServer(host, port, handler)

ConnectionHandler(fd):
- Connection conn(fd)
- loop:
  - if !conn.ReadLine(line): break
  - auto parsed = Parser::ParseLine(line)
  - if parse error: conn.WriteLine("-ERR ..."); continue
  - resp = dispatcher.Handle(req)
  - conn.WriteLine(resp)
- close(fd)

8) BUILD & RUN
Build:
- cmake -S . -B build
- cmake --build build -j

Run:
- ./build/kv_server --port 6379

Manual test:
- nc 127.0.0.1 6379
- type: PING, SET a 1, GET a

9) IMPLEMENTATION ORDER (MVP)
Step 1: CMake + main starts and parses --port
Step 2: TcpServer listen/accept and spawn connection threads
Step 3: Connection buffered ReadLine/WriteLine working with nc
Step 4: Parser splits line into Request
Step 5: KVStore with shared_mutex
Step 6: Dispatcher + commands: PING/SET/GET/DEL/EXISTS
Step 7: SIGINT shutdown (stop accept loop; exit cleanly)

10) “CURSOR TASK LIST” (COPY/PASTE)
- [ ] Create CMake project and folder structure under src/
- [ ] Implement storage::KVStore (unordered_map + shared_mutex)
- [ ] Implement protocol::Parser (trim, split, uppercase cmd)
- [ ] Implement core::Dispatcher and command handlers
- [ ] Implement net::Connection buffered ReadLine/WriteLine
- [ ] Implement net::TcpServer (bind/listen/accept + std::thread per connection)
- [ ] Wire up main.cpp (create store+dispatcher, start server, SIGINT stop)
- [ ] Verify manually via nc: PING, SET/GET, EXISTS, DEL
