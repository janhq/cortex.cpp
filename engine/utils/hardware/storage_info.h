#pragma once
#include <string>

namespace hardware {
struct StorageInfo {
  std::string type;
  int64_t total;
  int64_t available;
};
}  // namespace hardware