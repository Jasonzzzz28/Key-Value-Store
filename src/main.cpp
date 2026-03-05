#include "core/dispatcher.h"
#include "net/connection.h"
#include "net/tcp_server.h"
#include "protocol/parser.h"
#include "storage/kv_store.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace {

std::atomic<bool> g_stop{false};

void SignalHandler(int) {
  g_stop = true;
}

void ParseArgs(int argc, char* argv[], std::string& host, uint16_t& port) {
  host = "0.0.0.0";
  port = 6379;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--port" && i + 1 < argc) {
      port = static_cast<uint16_t>(std::stoul(argv[++i]));
    } else if (arg == "--host" && i + 1 < argc) {
      host = argv[++i];
    }
  }
}

}  // namespace

int main(int argc, char* argv[]) {
  std::string host;
  uint16_t port;
  ParseArgs(argc, argv, host, port);

  storage::KVStore store;
  core::Dispatcher dispatcher(store);

  std::signal(SIGINT, SignalHandler);

  net::TcpServer server(host, port, [&dispatcher](int client_fd) {
    net::Connection conn(client_fd);
    std::string line;
    std::string parse_error;
    while (conn.ReadLine(line)) {
      auto req = protocol::ParseLine(line, parse_error);
      if (!req.has_value()) {
        if (!parse_error.empty()) {
          conn.WriteLine("-ERR " + parse_error);
        }
        continue;
      }
      std::string resp = dispatcher.Handle(*req);
      if (!conn.WriteLine(resp)) {
        break;
      }
    }
  });

  g_stop = false;

  std::thread accept_thread([&server] { server.Run(); });

  while (!g_stop) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  server.Stop();
  accept_thread.join();

  std::cout << "Shutdown complete.\n";
  return 0;
}
