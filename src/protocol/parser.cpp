#include "protocol/parser.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <sstream>

namespace protocol {

namespace {

std::string Trim(const std::string& s) {
  auto start = s.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) {
    return "";
  }
  auto end = s.find_last_not_of(" \t\r\n");
  return s.substr(start, end - start + 1);
}

std::string ToUpper(std::string s) {
  for (auto& c : s) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }
  return s;
}

}  // namespace

std::optional<Request> ParseLine(const std::string& line,
                                 std::string& error_message) {
  error_message.clear();
  std::string trimmed = Trim(line);
  if (trimmed.empty()) {
    return std::nullopt;  // skip empty lines, no error
  }

  std::vector<std::string> tokens;
  std::istringstream iss(trimmed);
  std::string token;
  while (iss >> token) {
    tokens.push_back(token);
  }

  if (tokens.empty()) {
    return std::nullopt;
  }

  Request req;
  req.command = ToUpper(tokens[0]);
  for (size_t i = 1; i < tokens.size(); ++i) {
    req.args.push_back(tokens[i]);
  }
  return req;
}

}  // namespace protocol
