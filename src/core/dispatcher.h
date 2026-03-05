#pragma once

#include "protocol/parser.h"
#include "storage/kv_store.h"

#include <string>

namespace core {

class Dispatcher {
 public:
  explicit Dispatcher(storage::KVStore& store) : store_(store) {}

  // Handle(request) -> response line (no trailing '\n')
  std::string Handle(const protocol::Request& req);

 private:
  storage::KVStore& store_;
};

}  // namespace core
