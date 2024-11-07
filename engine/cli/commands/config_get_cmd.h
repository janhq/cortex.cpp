#pragma once

#include <string>

namespace commands {
class ConfigGetCmd {
 public:
  void Exec(const std::string& host, int port);
};
}  // namespace commands
