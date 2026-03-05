#include "protocol/response.h"

#include <sstream>

namespace protocol {

std::string Ok() { return "+OK"; }

std::string Pong() { return "+PONG"; }

std::string Value(const std::string& value) { return "+" + value; }

std::string Nil() { return "(nil)"; }

std::string Integer(int n) {
  std::ostringstream oss;
  oss << ":" << n;
  return oss.str();
}

std::string Err(const std::string& message) { return "-ERR " + message; }

}  // namespace protocol
