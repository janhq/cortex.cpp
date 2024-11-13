#pragma once
#include <vector>

namespace cortex::hw {
struct ActivateHardwareConfig {
  std::vector<int> gpus;
};

}