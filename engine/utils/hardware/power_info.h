#pragma once
#include <string>

namespace hardware {
struct PowerInfo {
  std::string charging_status;
  int battery_life;
  bool is_power_saving;
};
}  // namespace hardware