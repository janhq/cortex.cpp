#pragma once
#include <json/json.h>
#include <string>

namespace hardware {
struct PowerInfo {
  std::string charging_status;
  int battery_life;
  bool is_power_saving;
};

inline Json::Value ToJson(const PowerInfo& pi) {
  Json::Value res;
  res["charging_status"] = pi.charging_status;
  res["battery_life"] = pi.battery_life;
  res["is_power_saving"] = pi.is_power_saving;
  return res;
}

namespace power {
inline PowerInfo FromJson(const Json::Value& root) {
  return {.charging_status = root["charging_status"].asString(),
          .battery_life = root["battery_life"].asInt(),
          .is_power_saving = root["is_power_saving"].asBool()};
}
}  // namespace power

inline PowerInfo GetPowerInfo() {
  return PowerInfo{};
}
}  // namespace hardware