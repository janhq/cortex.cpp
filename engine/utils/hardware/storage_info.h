#pragma once
#include <json/json.h>
#include <string>

namespace hardware {
struct StorageInfo {
  std::string type;
  int64_t total;
  int64_t available;
};

inline Json::Value ToJson(const StorageInfo& si) {
  Json::Value res;
  res["total"] = si.total;
  res["available"] = si.available;
  res["type"] = si.type;
  return res;
}

namespace storage {
inline StorageInfo FromJson(const Json::Value& root) {
  return {.type = root["type"].asString(),
          .total = root["total"].asInt64(),
          .available = root["available"].asInt64()};
}
}  // namespace storage

inline StorageInfo GetStorageInfo() {
  return StorageInfo{};
}
}  // namespace hardware