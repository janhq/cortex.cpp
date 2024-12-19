#pragma once

#include <optional>
#include <string>

namespace commands {

class ModelPullCmd {
 public:
  std::optional<std::string> Exec(const std::string& host, int port,
                                  const std::string& input);

 private:
  bool AbortModelPull(const std::string& host, int port,
                      const std::string& task_id);
};
}  // namespace commands
