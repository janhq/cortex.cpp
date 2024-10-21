#pragma once

#include <string>

namespace commands {

class ModelListCmd {
 public:
  void Exec(const std::string& host, int port, std::string filter,
            bool display_engine = false, bool display_version = false);
};
}  // namespace commands
