#pragma once
#include <string>
#include <unordered_map>
#include "common/hardware_config.h"

namespace commands {
class HardwareActivateCmd {
 public:
  bool Exec(const std::string& host, int port,
            const std::unordered_map<std::string, std::string>& options);
};
}  // namespace commands