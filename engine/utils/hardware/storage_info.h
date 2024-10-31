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

inline StorageInfo GetStorageInfo() {
  return StorageInfo{};
}
}  // namespace hardware