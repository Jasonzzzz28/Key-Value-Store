#pragma once

#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace storage {

class KVStore {
 public:
  void Set(const std::string& key, const std::string& value);
  std::optional<std::string> Get(const std::string& key) const;
  int Del(const std::string& key);
  bool Exists(const std::string& key) const;

 private:
  std::unordered_map<std::string, std::string> data_;
  mutable std::shared_mutex mu_;
};

}  // namespace storage
