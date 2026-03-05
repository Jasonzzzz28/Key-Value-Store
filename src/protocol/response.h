#pragma once

#include <optional>
#include <string>

namespace protocol {

// Format responses (without trailing '\n'; caller adds it when writing).
std::string Ok();
std::string Pong();
std::string Value(const std::string& value);
std::string Nil();
std::string Integer(int n);
std::string Err(const std::string& message);

}  // namespace protocol
