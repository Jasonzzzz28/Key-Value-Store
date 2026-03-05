#include "net/connection.h"

#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstring>

namespace net {

namespace {

constexpr size_t kRecvBufSize = 4096;

}  // namespace

Connection::Connection(int fd) : fd_(fd) {}

Connection::~Connection() {
  if (fd_ >= 0) {
    close(fd_);
    fd_ = -1;
  }
}

bool Connection::ReadLine(std::string& out_line) {
  out_line.clear();
  for (;;) {
    auto pos = read_buf_.find('\n');
    if (pos != std::string::npos) {
      out_line = read_buf_.substr(0, pos);
      read_buf_.erase(0, pos + 1);
      // Strip trailing \r if present (telnet/netcat)
      if (!out_line.empty() && out_line.back() == '\r') {
        out_line.pop_back();
      }
      return true;
    }

    char buf[kRecvBufSize];
    ssize_t n = recv(fd_, buf, sizeof(buf), 0);
    if (n <= 0) {
      if (n == 0) {
        return false;  // EOF
      }
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        continue;
      }
      return false;
    }
    read_buf_.append(buf, static_cast<size_t>(n));
  }
}

bool Connection::WriteLine(const std::string& line) {
  std::string to_send = line + "\n";
  const char* p = to_send.data();
  size_t remaining = to_send.size();
  while (remaining > 0) {
    ssize_t n = send(fd_, p, remaining, 0);
    if (n <= 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        continue;
      }
      return false;
    }
    p += n;
    remaining -= static_cast<size_t>(n);
  }
  return true;
}

}  // namespace net
