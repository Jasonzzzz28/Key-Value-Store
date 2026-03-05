#include "net/tcp_server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>
#include <thread>

namespace net {

TcpServer::TcpServer(const std::string& host, uint16_t port,
                     ConnectionHandler handler)
    : host_(host), port_(port), handler_(std::move(handler)) {
  listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd_ < 0) {
    throw std::runtime_error("socket() failed");
  }

  int opt = 1;
  if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    close(listen_fd_);
    listen_fd_ = -1;
    throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
  }

  struct sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port_);
  if (host_.empty() || host_ == "0.0.0.0") {
    addr.sin_addr.s_addr = INADDR_ANY;
  } else {
    if (inet_pton(AF_INET, host_.c_str(), &addr.sin_addr) <= 0) {
      close(listen_fd_);
      listen_fd_ = -1;
      throw std::runtime_error("invalid host address");
    }
  }

  if (bind(listen_fd_, reinterpret_cast<struct sockaddr*>(&addr),
           sizeof(addr)) < 0) {
    close(listen_fd_);
    listen_fd_ = -1;
    throw std::runtime_error("bind() failed");
  }

  if (listen(listen_fd_, 128) < 0) {
    close(listen_fd_);
    listen_fd_ = -1;
    throw std::runtime_error("listen() failed");
  }
}

TcpServer::~TcpServer() {
  Stop();
  if (listen_fd_ >= 0) {
    close(listen_fd_);
    listen_fd_ = -1;
  }
}

void TcpServer::Run() {
  while (!stop_) {
    struct sockaddr_in client_addr {};
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(listen_fd_,
                           reinterpret_cast<struct sockaddr*>(&client_addr),
                           &client_len);
    if (client_fd < 0) {
      if (stop_) {
        break;
      }
      continue;
    }
    if (stop_) {
      close(client_fd);
      break;
    }
    std::thread t(handler_, client_fd);
    t.detach();
  }
}

void TcpServer::Stop() {
  stop_ = true;
  if (listen_fd_ >= 0) {
    shutdown(listen_fd_, SHUT_RDWR);
  }
}

}  // namespace net
