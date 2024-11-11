#pragma once
#include <json/json.h>
#include <string>
#include "hwinfo/hwinfo.h"

namespace hardware {
struct OS {
  std::string name;
  std::string version;
  std::string arch;
};

inline Json::Value ToJson(const OS& os) {
  Json::Value res;
  res["version"] = os.version;
  res["name"] = os.name;
  return res;
}

namespace os {
inline OS FromJson(const Json::Value& root) {
  return {.name = root["name"].asString(),
          .version = root["version"].asString()};
}
}  // namespace os

inline OS GetOSInfo() {
  hwinfo::OS os;
  return OS{.name = os.name(),
            .version = os.version(),
            .arch = os.is32bit() ? "32 bit" : "64 bit"};
}
}  // namespace hardware