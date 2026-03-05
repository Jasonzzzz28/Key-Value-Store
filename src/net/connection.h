#pragma once

#include <string>

namespace net {

class Connection {
 public:
  explicit Connection(int fd);
  ~Connection();

  Connection(const Connection&) = delete;
  Connection& operator=(const Connection&) = delete;

  // Returns false on EOF/disconnect.
  bool ReadLine(std::string& out_line);
  bool WriteLine(const std::string& line);

 private:
  int fd_;
  std::string read_buf_;
};

}  // namespace net
