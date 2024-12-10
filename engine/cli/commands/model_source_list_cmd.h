#pragma once

#include <string>

namespace commands {

class ModelSourceListCmd {
 public:
  bool Exec(const std::string& host, int port);
};
}  // namespace commands
