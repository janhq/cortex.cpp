#pragma once

#include <string>
#include <unordered_map>

namespace commands {
class HardwareActivateCmd {
 public:
  bool Exec(const std::string& host, int port,
            const std::unordered_map<std::string, std::string>& options);
};
}  // namespace commands
