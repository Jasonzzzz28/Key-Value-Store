#include "core/dispatcher.h"
#include "protocol/response.h"

namespace core {

std::string Dispatcher::Handle(const protocol::Request& req) {
  const std::string& cmd = req.command;
  const auto& args = req.args;

  if (cmd == "PING") {
    if (args.size() != 0) {
      return protocol::Err("wrong number of arguments");
    }
    return protocol::Pong();
  }

  if (cmd == "SET") {
    if (args.size() != 2) {
      return protocol::Err("wrong number of arguments");
    }
    store_.Set(args[0], args[1]);
    return protocol::Ok();
  }

  if (cmd == "GET") {
    if (args.size() != 1) {
      return protocol::Err("wrong number of arguments");
    }
    auto val = store_.Get(args[0]);
    if (val) {
      return protocol::Value(*val);
    }
    return protocol::Nil();
  }

  if (cmd == "DEL") {
    if (args.size() != 1) {
      return protocol::Err("wrong number of arguments");
    }
    int n = store_.Del(args[0]);
    return protocol::Integer(n);
  }

  if (cmd == "EXISTS") {
    if (args.size() != 1) {
      return protocol::Err("wrong number of arguments");
    }
    bool exists = store_.Exists(args[0]);
    return protocol::Integer(exists ? 1 : 0);
  }

  return protocol::Err("unknown command");
}

}  // namespace core
