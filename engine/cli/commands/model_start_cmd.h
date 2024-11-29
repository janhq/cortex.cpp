#pragma once

#include <string>
#include <unordered_map>

namespace commands {

class ModelStartCmd {
 public:
  bool Exec(const std::string& host, int port, const std::string& model_handle,
            const std::unordered_map<std::string, std::string>& options,
            bool print_success_log = true);
};
}  // namespace commands
