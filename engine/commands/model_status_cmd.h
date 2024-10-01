#pragma once
#include <string>

namespace commands {

class ModelStatusCmd {
 public:
  bool IsLoaded(const std::string& host, int port,
                const std::string& model_handle);
};
}  // namespace commands