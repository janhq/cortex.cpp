#pragma once

#include <string>
#include "utils/result.hpp"

namespace commands {
class EngineReleaseCmd {
 public:
  cpp::result<std::string, std::string> Exec(const std::string& host, int port);
};

}  // namespace commands
