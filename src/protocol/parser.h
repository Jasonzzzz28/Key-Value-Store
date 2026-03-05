#pragma once

#include <optional>
#include <string>
#include <vector>

namespace protocol {

struct Request {
  std::string command;
  std::vector<std::string> args;
};

// ParseLine(line) -> Request. Returns empty optional on parse error;
// error_message is set when returning nullopt.
// Empty lines are ignored (return nullopt with empty error = skip).
std::optional<Request> ParseLine(const std::string& line,
                                 std::string& error_message);

}  // namespace protocol
