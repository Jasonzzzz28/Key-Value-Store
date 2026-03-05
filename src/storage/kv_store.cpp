#include "storage/kv_store.h"

namespace storage {

void KVStore::Set(const std::string& key, const std::string& value) {
  std::unique_lock lock(mu_);
  data_[key] = value;
}

std::optional<std::string> KVStore::Get(const std::string& key) const {
  std::shared_lock lock(mu_);
  auto it = data_.find(key);
  if (it == data_.end()) {
    return std::nullopt;
  }
  return it->second;
}

int KVStore::Del(const std::string& key) {
  std::unique_lock lock(mu_);
  auto it = data_.find(key);
  if (it == data_.end()) {
    return 0;
  }
  data_.erase(it);
  return 1;
}

bool KVStore::Exists(const std::string& key) const {
  std::shared_lock lock(mu_);
  return data_.find(key) != data_.end();
}

}  // namespace storage
