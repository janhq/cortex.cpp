#pragma once

#include <string>
#include "utils/result.hpp"

namespace commands {
class EngineReleaseCmd {
 public:
  cpp::result<void, std::string> Exec(const std::string& host, int port,
                                      const std::string& engine_name);
};

}  // namespace commands
