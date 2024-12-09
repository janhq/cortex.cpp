#pragma once

#include <string>

namespace commands {

class ModelListCmd {
 public:
  void Exec(const std::string& host, int port, const std::string& filter,
            bool display_engine = false, bool display_version = false,
            bool display_cpu_mode = false, bool display_gpu_mode = false,
            bool available = false);
};
}  // namespace commands
