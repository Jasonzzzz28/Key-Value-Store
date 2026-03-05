#pragma once

#include <atomic>
#include <functional>
#include <string>

namespace net {

// ConnectionHandler(client_fd) is called from a dedicated thread per client.
using ConnectionHandler = std::function<void(int)>;

class TcpServer {
 public:
  TcpServer(const std::string& host, uint16_t port,
            ConnectionHandler handler);
  ~TcpServer();

  TcpServer(const TcpServer&) = delete;
  TcpServer& operator=(const TcpServer&) = delete;

  // Run accept loop. Returns when Stop() is called (e.g. from SIGINT).
  void Run();
  void Stop();

  std::atomic<bool>& StopFlag() { return stop_; }

 private:
  std::string host_;
  uint16_t port_;
  ConnectionHandler handler_;
  int listen_fd_{-1};
  std::atomic<bool> stop_{false};
};

}  // namespace net
